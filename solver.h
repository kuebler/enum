#ifndef _SOLVER_H
#define _SOLVER_H

/***** TYPES & STRUCTS *****/

/*
 * unifying solver structure
 *
 * Semantics of functions:
 *
 *   - void add(lit):   Adds a literal to a internal clause buffer, upon 
 *                      add(0), the currently buffered literals are added
 *                      to the solver as a clause
 *   - sat_state sat(): Check satisfiability status of formula, return values: 
 *                      SAT, UNSAT, UNKNOWN
 *   - sat_state sat_assum(assumptions, #assumptions):
 *                      Add assumptions and check satisfiability status
 *   - lit_value deref(lit):
 *                      Get value of literal in current satisfying assignment
 */
typedef enum { ST_UNKNOWN=0, ST_SAT=10, ST_UNSAT=20 } sat_state;
typedef enum { L_FALSE=-1, L_UNKNOWN=0, L_TRUE=1 } lit_value;

typedef struct solver {
  void (*add)(int lit);
  sat_state (*sat)(void);
  sat_state (*sat_assum)(int *, int);
  lit_value (*lit_deref)(int);
  void (*assume)(int);
} solver;

/***** DECLARATIONS *****/
void add_clause(solver *s, ...);
void print_add(int);
sat_state print_sat(void);
sat_state print_sat_assum(int *, int);

#endif /* _SOLVER_H */
