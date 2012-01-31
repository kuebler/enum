#include <stdio.h>
#include <stdarg.h>

#include "solver.h"

/*
 * Input: Variable list of integer arguments (0-terminated)
 */
void add_clause(solver *s, ...) {
  va_list vargs;
  int curr;
  
  va_start(vargs, s);
  while ((curr=va_arg(vargs, int))!=0)
    s->add(curr);
  va_end(vargs);

  s->add(0);
}

/*
 * Input: Literal
 *
 * Mock function: Uppon adding literals, simply print them. Output is space 
 * separated, end of clause (=0) starts a new line.
 */
void print_add(int lit) {
  printf("%d%s", lit, lit==0 ? "\n" : " ");
}

/*
 * Input:  Literal
 *
 * Mock function, always return UNKNOWN
 */
sat_state print_sat(void) {
  return ST_UNKNOWN;
}

/*
 * Input:  Array of assumed literals
 *
 * Mock function, always return UNKNOWN
 */
sat_state print_sat_assum(int *assumptions, int length) {
  printf("#assumptions: %d (%p)\n", length, assumptions);
  return ST_UNKNOWN;
}
