/*
 * Author: <kuebler@informatik.uni-tuebingen.de>
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

/* GNU multi-precision library */
#include <gmp.h>

#include "solver.h"
#include "array.h"

/***** MACROS *****/

#define FIRSTBOUND(nfree)     (nfree+1)
#define BOUNDVAR(v, nfree)    (v-FIRSTBOUND(nfree))
#define POS(v, nfree)         (FIRSTBOUND(nfree)+3*BOUNDVAR(v, nfree)+0)
#define NEG(v, nfree)         (FIRSTBOUND(nfree)+3*BOUNDVAR(v, nfree)+1)
#define DONTCARE(v, nfree)    (FIRSTBOUND(nfree)+3*BOUNDVAR(v, nfree)+2)
#define IGNORERES(v)          (v ? 1 : 1)
#define SHIFTBOUND(v, nfree)  (nfree*3)
#define RENAMEBOUND(v, nfree) (SHIFTBOUND(v, nfree)+(v-nfree))
#define POSFREE(v)            ((v-1)*3+1) /* v=1: v+=1, v-=2, v*=3; v=2: v+=4, v-=5, v*=6; ... */
#define NEGFREE(v)            ((v-1)*3+2)
#define DCFREE(v)             ((v-1)*3+3)
#define TOTALVARS(nv, nfree)  (3*nfree+(nv-nfree))

int expect_string(FILE *fp, char *str) {
//  puts("expect_string");
  assert(str!=NULL);

  int ch;
  char *start=str;

//  printf("1. ch: %c|\n", ch);
  while (*str!='\0' && (ch=fgetc(fp))!=EOF) {
//    printf("2. ch: %c|\n", ch);
    if (*str!=ch) 
      break;
    str++;
  }

  if (*str=='\0')
    return 1;

//  printf("3. ungetc(%c|, fp)\n", ch);
  ungetc(ch, fp);
  while (str--!=start) {
//    printf("4. unget(%c|, fp)\n", *str);
    ungetc(*str, fp);
  }

  return 0;
}

int expect_and_skip_spaces(FILE *fp, int (*space)(int)) {
//  puts("expect_and_skip_spaces");
  int ch, rv=0;
  while (space((ch=fgetc(fp)))) {
    if (!rv)
      rv=1;
  }
//  printf("5. ungetc(%c|, fp)\n", ch);
  ungetc(ch, fp);
  return rv;
}

int expect_and_skip(FILE *fp, int (*check)(FILE *)) {
//  puts("expect_and_skip");
  int rv=0;
  while (check(fp)) {
    if (!rv)
      rv=1;
  }
  return rv;
}

int not_newline(int c) {
  return c!='\n';
}

int comment(FILE *fp) {
  expect_and_skip_spaces(fp, isspace);
  return expect_string(fp, "c") && expect_and_skip_spaces(fp, not_newline) && expect_and_skip_spaces(fp, isspace);
}

int parse_sign(FILE *fp) {
  return (expect_string(fp, "-") && IGNORERES(expect_and_skip_spaces(fp, isblank))) 
         ? -1 
         : 1;
}

int parse_int(FILE *fp, int *num) {
//  puts("parse_int");
  int ch, res=0, rv=0, sign=parse_sign(fp);
  while ((ch=fgetc(fp))!=EOF && isdigit(ch)) {
    if (!rv) 
      rv=1;
    res=10*res+(ch-'0');
  }

  if (rv)
    *num=(res*sign);
//  printf("6. ungetc(%c|, fp)\n", ch);
  ungetc(ch, fp);

  return rv;
}

int parse_pline(FILE *fp, int *vars, int *clauses) {
//  puts("parse_pline");
  expect_and_skip_spaces(fp, isblank);
  if (expect_string(fp, "p") 
      && expect_and_skip_spaces(fp, isblank) 
      && expect_string(fp, "cnf")
      && expect_and_skip_spaces(fp, isblank) 
      && parse_int(fp, vars)
      && expect_and_skip_spaces(fp, isblank)
      && parse_int(fp, clauses)
      && expect_and_skip_spaces(fp, isspace))
    return 1;
  return 0;
}

