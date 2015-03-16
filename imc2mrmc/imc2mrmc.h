#ifdef __cplusplus
extern "C" {
#endif

#include "include/label.h"
#include "include/mdp_sparse.h"

  /* reads the IMC from filename, converts it to an CTMDP (using actionLabel)
   * and stores it in sparse and labels
   */
  void read_imc(const char filename[], const char actionLabel[],
      mdp_sparse** sparse, labelling** labels);

  // the label by that the interesting states are marked
  extern char* markedToken;

#ifdef __cplusplus
}
#endif

