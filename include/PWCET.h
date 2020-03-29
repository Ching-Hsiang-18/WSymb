/* ----------------------------------------------------------------------------
   Copyright (C) 2020, Université de Lille, Lille, FRANCE

   This file is part of CFTExtractor.

   CFTExtractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation ; either version 2 of
   the License, or (at your option) any later version.

   CFTExtractor is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY ; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program ; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA
   ---------------------------------------------------------------------------- */

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
