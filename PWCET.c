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

#include <string.h>
#include <stdlib.h>

#include "include/PWCET.h"
#include "pwcet/include/pwcet-runtime.h"

#include <stdio.h>

void compute_node(evalctx_t * ctx, formula_t * f)
{
	int i;
	switch (f->kind) {
		case KIND_SEQ:
#ifdef DEBUG
			printf("compute_node: start processing SEQ node\n");
#endif
			for (i = 0; i < f->opdata.children_count; i++)
				compute_node(ctx, &f->children[i]);
			awcet_seq(ctx, f->opdata.children_count, f->children, &f->aw);
#ifdef DEBUG
			printf("compute_node: end processing SEQ node\n");
#endif
			break;
		case KIND_ALT:
#ifdef DEBUG
			printf("compute_node: start processing ALT node\n");
#endif
			for (i = 0; i < f->opdata.children_count; i++)
				compute_node(ctx, &f->children[i]);
			awcet_alt(ctx, f->opdata.children_count, f->children, &f->aw);
#ifdef DEBUG
			printf("compute_node: end processing ALT node\n");
#endif
			break;
		case KIND_LOOP:
#ifdef DEBUG
			printf("compute_node: start processing LOOP node\n");
#endif
			compute_node(ctx, &f->children[0]);
			awcet_loop(ctx, &f->children->aw, f);
#ifdef DEBUG
			printf("compute_node: end processing LOOP node\n");
#endif
			break;
		case KIND_ANN:
#ifdef DEBUG
			printf("compute_node: start processing ANN node\n");
#endif
			compute_node(ctx, &f->children[0]);
			awcet_ann(ctx, &f->children->aw, f);
#ifdef DEBUG
			printf("compute_node: end processing ANN node\n");
#endif
			break;
		case KIND_AWCET:
#ifdef DEBUG
			printf("compute_node: processing AWCET node\n");
#endif
			ctx->param_valuation(f->param_id, (union param_value_u*)&f->aw, ctx->pv_data);
			break;
		case KIND_CONST:
#ifdef DEBUG
			printf("compute_node: processing CONST node\n");
#endif
			break;
#ifdef DEBUG
		default:
			printf("compute_node: unknown node type %d\n", f->kind);
			abort();
#endif
	}
#ifdef DEBUG
	printf("compute_node: loop=%d, eta[]={", f->aw.loop_id);
	for (i = 0; i < f->aw.eta_count; i++)
		printf("%lld, ", f->aw.eta[i]);
	printf("}, others=%lld\n", f->aw.others);

#endif
}


static int loop_inner(loopinfo_t * li, int inner_id, int outer_id)
{
	if ((inner_id == outer_id) || (inner_id == -1))
		return 0;
	if (outer_id == -1)
		return 1;
	return (li->hier) (inner_id, outer_id);
}

static int loop_bound(loopinfo_t * li, int loop_id)
{
	return (li->bnd) (loop_id);
}

void awcet_seq(evalctx_t * ctx, int source_count, formula_t * source,
			   awcet_t * dest)
{
	int inner_loop = -1;
	int i, j;
	dest->others = 0;
	for (i = 0; i < source_count; i++) {
		if ((inner_loop == -1)
			|| loop_inner(ctx->li, source[i].aw.loop_id, inner_loop))
			inner_loop = source[i].aw.loop_id;
		dest->others += source[i].aw.others;
		if (dest->eta_count < source[i].aw.eta_count)
			dest->eta_count = source[i].aw.eta_count;
	}
	dest->loop_id = inner_loop;
	memset(dest->eta, 0, sizeof(int) * dest->eta_count);
	for (j = 0; j < dest->eta_count; j++) {
		long long *ptr = &dest->eta[j];
		for (i = 0; i < source_count; i++) {
			*ptr +=
				(source[i].aw.eta_count >
				 j) ? source[i].aw.eta[j] : source[i].aw.others;
		}
	}
}

void awcet_alt(evalctx_t * ctx, int source_count, formula_t * source,
			   awcet_t * dest)
{
	int i, temp_max_source, dest_idx;
	long long temp_max;
	int inner_loop = -1;
	static int tab[ALT_MAX];
	dest->others = -1;
	dest->eta_count = 0;
	for (i = 0; i < source_count; i++) {
		if ((inner_loop == -1)
			|| loop_inner(ctx->li, source[i].aw.loop_id, inner_loop))
			inner_loop = source[i].aw.loop_id;
		if (dest->others < source[i].aw.others)
			dest->others = source[i].aw.others;
	}
	dest->loop_id = inner_loop;

	dest_idx = 0;
	memset(tab, 0, sizeof(tab));

	if (source_count > ALT_MAX) {
		fprintf(stderr, "Increase ALT_MAX\n");
		abort();
	}
	/* Loop iteration count bounded by sum[i=1..source_count](source[i].eta_count) */
	while (1) {
		temp_max = -1;
		for (i = 0; i < source_count; i++) {
			if (tab[i] == source[i].aw.eta_count)
				continue;
			if (source[i].aw.eta[tab[i]] <= dest->others) {
				continue;
			}
			if (source[i].aw.eta[tab[i]] > temp_max) {
				temp_max = source[i].aw.eta[tab[i]];
				temp_max_source = i;
			}
		}
		if (temp_max == -1)
			break;
		dest->eta[dest_idx] = temp_max;
		dest_idx++;
		tab[temp_max_source]++;
		dest->eta_count++;
	}
}

