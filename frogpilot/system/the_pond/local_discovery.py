#!/usr/bin/env python3
import socket
import struct
import threading

from dataclasses import dataclass
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from ipaddress import ip_address
from urllib.parse import urlsplit

from openpilot.frogpilot.system.the_pond.config import LOCAL_DISCOVERY_HOSTNAME, LOCAL_DISCOVERY_INSTANCE, LOCAL_REDIRECT_PORT

MDNS_ADDR = "224.0.0.251"
MDNS_PORT = 5353
DNS_CLASS_IN = 1
DNS_CACHE_FLUSH = 0x8000
DNS_QCLASS_UNICAST_RESPONSE = 0x8000
DNS_QTYPE_A = 1
DNS_QTYPE_PTR = 12
DNS_QTYPE_TXT = 16
DNS_QTYPE_SRV = 33
DNS_QTYPE_ANY = 255
DNS_FLAGS_RESPONSE = 0x8400
DNS_TTL_SECONDS = 120
SERVICE_TYPES = ("_http._tcp.local", "_thepond._tcp.local")
SERVICE_ENUMERATION_NAME = "_services._dns-sd._udp.local"


@dataclass(frozen=True)
class DnsQuestion:
  name: str
  qtype: int
  qclass: int


@dataclass(frozen=True)
class DnsQuery:
  identifier: int
  questions: tuple[DnsQuestion, ...]


def start_local_discovery(app_port):
  discovery = LocalDiscovery(app_port)
  discovery.start()
  return discovery


class LocalDiscovery:
  def __init__(self, app_port, hostname=LOCAL_DISCOVERY_HOSTNAME, instance=LOCAL_DISCOVERY_INSTANCE, redirect_port=LOCAL_REDIRECT_PORT):
    self.app_port = app_port
    self.hostname = hostname
    self.instance = instance
    self.redirect_port = redirect_port
    self.redirect = None
    self.mdns = None

  def start(self):
    service_port = self.app_port
    try:
      self.redirect = RedirectServer(self.app_port, self.hostname, self.redirect_port)
      self.redirect.start()
      service_port = self.redirect_port
      print(f"The Pond local redirect enabled at http://{self.hostname}")
    except OSError as exception:
      print(f"The Pond could not bind local redirect port {self.redirect_port}: {exception}")
      print(f"The Pond will still advertise http://{self.hostname}:{self.app_port}")

    try:
      self.mdns = MdnsResponder(self.hostname, self.instance, service_port)
      self.mdns.start()
      print(f"The Pond mDNS discovery enabled for {self.hostname}")
    except OSError as exception:
      print(f"The Pond mDNS discovery is unavailable: {exception}")

  def stop(self):
    if self.mdns:
      self.mdns.stop()
    if self.redirect:
      self.redirect.stop()


class RedirectServer:
  def __init__(self, app_port, hostname, redirect_port):
    self.app_port = app_port
    self.hostname = hostname
    self.redirect_port = redirect_port
    handler = redirect_handler(app_port, hostname)
    self.server = ReusableThreadingHTTPServer(("0.0.0.0", redirect_port), handler)
    self.thread = threading.Thread(target=self.server.serve_forever, name="the-pond-local-redirect", daemon=True)

  def start(self):
    self.thread.start()

  def stop(self):
    self.server.shutdown()
    self.server.server_close()


class ReusableThreadingHTTPServer(ThreadingHTTPServer):
  allow_reuse_address = True
  daemon_threads = True


def redirect_handler(app_port, fallback_hostname):
  class ThePondRedirectHandler(BaseHTTPRequestHandler):
    server_version = "ThePondRedirect/1.0"

    def do_GET(self):
      self.redirect(302)

    def do_HEAD(self):
      self.redirect(302)

    def do_POST(self):
      self.redirect(307)

    def do_PUT(self):
      self.redirect(307)

    def do_PATCH(self):
      self.redirect(307)

    def do_DELETE(self):
      self.redirect(307)

    def redirect(self, status):
      target = redirect_target(self.headers.get("Host"), self.path, app_port, fallback_hostname)
      self.send_response(status)
      self.send_header("Location", target)
      self.send_header("Cache-Control", "no-store")
      self.send_header("Content-Length", "0")
      self.end_headers()

    def log_message(self, *_args):
      return

  return ThePondRedirectHandler


def redirect_target(host_header, request_path, app_port, fallback_hostname=LOCAL_DISCOVERY_HOSTNAME):
  host = redirect_host(host_header, fallback_hostname)
  parsed_path = urlsplit(request_path or "/")
  path = parsed_path.path or "/"
  if parsed_path.query:
    path = f"{path}?{parsed_path.query}"
  return f"http://{host}:{app_port}{path}"


