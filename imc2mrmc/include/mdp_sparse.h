#ifndef MDP_SPARSE_H
#define MDP_SPARSE_H

#include "mdp_labelset.h"

/* datatypes and functions for CTMDPs without internal non-determinism
 */
typedef struct mdp_values
{
  int **col;    //the column ids
  double **val; //the corresponding values
  int *val_sizes;
  int size;
} mdp_values;

typedef struct mdp_sparse
{
  int num_states;
  mdp_values *val;
  mdp_labelset *label;
} mdp_sparse;

extern void mdp_sparse_free(mdp_sparse *);
extern void mdp_sparse_print(mdp_sparse *);

/* datatypes and functions for MDPs with internal non-determinism
 */

typedef struct mdpi_st_r_map {
  int *col;
  double *rate;
  int size;
} mdpi_st_r_map;

typedef struct mdpi_values {
  int *label;
  mdpi_st_r_map *map;
  int size;
} mdpi_values;

typedef struct mdpi_sparse {
  int num_states;
  mdpi_values *val;
  mdp_labelset *label;
} mdpi_sparse;

extern void mdpi_sparse_print(mdpi_sparse *);
extern void mdpi_sparse_free(mdpi_sparse *);

#endif