int parse_freevars_comment(FILE *fp, int *freevars) {
//  puts("parse_freevars_comment");
  expect_and_skip_spaces(fp, isblank);
  if (expect_string(fp, "c") 
      && expect_and_skip_spaces(fp, isblank)
      && parse_int(fp, freevars)
      && expect_and_skip_spaces(fp, isspace))
    return 1;
  return 0;
}

void die(char *fmt, ...) {
  va_list v;
  va_start(v, fmt);

  vfprintf(stderr, fmt, v);
  fflush(stderr);

  va_end(v);
  exit(1);
}

void handle_bound(solver *s, int var, int lit, int nfree) {
  s->add((lit<0 ? -1 : 1)*RENAMEBOUND(var, nfree));
}

void handle_free(solver *s, int var, int lit, int nfree) {
  s->add(lit<0 ? NEGFREE(var) : POSFREE(var));
}

void parse_clause(FILE *fp, solver *s, int nfree, 
    void (*handle_bound)(solver *,int, int, int), 
    void (*handle_free)(solver *,int, int, int)) 
{
  //  puts("parse_clause");
  int lit=0;

  expect_and_skip(fp, comment);
  while (parse_int(fp, &lit)) {
    int v=abs(lit);
    //    puts("parse_clause: while");
    if (v>nfree) /* bound variable, rename */
      handle_bound(s, v, lit, nfree);
    else
      if (lit!=0)
        handle_free(s, v, lit, nfree);
      else {
        s->add(0);
        break;
      }

    expect_and_skip_spaces(fp, isspace);
  }
  expect_and_skip_spaces(fp, isspace);
}

/* 
 * Input: Solver s, #(Free variables) freevars, #(Variables) vars
 *
 * Introduce clauses for newly introduced variables (v+: positive phase of v, 
 * v-: negative phase of v, v*: don't care):
 *
 *   1. ~(v+ /\ v-)
 *   2. v* <-> ~(v+ \/ v-)
 * 
 * for each free variable v
 */
void phase_relaxation_clauses(solver *s, int freevars, int vars) {
  int i=1;

  for (; i<=freevars; i++) {
    /* ~(v+ /\ v-) */
    add_clause(s, -POSFREE(i), -NEGFREE(i), 0);

    /* (v* -> ~v+) /\ (v* -> ~v-) */
    add_clause(s, -DCFREE(i), -POSFREE(i), 0);
    add_clause(s, -DCFREE(i), -NEGFREE(i), 0);

    /* ~(v+ \/ v-) -> v* */
    add_clause(s, POSFREE(i), NEGFREE(i), DCFREE(i), 0);
  }
}

/*
 * Input:  Number of free variables
 * Output: array containing don't cares of phase relaxation
 *
 * Return freshly allocated array of input variables (i.e. the don't care 
 * representatives of the phase relaxation)
 */
array *in_array(int freevars) {
  array *rv=NULL;

  if (freevars>0) {
    unsigned int i=1;

    rv=new_array_of_size(freevars);
    for (; i<=freevars; i++)
      rv->content[i-1]=DCFREE(i);
  }

  return rv;
}

/*
 * Input:  Pointer to file
 * Output: Array of sorter outputs or NULL
 *
 * Parse DIMACS file annotated with free clauses. Side effect: Solver is filled with 
 * phase relaxated problem
 */
array *parse_dimacs(FILE *fp, solver *s, int *fv) {
//  puts("parse_dimacs");
  int freevars, vars, clauses, to_read;
  
  if (!parse_freevars_comment(fp, &freevars))
    die("failed to parse freevars line!");

  *fv=freevars;
  expect_and_skip(fp, comment); /* skip comments */

  if (!parse_pline(fp, &vars, &clauses))
    die("failed to parse p-line!");

  fprintf(stderr, "c #vars (free): %d, #vars (total): %d, #clauses: %d\n", freevars, vars, clauses);
  to_read=clauses;
  while (!feof(fp) && to_read-->0 )
    parse_clause(fp, s, freevars, handle_bound, handle_free);
  
  if (to_read>0)
    die("Number of clauses < declared number of clauses in p-line!");

  phase_relaxation_clauses(s, freevars, vars);
  /*
   * Introduce cardinality constraint over v* for all free
   * variables v
   */
  if (freevars>0) {
    array *in, *rv;

    in=in_array(freevars);
    rv=non_deleting_sorter(s, in, TOTALVARS(vars, freevars)+1);
    delete_array_complete(in);

    return rv;
  } else
    return NULL;
}

