#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void live_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_9(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_12(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_35(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_32(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_33(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_H(double *in_vec, double *out_8714777534752411770);
void live_err_fun(double *nom_x, double *delta_x, double *out_8420908973345152243);
void live_inv_err_fun(double *nom_x, double *true_x, double *out_6373820483983697623);
void live_H_mod_fun(double *state, double *out_2014648748224606869);
void live_f_fun(double *state, double dt, double *out_572770800773414698);
void live_F_fun(double *state, double dt, double *out_4628830018475769441);
void live_h_4(double *state, double *unused, double *out_5644676004644475384);
void live_H_4(double *state, double *unused, double *out_175590082113770210);
void live_h_9(double *state, double *unused, double *out_6693623896431741862);
void live_H_9(double *state, double *unused, double *out_65599564515820435);
void live_h_10(double *state, double *unused, double *out_358728263573819771);
void live_H_10(double *state, double *unused, double *out_3475824859588229368);
void live_h_12(double *state, double *unused, double *out_5012611579788858310);
void live_H_12(double *state, double *unused, double *out_4843866325918191585);
void live_h_35(double *state, double *unused, double *out_5012948391230367202);
void live_H_35(double *state, double *unused, double *out_3191071975258837166);
void live_h_32(double *state, double *unused, double *out_1968173209890023816);
void live_H_32(double *state, double *unused, double *out_8992376347466449724);
void live_h_13(double *state, double *unused, double *out_7677932994835457246);
void live_H_13(double *state, double *unused, double *out_4303455988258806300);
void live_h_14(double *state, double *unused, double *out_6693623896431741862);
void live_H_14(double *state, double *unused, double *out_65599564515820435);
void live_h_33(double *state, double *unused, double *out_8504974916418743967);
void live_H_33(double *state, double *unused, double *out_6341628979897694770);
void live_predict(double *in_x, double *in_P, double *in_Q, double dt);
}