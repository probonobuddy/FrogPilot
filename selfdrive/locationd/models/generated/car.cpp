#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                       Code generated with SymPy 1.12                       *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_3439797905750062265) {
   out_3439797905750062265[0] = delta_x[0] + nom_x[0];
   out_3439797905750062265[1] = delta_x[1] + nom_x[1];
   out_3439797905750062265[2] = delta_x[2] + nom_x[2];
   out_3439797905750062265[3] = delta_x[3] + nom_x[3];
   out_3439797905750062265[4] = delta_x[4] + nom_x[4];
   out_3439797905750062265[5] = delta_x[5] + nom_x[5];
   out_3439797905750062265[6] = delta_x[6] + nom_x[6];
   out_3439797905750062265[7] = delta_x[7] + nom_x[7];
   out_3439797905750062265[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_9037629085952059152) {
   out_9037629085952059152[0] = -nom_x[0] + true_x[0];
   out_9037629085952059152[1] = -nom_x[1] + true_x[1];
   out_9037629085952059152[2] = -nom_x[2] + true_x[2];
   out_9037629085952059152[3] = -nom_x[3] + true_x[3];
   out_9037629085952059152[4] = -nom_x[4] + true_x[4];
   out_9037629085952059152[5] = -nom_x[5] + true_x[5];
   out_9037629085952059152[6] = -nom_x[6] + true_x[6];
   out_9037629085952059152[7] = -nom_x[7] + true_x[7];
   out_9037629085952059152[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_8501126860316219630) {
   out_8501126860316219630[0] = 1.0;
   out_8501126860316219630[1] = 0;
   out_8501126860316219630[2] = 0;
   out_8501126860316219630[3] = 0;
   out_8501126860316219630[4] = 0;
   out_8501126860316219630[5] = 0;
   out_8501126860316219630[6] = 0;
   out_8501126860316219630[7] = 0;
   out_8501126860316219630[8] = 0;
   out_8501126860316219630[9] = 0;
   out_8501126860316219630[10] = 1.0;
   out_8501126860316219630[11] = 0;
   out_8501126860316219630[12] = 0;
   out_8501126860316219630[13] = 0;
   out_8501126860316219630[14] = 0;
   out_8501126860316219630[15] = 0;
   out_8501126860316219630[16] = 0;
   out_8501126860316219630[17] = 0;
   out_8501126860316219630[18] = 0;
   out_8501126860316219630[19] = 0;
   out_8501126860316219630[20] = 1.0;
   out_8501126860316219630[21] = 0;
   out_8501126860316219630[22] = 0;
   out_8501126860316219630[23] = 0;
   out_8501126860316219630[24] = 0;
   out_8501126860316219630[25] = 0;
   out_8501126860316219630[26] = 0;
   out_8501126860316219630[27] = 0;
   out_8501126860316219630[28] = 0;
   out_8501126860316219630[29] = 0;
   out_8501126860316219630[30] = 1.0;
   out_8501126860316219630[31] = 0;
   out_8501126860316219630[32] = 0;
   out_8501126860316219630[33] = 0;
   out_8501126860316219630[34] = 0;
   out_8501126860316219630[35] = 0;
   out_8501126860316219630[36] = 0;
   out_8501126860316219630[37] = 0;
   out_8501126860316219630[38] = 0;
   out_8501126860316219630[39] = 0;
   out_8501126860316219630[40] = 1.0;
   out_8501126860316219630[41] = 0;
   out_8501126860316219630[42] = 0;
   out_8501126860316219630[43] = 0;
   out_8501126860316219630[44] = 0;
   out_8501126860316219630[45] = 0;
   out_8501126860316219630[46] = 0;
   out_8501126860316219630[47] = 0;
   out_8501126860316219630[48] = 0;
   out_8501126860316219630[49] = 0;
   out_8501126860316219630[50] = 1.0;
   out_8501126860316219630[51] = 0;
   out_8501126860316219630[52] = 0;
   out_8501126860316219630[53] = 0;
   out_8501126860316219630[54] = 0;
   out_8501126860316219630[55] = 0;
   out_8501126860316219630[56] = 0;
   out_8501126860316219630[57] = 0;
   out_8501126860316219630[58] = 0;
   out_8501126860316219630[59] = 0;
   out_8501126860316219630[60] = 1.0;
   out_8501126860316219630[61] = 0;
   out_8501126860316219630[62] = 0;
   out_8501126860316219630[63] = 0;
   out_8501126860316219630[64] = 0;
   out_8501126860316219630[65] = 0;
   out_8501126860316219630[66] = 0;
   out_8501126860316219630[67] = 0;
   out_8501126860316219630[68] = 0;
   out_8501126860316219630[69] = 0;
   out_8501126860316219630[70] = 1.0;
   out_8501126860316219630[71] = 0;
   out_8501126860316219630[72] = 0;
   out_8501126860316219630[73] = 0;
   out_8501126860316219630[74] = 0;
   out_8501126860316219630[75] = 0;
   out_8501126860316219630[76] = 0;
   out_8501126860316219630[77] = 0;
   out_8501126860316219630[78] = 0;
   out_8501126860316219630[79] = 0;
   out_8501126860316219630[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_2025150200435505378) {
   out_2025150200435505378[0] = state[0];
   out_2025150200435505378[1] = state[1];
   out_2025150200435505378[2] = state[2];
   out_2025150200435505378[3] = state[3];
   out_2025150200435505378[4] = state[4];
   out_2025150200435505378[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_2025150200435505378[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_2025150200435505378[7] = state[7];
   out_2025150200435505378[8] = state[8];
}
void F_fun(double *state, double dt, double *out_6775967239465341397) {
   out_6775967239465341397[0] = 1;
   out_6775967239465341397[1] = 0;
   out_6775967239465341397[2] = 0;
   out_6775967239465341397[3] = 0;
   out_6775967239465341397[4] = 0;
   out_6775967239465341397[5] = 0;
   out_6775967239465341397[6] = 0;
   out_6775967239465341397[7] = 0;
   out_6775967239465341397[8] = 0;
   out_6775967239465341397[9] = 0;
   out_6775967239465341397[10] = 1;
   out_6775967239465341397[11] = 0;
   out_6775967239465341397[12] = 0;
   out_6775967239465341397[13] = 0;
   out_6775967239465341397[14] = 0;
   out_6775967239465341397[15] = 0;
   out_6775967239465341397[16] = 0;
   out_6775967239465341397[17] = 0;
   out_6775967239465341397[18] = 0;
   out_6775967239465341397[19] = 0;
   out_6775967239465341397[20] = 1;
   out_6775967239465341397[21] = 0;
   out_6775967239465341397[22] = 0;
   out_6775967239465341397[23] = 0;
   out_6775967239465341397[24] = 0;
   out_6775967239465341397[25] = 0;
   out_6775967239465341397[26] = 0;
   out_6775967239465341397[27] = 0;
   out_6775967239465341397[28] = 0;
   out_6775967239465341397[29] = 0;
   out_6775967239465341397[30] = 1;
   out_6775967239465341397[31] = 0;
   out_6775967239465341397[32] = 0;
   out_6775967239465341397[33] = 0;
   out_6775967239465341397[34] = 0;
   out_6775967239465341397[35] = 0;
   out_6775967239465341397[36] = 0;
   out_6775967239465341397[37] = 0;
   out_6775967239465341397[38] = 0;
   out_6775967239465341397[39] = 0;
   out_6775967239465341397[40] = 1;
   out_6775967239465341397[41] = 0;
   out_6775967239465341397[42] = 0;
   out_6775967239465341397[43] = 0;
   out_6775967239465341397[44] = 0;
   out_6775967239465341397[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_6775967239465341397[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_6775967239465341397[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_6775967239465341397[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_6775967239465341397[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_6775967239465341397[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_6775967239465341397[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_6775967239465341397[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_6775967239465341397[53] = -9.8000000000000007*dt;
   out_6775967239465341397[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_6775967239465341397[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_6775967239465341397[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_6775967239465341397[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_6775967239465341397[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_6775967239465341397[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_6775967239465341397[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_6775967239465341397[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_6775967239465341397[62] = 0;
   out_6775967239465341397[63] = 0;
   out_6775967239465341397[64] = 0;
   out_6775967239465341397[65] = 0;
   out_6775967239465341397[66] = 0;
   out_6775967239465341397[67] = 0;
   out_6775967239465341397[68] = 0;
   out_6775967239465341397[69] = 0;
   out_6775967239465341397[70] = 1;
   out_6775967239465341397[71] = 0;
   out_6775967239465341397[72] = 0;
   out_6775967239465341397[73] = 0;
   out_6775967239465341397[74] = 0;
   out_6775967239465341397[75] = 0;
   out_6775967239465341397[76] = 0;
   out_6775967239465341397[77] = 0;
   out_6775967239465341397[78] = 0;
   out_6775967239465341397[79] = 0;
   out_6775967239465341397[80] = 1;
}
void h_25(double *state, double *unused, double *out_6599346279525017865) {
   out_6599346279525017865[0] = state[6];
}
void H_25(double *state, double *unused, double *out_2461857742238722549) {
   out_2461857742238722549[0] = 0;
   out_2461857742238722549[1] = 0;
   out_2461857742238722549[2] = 0;
   out_2461857742238722549[3] = 0;
   out_2461857742238722549[4] = 0;
   out_2461857742238722549[5] = 0;
   out_2461857742238722549[6] = 1;
   out_2461857742238722549[7] = 0;
   out_2461857742238722549[8] = 0;
}
void h_24(double *state, double *unused, double *out_1342302019616489287) {
   out_1342302019616489287[0] = state[4];
   out_1342302019616489287[1] = state[5];
}
void H_24(double *state, double *unused, double *out_6313034665573448345) {
   out_6313034665573448345[0] = 0;
   out_6313034665573448345[1] = 0;
   out_6313034665573448345[2] = 0;
   out_6313034665573448345[3] = 0;
   out_6313034665573448345[4] = 1;
   out_6313034665573448345[5] = 0;
   out_6313034665573448345[6] = 0;
   out_6313034665573448345[7] = 0;
   out_6313034665573448345[8] = 0;
   out_6313034665573448345[9] = 0;
   out_6313034665573448345[10] = 0;
   out_6313034665573448345[11] = 0;
   out_6313034665573448345[12] = 0;
   out_6313034665573448345[13] = 0;
   out_6313034665573448345[14] = 1;
   out_6313034665573448345[15] = 0;
   out_6313034665573448345[16] = 0;
   out_6313034665573448345[17] = 0;
}
void h_30(double *state, double *unused, double *out_5148714886896134698) {
   out_5148714886896134698[0] = state[4];
}
void H_30(double *state, double *unused, double *out_2332518795095482479) {
   out_2332518795095482479[0] = 0;
   out_2332518795095482479[1] = 0;
   out_2332518795095482479[2] = 0;
   out_2332518795095482479[3] = 0;
   out_2332518795095482479[4] = 1;
   out_2332518795095482479[5] = 0;
   out_2332518795095482479[6] = 0;
   out_2332518795095482479[7] = 0;
   out_2332518795095482479[8] = 0;
}
void h_26(double *state, double *unused, double *out_6295266483020351139) {
   out_6295266483020351139[0] = state[7];
}
void H_26(double *state, double *unused, double *out_1279645576635333675) {
   out_1279645576635333675[0] = 0;
   out_1279645576635333675[1] = 0;
   out_1279645576635333675[2] = 0;
   out_1279645576635333675[3] = 0;
   out_1279645576635333675[4] = 0;
   out_1279645576635333675[5] = 0;
   out_1279645576635333675[6] = 0;
   out_1279645576635333675[7] = 1;
   out_1279645576635333675[8] = 0;
}
void h_27(double *state, double *unused, double *out_3786794176255164482) {
   out_3786794176255164482[0] = state[3];
}
void H_27(double *state, double *unused, double *out_157755483295057568) {
   out_157755483295057568[0] = 0;
   out_157755483295057568[1] = 0;
   out_157755483295057568[2] = 0;
   out_157755483295057568[3] = 1;
   out_157755483295057568[4] = 0;
   out_157755483295057568[5] = 0;
   out_157755483295057568[6] = 0;
   out_157755483295057568[7] = 0;
   out_157755483295057568[8] = 0;
}
void h_29(double *state, double *unused, double *out_8296355919152571273) {
   out_8296355919152571273[0] = state[1];
}
void H_29(double *state, double *unused, double *out_1555607243574493465) {
   out_1555607243574493465[0] = 0;
   out_1555607243574493465[1] = 1;
   out_1555607243574493465[2] = 0;
   out_1555607243574493465[3] = 0;
   out_1555607243574493465[4] = 0;
   out_1555607243574493465[5] = 0;
   out_1555607243574493465[6] = 0;
   out_1555607243574493465[7] = 0;
   out_1555607243574493465[8] = 0;
}
void h_28(double *state, double *unused, double *out_2619050375179022745) {
   out_2619050375179022745[0] = state[0];
}
void H_28(double *state, double *unused, double *out_408023027990832786) {
   out_408023027990832786[0] = 1;
   out_408023027990832786[1] = 0;
   out_408023027990832786[2] = 0;
   out_408023027990832786[3] = 0;
   out_408023027990832786[4] = 0;
   out_408023027990832786[5] = 0;
   out_408023027990832786[6] = 0;
   out_408023027990832786[7] = 0;
   out_408023027990832786[8] = 0;
}
void h_31(double *state, double *unused, double *out_5076562567834182815) {
   out_5076562567834182815[0] = state[8];
}
void H_31(double *state, double *unused, double *out_2492503704115682977) {
   out_2492503704115682977[0] = 0;
   out_2492503704115682977[1] = 0;
   out_2492503704115682977[2] = 0;
   out_2492503704115682977[3] = 0;
   out_2492503704115682977[4] = 0;
   out_2492503704115682977[5] = 0;
   out_2492503704115682977[6] = 0;
   out_2492503704115682977[7] = 0;
   out_2492503704115682977[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_3439797905750062265) {
  err_fun(nom_x, delta_x, out_3439797905750062265);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_9037629085952059152) {
  inv_err_fun(nom_x, true_x, out_9037629085952059152);
}
void car_H_mod_fun(double *state, double *out_8501126860316219630) {
  H_mod_fun(state, out_8501126860316219630);
}
void car_f_fun(double *state, double dt, double *out_2025150200435505378) {
  f_fun(state,  dt, out_2025150200435505378);
}
void car_F_fun(double *state, double dt, double *out_6775967239465341397) {
  F_fun(state,  dt, out_6775967239465341397);
}
void car_h_25(double *state, double *unused, double *out_6599346279525017865) {
  h_25(state, unused, out_6599346279525017865);
}
void car_H_25(double *state, double *unused, double *out_2461857742238722549) {
  H_25(state, unused, out_2461857742238722549);
}
void car_h_24(double *state, double *unused, double *out_1342302019616489287) {
  h_24(state, unused, out_1342302019616489287);
}
void car_H_24(double *state, double *unused, double *out_6313034665573448345) {
  H_24(state, unused, out_6313034665573448345);
}
void car_h_30(double *state, double *unused, double *out_5148714886896134698) {
  h_30(state, unused, out_5148714886896134698);
}
void car_H_30(double *state, double *unused, double *out_2332518795095482479) {
  H_30(state, unused, out_2332518795095482479);
}
void car_h_26(double *state, double *unused, double *out_6295266483020351139) {
  h_26(state, unused, out_6295266483020351139);
}
void car_H_26(double *state, double *unused, double *out_1279645576635333675) {
  H_26(state, unused, out_1279645576635333675);
}
void car_h_27(double *state, double *unused, double *out_3786794176255164482) {
  h_27(state, unused, out_3786794176255164482);
}
void car_H_27(double *state, double *unused, double *out_157755483295057568) {
  H_27(state, unused, out_157755483295057568);
}
void car_h_29(double *state, double *unused, double *out_8296355919152571273) {
  h_29(state, unused, out_8296355919152571273);
}
void car_H_29(double *state, double *unused, double *out_1555607243574493465) {
  H_29(state, unused, out_1555607243574493465);
}
void car_h_28(double *state, double *unused, double *out_2619050375179022745) {
  h_28(state, unused, out_2619050375179022745);
}
void car_H_28(double *state, double *unused, double *out_408023027990832786) {
  H_28(state, unused, out_408023027990832786);
}
void car_h_31(double *state, double *unused, double *out_5076562567834182815) {
  h_31(state, unused, out_5076562567834182815);
}
void car_H_31(double *state, double *unused, double *out_2492503704115682977) {
  H_31(state, unused, out_2492503704115682977);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