/*
 * Input:  width of bit shift to the left, current #models, increment
 * Output: 
 *
 * Sideffect: models := models + 2^power, output models
 */
#define inc_and_output_models(power, models, inc) do { \
  mpz_set_ui(inc, 1UL);                                \
  mpz_mul_2exp(inc, inc, power);                       \
  mpz_add(models, models, inc);                        \
                                                       \
  gmp_printf("c currently found %Zd models\n", models); \
} while (0)

/*
 * Input:  solver, array of variables, number of variables assumed true, 
 *         total number of assumed variables
 * Output: satisfiability state of loaded formula in solver + assumptions
 *
 * Used to restrict the cardinality constraint ranging over the free variables
 */
sat_state assume_exactly(solver *s, int *assumptions, int n, int length) {
  int i=0;
#ifdef DEBUG
  printf("assume: ");
#endif
  for (; i<length; i++) {
#ifdef DEBUG
    printf("%d ", (i<n ? assumptions[i] : -assumptions[i]));
#endif
    s->assume((i<n ? assumptions[i] : -assumptions[i]));
  }
#ifdef DEBUG
  putchar('\n');
#endif
  return s->sat();
}

/*
 * Input:  solver, array of sorter outputs, number of free variables
 *
 * Generate terms one after the other; optionally track number of models 
 * enumerated.
 *
 * Degenerate cases:
 *
 *   1. No free variables: Return True/\infty if problem is SAT, False/0 if UNSAT
 *   2. Formula is UNSAT: Return False/0
 *   3. Formula is SAT with all don't care variables set to True, i.e. 
 *      outcome doesn't depend on any of the free variables: True/2^#freevars
 *
 * Overall algorithm:
 *
 *   Test for degenerate cases, return if one is hit
 *   for i=#freevars-1 to 0
 *     constrain problem to contain at least i don't cares
 *     while problem is sat
 *       vs: non-don't care variables assigned True
 *       output term build from vs
 *       add 2^i to lower model bound
 *   
 */
void enumerate(solver *s, array *sout, int freevars, int count) {
  sat_state rv=s->sat();
  int i=freevars, fvclause[freevars];
  mpz_t models, inc;

  if (freevars <= 0 || rv==ST_UNSAT) {
    if (rv==ST_UNSAT)
      puts("rv==ST_UNSAT");
    if (freevars<=0)
      puts("freevars<=0");
    /* no free variables, test satisfiability (SAT: True/+Inf, UNSAT: False/0) */
    printf("%s\n", rv==ST_SAT ? (count ? "T\nc INFINITE" : "T") 
                              : (rv==ST_UNSAT ? (count ? "F\n c 0" : "F") 
                                              : "?"));
    return;
  }

  mpz_init(models); mpz_init(inc);
  for (; i>=0; i--) /* constrain problem to contain at least i don't cares */
    while (assume_exactly(s, sout->content, i, freevars)==ST_SAT) {
      int j=1, cnt=0;

      if (i==freevars) {
        /* all free variables are don't care --> return True/2^#freevars */
        puts("T");
        if (count)
          inc_and_output_models(i, models, inc);

        mpz_clears(models, inc, NULL);
        return;
      }

      /* 
       * 1. extract non-dcs assigned to true, 
       * 2. output them, 
       * 3. conditionaly update number of models
       * 4. add blocking clause 
       */
      for (; j<=freevars; j++)
        if (s->lit_deref(DCFREE(j))!=L_TRUE) {
          if (s->lit_deref(POSFREE(j))==L_TRUE) {
            printf("%d ", j);
            fvclause[cnt++]=NEGFREE(j);
          }
          else {
            printf("-%d ", j);
            fvclause[cnt++]=POSFREE(j);
          }
        }
      putchar('\n');

      for (j=0; j<cnt; j++)
        s->add(fvclause[j]);

      s->add(0);
      if (count)
        inc_and_output_models(i, models, inc);
    }
  mpz_clears(models, inc, NULL);
}

extern solver picosat;

int main(int argc, char **argv) {
  int freevars=0;
/*  solver s;

  s.add=print_add; 
  parse_dimacs(stdin, &s, &freevars); 
*/  array *sout=parse_dimacs(stdin, &picosat, &freevars);

  enumerate(&picosat, sout, freevars, 1);

  return 0;
}