void awcet_loop(evalctx_t * ctx, awcet_t * source, formula_t * dest)
{
	int i, j;
	int bound;
	long long loop_wcet = 0;
	param_value_t pv;
	if (dest->param_id != IDENT_NONE) {
		ctx->param_valuation(dest->param_id, &pv, ctx->pv_data);
		bound = pv.bound;
	} else {
		bound = loop_bound(ctx->li, dest->opdata.loop_id);
	}
	if (bound == 0) {
		dest->aw.loop_id = LOOP_TOP;
		dest->aw.eta_count = 0;
		dest->aw.eta = NULL;
		dest->aw.others = 0;
		return;
	}
	if (source->loop_id == dest->opdata.loop_id) {
		for (i = 0; (i < bound) && (i < source->eta_count); i++)
			loop_wcet += source->eta[i];
		if (i < bound)
			loop_wcet += (bound - i) * source->others;
		dest->aw.others = loop_wcet;
		dest->aw.eta_count = 0;
		dest->aw.loop_id = LOOP_TOP;
	} else {
		dest->aw.loop_id = source->loop_id;
		dest->aw.eta_count = source->eta_count / bound;
		dest->aw.others = source->others * bound;
		if (source->eta_count % bound)
			dest->aw.eta_count++;

		for (i = 0; i < dest->aw.eta_count; i++) {
			loop_wcet = 0;
			for (j = 0; j < bound; j++) {
				if ((j + i * bound) < source->eta_count) {
					loop_wcet += source->eta[j + i * bound];
				} else
					loop_wcet += source->others;
			}
			dest->aw.eta[i] = loop_wcet;
		}

	}
}

void awcet_ann(evalctx_t * ctx, awcet_t * source, formula_t * dest)
{
	int i;
	int inner_ann_takeover = 0;
	param_value_t pv;
	annotation_t *ann;

	if (dest->param_id != IDENT_NONE) {
		ctx->param_valuation(dest->param_id, &pv, ctx->pv_data);
		ann = &pv.ann;
	} else {
		ann = &dest->opdata.ann;
	}

	if (ann->count == -1) { 
		memcpy(&dest->aw, source, sizeof(awcet_t));
		return;
	}

	if ((source->eta_count != 0)
		&& loop_inner(ctx->li, ann->loop_id, source->loop_id)) {
		if ((ANN_CONFLICT_PRIORITY == ANN_INNER)
			&& (ann->count < source->eta_count)) {
			inner_ann_takeover = 1;
		} else {
			memcpy(&dest->aw, source, sizeof(awcet_t));
			return;
		}
	}

	dest->aw.eta_count = ann->count;
	for (i = 0; i < dest->aw.eta_count; i++) {
		if (i < source->eta_count) {
			dest->aw.eta[i] = source->eta[i];
		} else {
			dest->aw.eta[i] = source->others;
		}
	}
	dest->aw.others = 0;
	if ((source->eta_count == 0) || (inner_ann_takeover == 1)) {
		dest->aw.loop_id = ann->loop_id;
	} else
		dest->aw.loop_id = source->loop_id;
}

int awcet_is_equal(awcet_t * s1, awcet_t * s2)
{
	int i;

	if (s1->eta_count != s2->eta_count)
		return 0;
	if (s1->loop_id != s2->loop_id)
		return 0;
	for (i = 0; i < s1->eta_count; i++) {
		if (s1->eta[i] != s2->eta[i])
			return 0;
	}
	return s1->others == s2->others;

}

long long evaluate(formula_t *f, loopinfo_t *li, param_valuation_t pv, void *data)
{
	evalctx_t ctx;
	ctx.li = li;
	ctx.param_valuation = pv;
	ctx.pv_data = data;
	compute_node(&ctx, f);
	if (f->aw.eta_count == 0) {
		return f->aw.others;
	} else {
		return f->aw.eta[0];
	}
}

void compute_eta_count(formula_t *f) {
	switch(f->kind) {
		case KIND_ANN:
			compute_eta_count(f->children);
			f->aw.eta_count = f->opdata.ann.count;
			break;
		case KIND_LOOP:
			{
				compute_eta_count(f->children);
				f->aw.eta_count = f->children[0].aw.eta_count;
			}
			break;
		case KIND_ALT:
			{
				f->aw.eta_count = 0;
				for (int i = 0; i < f->opdata.children_count; i++) {
					compute_eta_count(f->children + i);
					f->aw.eta_count += f->children[i].aw.eta_count;
				}
			}
			break;
		case KIND_SEQ:
			{
				f->aw.eta_count = 0;
				for (int i = 0; i < f->opdata.children_count; i++) {
					compute_eta_count(f->children + i);
					if (f->aw.eta_count < f->children[i].aw.eta_count)
						f->aw.eta_count = f->children[i].aw.eta_count;
				}
			}
			break;
		case KIND_CONST:
			break;
	}
}

