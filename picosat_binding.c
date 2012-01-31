#include <stdio.h>

#include <picosat.h>

#include "solver.h"

#define REQUIRE_INIT() do {                  \
                         if (!initialized) { \
                           picosat_init();   \
                           initialized=1;    \
                         }                   \
                       } while(0)

static char initialized=0;

void ps_add(int lit) {
  REQUIRE_INIT();
#ifdef DEBUG
  printf("%d%s", lit, lit==0 ? "\n" : " ");
#endif
  picosat_add(lit);
}

sat_state ps_sat(void) {
  REQUIRE_INIT();
//  puts("ps_sat");
  switch (picosat_sat(-1)) {
    case PICOSAT_SATISFIABLE:
      return ST_SAT;
    case PICOSAT_UNSATISFIABLE:
      return ST_UNSAT;
    default:
      return ST_UNKNOWN;
  }
}

sat_state ps_sat_assum(int *assumptions, int length) {
  REQUIRE_INIT();
  int i=0;

//  puts("ps_sat_assum");
  for (; i<length; i++)
    picosat_assume(assumptions[i]);

  return ps_sat();
}

lit_value ps_lit_deref(int lit) {
  REQUIRE_INIT();
//  puts("ps_lit_deref");
  switch (picosat_deref(lit)) {
    case 1:
      return L_TRUE;
    case -1:
      return L_FALSE;
    default:
      return L_UNKNOWN;
  }
}

void ps_assume(int lit) {
  picosat_assume(lit);
}

extern solver picosat={
  ps_add,
  ps_sat,
  ps_sat_assum,
  ps_lit_deref,
  ps_assume
};
