#include <stdlib.h>
#include <stdio.h>

#include "../pwcet/include/pwcet-runtime.h"
#include "example-pwcet.h"

#define PARAM_FLAG 0x40000000

int b = 10;

int has_param = 0;

int param_value(int param_id) {
  has_param = 1;
  if (param_id == 0) {
    return b;
  } else abort();
}


int param_loop_bound(int loop_id) {
    int bound = loop_bounds(loop_id);
    if (bound & PARAM_FLAG) {
        return param_value(bound & ~PARAM_FLAG);
        
    } else return bound;
}

int main(void) {
    loopinfo_t li = {.hier = loop_hierarchy, .bnd = param_loop_bound };
    
    for (int i = 0; i < 20; i++) {
      b = i;
      long long wcet = evaluate(&f, &li, NULL, NULL);
      if (has_param) {
        printf("for param value %d, wcet: %lld\n", i, wcet);
      } else {
        printf("wcet: %lld\n", wcet);
        break;
      }
    }

}
