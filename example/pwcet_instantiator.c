#include <stdlib.h>
#include <stdio.h>

#include "../pwcet/include/pwcet-runtime.h"
#include "example-pwcet.h"

int main(void) {
    loopinfo_t li = {.hier = loop_hierarchy, .bnd = loop_bounds };
    
    int wcet = evaluate(&f, &li, NULL, NULL);
    printf("wcet: %d\n", wcet);

}