void writePWF(formula_t *f, FILE *out, long long *bounds) {
	switch (f->kind) {
		case KIND_CONST:
			fprintf(out, "(l%d;{", f->aw.loop_id < 0 ? 0 : f->aw.loop_id);
			for (int i = 0; i < f->aw.eta_count; i++) {
				fprintf(out, "%lld", f->aw.eta[i]);
				fprintf(out, ",");
			}
			fprintf(out, "%lld", f->aw.others);
			fprintf(out, "}) ");
			break;
		case KIND_SEQ:
			fprintf(out, "(");
			for (int i = 0; i < f->opdata.children_count; i++) {
				if (i > 0) fprintf(out, " + ");
				writePWF(f->children + i, out, bounds);
				
			}
			fprintf(out, ")");
			break;
		case KIND_ALT:
			fprintf(out, "(");
			for (int i = 0; i < f->opdata.children_count; i++) {
				if (i > 0) fprintf(out, " U ");
				writePWF(f->children + i, out, bounds);
				
			}
			fprintf(out, ")");
			break;
		case KIND_LOOP: 
			{
				fprintf(out, "(");
				writePWF(f->children, out, bounds);
				long long bound = bounds[f->opdata.loop_id];
				if (bound < 0) {
					fprintf(stderr, "warning: loop %d is unbounded\n", f->opdata.loop_id);
				}
				fprintf(out, ", (l0;{0}), l%d)^%lld", f->opdata.loop_id, bound);
			}
			break;
			
		case KIND_ANN:
			fprintf(out, "(");
			writePWF(f->children, out, bounds);
			fprintf(out, "|(l%d,%d))", f->opdata.ann.loop_id, f->opdata.ann.count);
			break;
	}
}


void writeC(formula_t *f, FILE *out, int indent) {
	static unsigned int uuid = 0;
	uuid += 1;
	char eta_str[64] = "NULL";
	if (f->aw.eta_count > 0) {
		snprintf(eta_str, sizeof(eta_str), "(long long[%d]) {0}", f->aw.eta_count);
		eta_str[sizeof(eta_str) - 1] = 0;
	}
	switch(f->kind) {
		case KIND_ANN: /* NOT TESTED TODO FIXME */
			for (int i = 0; i < indent; i++) fprintf(out, " ");
			fprintf(out, "{KIND_ANN, %d, .ann={{%d,%d}}, {%d, %d, %s, 0}, (formula_t[%d]) {\n", f->param_id, f->opdata.ann.loop_id, f->opdata.ann.count, -1, f->aw.eta_count, eta_str, 1);
			writeC(f->children, out, indent + 2);
			fprintf(out, "\n");
			for (int i = 0; i < indent; i++) fprintf(out, " ");
			fprintf(out, "}}");
			break;
		case KIND_LOOP:
			{
				for (int i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "{KIND_LOOP, %d, {%d}, {%d, %d, %s, 0}, (formula_t[%d]) {\n", f->param_id, f->opdata.loop_id, -1, f->aw.eta_count, eta_str, 1);

				writeC(f->children, out, indent + 2);
				fprintf(out, "\n");
				for (int i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "}}");
			}
			break;
		case KIND_ALT:
			{
				for (int i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "{KIND_ALT, %d, {%d}, {%d, %d, %s, 0}, (formula_t[%d]) {\n", f->param_id, f->opdata.children_count, -1, f->aw.eta_count, eta_str, f->opdata.children_count);
				for (int i = 0; i < f->opdata.children_count; i++) {
					writeC(f->children + i, out, indent + 2);
					if (i < f->opdata.children_count - 1) fprintf(out, ", ");
					fprintf(out, "\n");
				}
				for (int i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "}}");
			}
			break;
		case KIND_CONST:

			{
				for (int i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "{KIND_CONST, %d, {0}, {%d, %d, ", f->param_id, f->aw.loop_id, f->aw.eta_count);
				if (f->aw.eta_count > 0) {
					fprintf(out, "(long long[%d]) {", f->aw.eta_count);
					for (int i = 0; i < f->aw.eta_count; i++) {
						fprintf(out, "%lld, ", f->aw.eta[i]);
					}
				} else fprintf(out, "NULL");
				fprintf(out, ", %lld }, NULL}", f->aw.others);
				break;
			}
		case KIND_SEQ:
			{
				for (int i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "{KIND_SEQ, %d, {%d}, {%d, %d, %s, 0}, (formula_t[%d]) {\n", f->param_id, f->opdata.children_count, -1, f->aw.eta_count, eta_str, f->opdata.children_count);
				for (int i = 0; i < f->opdata.children_count; i++) {
					writeC(f->children + i, out, indent + 2);
					if (i < f->opdata.children_count - 1) fprintf(out, ", ");
					fprintf(out, "\n");
				}
				for (int i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "}}");
			}
			break;
	}
}

