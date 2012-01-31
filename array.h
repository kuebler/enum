#ifndef _ARRAY_H
#define _ARRAY_H

/***** MACROS *****/

#define deref(a, n) (a)->content[n]
#define min(a, b) ((a)->length<=(b)->length ? a : b)
#define max(a, b) ((a)->length>(b)->length ? a : b)

/***** TYPES & STRUCTS *****/

typedef struct array {
  unsigned int length;
  int *content;
} array;

/***** DECLARATIONS *****/

void print_array(array *);
void delete_array(array *);
void delete_array_complete(array *);

array *new_array(void);
array *new_array_of_size(unsigned int);
array *take(array *, unsigned int);
array *drop(array *, unsigned int);

#endif /* _ARRAY_H */
