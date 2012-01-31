#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "array.h"

/*
 * Input: Array arr
 *
 * Output array arr
 */
void print_array(array *arr) {
  unsigned int i;

  (void)putchar('[');
  for (i=0; i<arr->length; i++)
    printf("%d%s", arr->content[i], i<arr->length-1 ? " " : "");
  (void)puts("]");
}

/* Try to allocate a new array */
array *new_array(void) {
  array *rv=(array *)malloc(sizeof(array));
  assert(rv!=NULL);
  return rv;
}

/*
 * Input:  length of array to allocate
 * Output: pointer to allocated array
 */
array *new_array_of_size(unsigned int length) {
  array *rv=new_array();
  
  rv->length=length;
  if (length>0) {
    rv->content=(int *)malloc(sizeof(int)*length);
    assert(rv->content!=NULL);
  } else
    rv->content=NULL;

  return rv;
}

/*
 * Input: Array arr
 *
 * Free Array arr (NOT its contents!)
 */
void delete_array(array *arr) {
  free(arr);
}

/*
 * Input: Array to Delete
 *
 * Free array & its contents
 */
void delete_array_complete(array *arr) {
  assert(arr!=NULL);
  if (arr->content!=NULL)
    free(arr->content);

  delete_array(arr);
}

/* 
 * Input:  Array arr, length n
 * Output: Array rv
 *
 * Return array which contains (at most) first n elements of arr
 */
array *take(array *arr, unsigned int n) {
  assert(arr!=NULL);
  array *rv=new_array();

  rv->length=(n>arr->length) ? arr->length : n;
  rv->content=arr->content;

  return rv;
}

/*
 * Input:  Array arr, Length n
 * Output: Array rv
 *
 * Return (possibly zero length) array which contains n+1..arr->length 
 * elements of arr
 */
array *drop(array *arr, unsigned n) {
  assert(arr!=NULL);
  array *rv=new_array();

  rv->length=(n<arr->length) ? arr->length-n : 0;
  rv->content=arr->content+(arr->length-rv->length);

  return rv;
}
