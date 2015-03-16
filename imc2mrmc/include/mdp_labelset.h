#ifndef MDP_LABELSET_H
#define MDP_LABELSET_H

typedef struct mdp_labelset
{
  int reserved;
  int next;
  char **label;
} mdp_labelset;

extern mdp_labelset *mdp_labelset_new(int);
extern void mdp_labelset_free(mdp_labelset *);
extern int mdp_labelset_add(mdp_labelset *, const char *);
extern int mdp_labelset_get_label_no(const mdp_labelset *, const char *);

#endif
