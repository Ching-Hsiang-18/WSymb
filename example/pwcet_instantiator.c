#include <stdlib.h>
#include <stdio.h>

#include "../pwcet/include/pwcet-runtime.h"
#include "example-pwcet.h"

int has_param = 0;
int b;

void param_valuation(int param_id, param_value_t * param_val, void *data) {
  (void)(data);
  
  /* parametric bound */
  if (param_id == 1) {
    has_param = 1;
    param_val->bound = b;
    return;
  }
  
  /* parametric awcet for function */
  if (param_id == 42) {
    param_val->aw.eta = NULL;
    param_val->aw.others = 1000; /* actual wcet of function associated with param_id 42 */
    return;
  }
  abort();
}


int main(void) {
    loopinfo_t li = {.hier = loop_hierarchy, .bnd = loop_bounds};
    
    for (int i = 0; i < 20; i++) {
      b = i;
      long long wcet = evaluate(&f, &li, param_valuation, NULL);
      if (has_param) {
        printf("for param value %d, wcet: %lld\n", i, wcet);
      } else {
        printf("wcet: %lld\n", wcet);
        break;
      }
    }

}
