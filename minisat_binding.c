#include "solver.h"

void minisat_add(int lit) {
  /* TODO */
}

sat_state minisat_sat(void) {
  /* TODO */
  return ST_UNKNOWN;
}

sat_state minisat_sat_assum(int *assumptions, int length) {
  /* TODO */
  return ST_UNKNOWN;
}

lit_value minisat_lit_deref(int lit) {
  /* TODO */
  return L_UNKNOWN;
}

extern solver minisat={
  minisat_add,
  minisat_sat,
  minisat_sat_assum,
  minisat_lit_deref
};