def redirect_host(host_header, fallback_hostname=LOCAL_DISCOVERY_HOSTNAME):
  parsed = urlsplit(f"//{host_header or ''}")
  hostname = (parsed.hostname or "").lower()
  if hostname.endswith(".local"):
    return hostname

  if hostname in ("localhost", "127.0.0.1", "::1"):
    return hostname

  try:
    if not ip_address(hostname).is_global:
      return hostname
  except ValueError:
    pass

  return fallback_hostname


class MdnsResponder:
  def __init__(self, hostname, instance, service_port):
    self.hostname = normalized_dns_name(hostname)
    self.instance = instance
    self.service_port = service_port
    self.host_ips = local_ipv4_addresses
    self.stop_event = threading.Event()
    self.socket = None
    self.thread = threading.Thread(target=self.run, name="the-pond-mdns", daemon=True)

  def start(self):
    self.socket = mdns_socket()
    self.thread.start()
    self.announce()

  def stop(self):
    self.stop_event.set()
    if self.socket:
      self.socket.close()

  def announce(self):
    packet = self.announcement_packet()
    if packet and self.socket:
      self.socket.sendto(packet, (MDNS_ADDR, MDNS_PORT))

  def run(self):
    while not self.stop_event.is_set():
      try:
        data, _address = self.socket.recvfrom(9000)
      except OSError:
        return

      try:
        query = parse_query(data)
      except ValueError:
        continue

      response = self.response_for_query(query)
      if response:
        try:
          target = _address if wants_unicast_response(query, _address) else (MDNS_ADDR, MDNS_PORT)
          self.socket.sendto(response, target)
        except OSError:
          return

  def announcement_packet(self):
    records = []
    for service_type in SERVICE_TYPES:
      records.extend(self.service_records(service_type))
    return dns_response(records) if records else b""

  def response_for(self, data):
    try:
      query = parse_query(data)
    except ValueError:
      return b""

    return self.response_for_query(query)

  def response_for_query(self, query):
    records = []
    for question in query.questions:
      records.extend(self.records_for(question))

    return dns_response(dedupe_records(records), query.identifier) if records else b""

  def records_for(self, question):
    name = normalized_dns_name(question.name)
    qtype = question.qtype
    if question.qclass & 0x7FFF not in (DNS_CLASS_IN, 0):
      return []

    if name == self.hostname and qtype in (DNS_QTYPE_A, DNS_QTYPE_ANY):
      return self.a_records()

    if name == SERVICE_ENUMERATION_NAME and qtype in (DNS_QTYPE_PTR, DNS_QTYPE_ANY):
      return [ptr_record(SERVICE_ENUMERATION_NAME, service_type) for service_type in SERVICE_TYPES]

    if name in SERVICE_TYPES and qtype in (DNS_QTYPE_PTR, DNS_QTYPE_ANY):
      return self.service_records(name)

    for service_type in SERVICE_TYPES:
      if name == self.instance_name(service_type) and qtype in (DNS_QTYPE_SRV, DNS_QTYPE_TXT, DNS_QTYPE_ANY):
        records = []
        if qtype in (DNS_QTYPE_SRV, DNS_QTYPE_ANY):
          records.append(self.srv_record(service_type))
          records.extend(self.a_records())
        if qtype in (DNS_QTYPE_TXT, DNS_QTYPE_ANY):
          records.append(self.txt_record(service_type))
        return records

    return []

  def service_records(self, service_type):
    return [
      ptr_record(service_type, self.instance_name(service_type)),
      self.srv_record(service_type),
      self.txt_record(service_type),
      *self.a_records(),
    ]

  def instance_name(self, service_type):
    return f"{self.instance}.{service_type}"

  def srv_record(self, service_type):
    rdata = struct.pack("!HHH", 0, 0, self.service_port) + dns_name(self.hostname)
    return resource_record(self.instance_name(service_type), DNS_QTYPE_SRV, rdata, cache_flush=True)

  def txt_record(self, service_type):
    return resource_record(self.instance_name(service_type), DNS_QTYPE_TXT, dns_txt(["path=/"]), cache_flush=True)

  def a_records(self):
    return [resource_record(self.hostname, DNS_QTYPE_A, socket.inet_aton(ip), cache_flush=True) for ip in self.host_ips()]


