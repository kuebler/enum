/*
 * Author: <kuebler@informatik.uni-tuebingen.de>
 *
 * Compile with 
 *
 *   gcc -O3 -o bailleuxboufkhad bailleuxboufkhad.c
 *
 * Run with
 * 
 *   ./bailleuxboufkhad <numberofinputs>
 *
 * TODO needs proper documentation
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include "solver.h"
#include "array.h"

/*
 * Input:  Integer n
 * Output: Number of auxillary variables
 *
 * Calculate number of auxillary variables that have to be introduced to 
 * realize Bailleux-Boufkhad style sorting network for n inputs
 */
int space(int n) {
  if (n<2)
    return 0;
  else
    return n+space(n/2)+space(n-(n/2));
}

void add_out_implications(solver *s, array *in, array *o, int i, int len) {
  /* out_i <- in_i */
  add_clause(s, -deref(in, i), deref(o, i), 0);
  /* in_i <- out_{len(other)+i} */
  add_clause(s, -deref(o, len+i), deref(in, i), 0);
}

void totalitarize(solver *solver, array *in0, array *in1, array *out) {
  array *i0=max(in0, in1), *i1=min(in0, in1);
  unsigned int i, j, len0=i0->length, len1=i1->length;

  for (i=0; i<len0; i++) {
    for (j=0; j<len1; j++)
      if (i+j < len0+len1) {
        /* out_{i+j+1} <- in0_i, in1_j */
        add_clause(solver, 
            -deref(i0, i), -deref(i1, j), deref(out, i+j+1), 0);

        /* in0_i, in1_j <- out_{i+j} */
        add_clause(solver, -deref(out, i+j), deref(i0, i), deref(i1, j), 0);
      }

    add_out_implications(solver, i0, out, i, len1);
    if (i<len1) 
      add_out_implications(solver, i1, out, i, len0);
  }
}

array *auxsorter(solver *solver, array *in, array *out, int start, int end)
{
  array *this_out, *left_out, *right_out;
  if (end-start > 1) {
    this_out=take(out, end-start);
    left_out=drop(out, end-start);
    right_out=drop(left_out, space((end-start)/2));

    array *left=auxsorter(solver, in, left_out, start, start+(end-start)/2);
    array *right=auxsorter(solver, in, right_out, start+(end-start)/2, end);

    totalitarize(solver, left, right, this_out);

    delete_array(left_out); delete_array(left);
    delete_array(right_out); delete_array(right);
  } else {
    this_out=new_array();
    this_out->length=1;
    this_out->content=&in->content[start];
  }

  return this_out;
}

array *get_out(int in_length, unsigned int start) {
  array *rv=new_array_of_size(space(in_length));
  unsigned int i=0;

  for (; i<rv->length; i++)
    rv->content[i]=start+i;

  return rv;
}

void sorter(solver *solver, array *in, unsigned int offset) {
  assert(solver!=NULL);
  assert(in!=NULL);

  array *out=get_out(in->length, offset), 
        *rv;

  rv=auxsorter(solver, in, out, 0, in->length);

  delete_array_complete(out);
  delete_array(rv);
}

array *non_deleting_sorter(solver *solver, array *in, unsigned int offset) {
  assert(solver!=NULL);
  assert(in!=NULL);

  array *out=get_out(in->length, offset), 
        *rv;

  rv=auxsorter(solver, in, out, 0, in->length);

//  delete_array_complete(out);
  delete_array(out);

  return rv;
}

array *get_in(unsigned int length, unsigned int offset) {
  array *rv=new_array_of_size(length);
  unsigned int i=0;

  for (; i<length; i++)
    rv->content[i]=offset+i;

  return rv;
}

void usage(char *name) {
  fprintf(stderr, "usage:\n\t%s <inputlength>\n", name);
  exit(1);
}
/*
int main(int argc, char **argv) {
  array *in;
  solver s;

  if (argc!=2)
    usage(argv[0]);

  in=get_in((unsigned int)atoi(argv[1]), 1);
  s.add=print_add;
  sorter(&s, in, in->length+1);

  return 0;
}*/
