#!/usr/bin/env python3
from openpilot.system.hardware import PC

from openpilot.frogpilot.system.the_pond.app import create_app
from openpilot.frogpilot.system.the_pond.config import DEBUG_PORT, DEVICE_PORT
from openpilot.frogpilot.system.the_pond.local_discovery import start_local_discovery

local_discovery = None


def main():
  global local_discovery

  app = create_app()

  debug = PC or __package__ == "the_pond"
  port = DEBUG_PORT if debug else DEVICE_PORT

  if debug:
    print("\"The Pond\" is not running on a comma device, enabling debug mode")
  else:
    local_discovery = start_local_discovery(port)

  # threaded so SSE streams, video range requests, and long device actions can run concurrently
  # instead of blocking the single-threaded dev server. No new WSGI dependency.
  app.run(host="0.0.0.0", port=port, debug=debug, threaded=True)


if __name__ == "__main__":
  main()