def mdns_socket():
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
  sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  if hasattr(socket, "SO_REUSEPORT"):
    try:
      sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    except OSError:
      pass

  sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack("B", 255))
  sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, struct.pack("B", 0))
  sock.bind(("", MDNS_PORT))
  group = socket.inet_aton(MDNS_ADDR) + socket.inet_aton("0.0.0.0")
  sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, group)
  return sock


def local_ipv4_addresses():
  default_ip = default_ipv4_address()
  if default_ip:
    return [default_ip]

  ips = []
  try:
    import psutil

    for addresses in psutil.net_if_addrs().values():
      for address in addresses:
        if address.family == socket.AF_INET and useful_ipv4(address.address):
          ips.append(address.address)
  except Exception:
    pass

  return sorted(set(ips))


def default_ipv4_address():
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  try:
    sock.connect(("8.8.8.8", 80))
    address = sock.getsockname()[0]
    return address if useful_ipv4(address) else ""
  except OSError:
    return ""
  finally:
    sock.close()


def useful_ipv4(address):
  try:
    parsed = ip_address(address)
  except ValueError:
    return False
  return parsed.version == 4 and not parsed.is_loopback and not parsed.is_link_local and not parsed.is_unspecified


def parse_questions(data):
  return list(parse_query(data).questions)


def parse_query(data):
  if len(data) < 12:
    raise ValueError("DNS packet is too short")

  identifier, _flags, qdcount, _ancount, _nscount, _arcount = struct.unpack("!HHHHHH", data[:12])
  offset = 12
  questions = []
  for _ in range(qdcount):
    name, offset = read_dns_name(data, offset)
    if offset + 4 > len(data):
      raise ValueError("DNS question is truncated")
    qtype, qclass = struct.unpack("!HH", data[offset:offset + 4])
    offset += 4
    questions.append(DnsQuestion(name, qtype, qclass))
  return DnsQuery(identifier, tuple(questions))


def read_dns_name(data, offset):
  labels = []
  jumped = False
  next_offset = offset
  seen = 0

  while True:
    if offset >= len(data):
      raise ValueError("DNS name is truncated")

    length = data[offset]
    if length & 0xC0 == 0xC0:
      if offset + 1 >= len(data):
        raise ValueError("DNS pointer is truncated")
      pointer = ((length & 0x3F) << 8) | data[offset + 1]
      if pointer >= len(data):
        raise ValueError("DNS pointer is out of range")
      if not jumped:
        next_offset = offset + 2
      offset = pointer
      jumped = True
      seen += 1
      if seen > 20:
        raise ValueError("DNS pointer loop")
      continue

    offset += 1
    if length == 0:
      if not jumped:
        next_offset = offset
      return ".".join(labels), next_offset

    if length & 0xC0:
      raise ValueError("DNS label is invalid")
    if offset + length > len(data):
      raise ValueError("DNS label is truncated")

    labels.append(data[offset:offset + length].decode("utf-8", "replace"))
    offset += length


def wants_unicast_response(query, address):
  return address[1] != MDNS_PORT or any(question.qclass & DNS_QCLASS_UNICAST_RESPONSE for question in query.questions)


def dns_response(records, identifier=0):
  header = struct.pack("!HHHHHH", identifier, DNS_FLAGS_RESPONSE, 0, len(records), 0, 0)
  return header + b"".join(records)


def dedupe_records(records):
  return list(dict.fromkeys(records))


def resource_record(name, record_type, rdata, cache_flush=False):
  record_class = DNS_CLASS_IN | (DNS_CACHE_FLUSH if cache_flush else 0)
  return dns_name(name) + struct.pack("!HHIH", record_type, record_class, DNS_TTL_SECONDS, len(rdata)) + rdata


def ptr_record(name, target):
  return resource_record(name, DNS_QTYPE_PTR, dns_name(target), cache_flush=False)


def dns_name(name):
  labels = [label for label in str(name).rstrip(".").split(".") if label]
  encoded = bytearray()
  for label in labels:
    raw = label.encode("utf-8")
    if len(raw) > 63:
      raise ValueError("DNS label is too long")
    encoded.append(len(raw))
    encoded.extend(raw)
  encoded.append(0)
  return bytes(encoded)


def dns_txt(values):
  encoded = bytearray()
  for value in values:
    raw = value.encode("utf-8")
    if len(raw) > 255:
      raise ValueError("DNS TXT value is too long")
    encoded.append(len(raw))
    encoded.extend(raw)
  return bytes(encoded)


def normalized_dns_name(name):
  return str(name).rstrip(".").lower()
