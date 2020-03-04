#ifndef PWCET_H
#define PWCET_H 1

#define ANN_INNER 0
#define ANN_OUTER 1
#define ANN_CONFLICT_PRIORITY ANN_INNER
#define ALT_MAX 1024

#include <stdio.h>

#include "../pwcet/include/pwcet-runtime.h"
struct evalctx_s {
	loopinfo_t *li;
	param_valuation_t *param_valuation;
	void *pv_data;
};
typedef struct evalctx_s evalctx_t;


void awcet_seq(evalctx_t * ctx, int source_count, formula_t * source,
                           awcet_t * dest);
void awcet_alt(evalctx_t * ctx, int source_count, formula_t * source,
                           awcet_t * dest);
void awcet_loop(evalctx_t * ctx, awcet_t * source, formula_t * dest);
void awcet_ann(evalctx_t * ctx, awcet_t * source, formula_t * dest);


void writeC(formula_t *f, FILE *out, int indent);
void compute_eta_count(formula_t *f);

#endif
