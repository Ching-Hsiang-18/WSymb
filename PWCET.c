/* ----------------------------------------------------------------------------
   Copyright (C) 2020, Université de Lille, Lille, FRANCE

   This file is part of WSymb.

   WSymb is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation ; either version 2 of
   the License, or (at your option) any later version.

   WSymb is distributed in the hope that it will be useful, but
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
		case KIND_INTMULT:
#ifdef DEBUG
			printf("compute_node: start processing INTMULT node\n");
#endif
			compute_node(ctx, &f->children[0]);
			awcet_intmult(ctx, &f->children->aw, f);
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
		case KIND_BOOLMULT:
#ifdef DEBUG
			printf("compute_node: processing BOOLMULT node\n");
#endif
			awcet_boolmult(ctx, f, &f->aw);
			break;
		case KIND_PARAM_LOOP:
#ifdef DEBUG
			printf("compute_node: processing PARAM_LOOP node\n");
#endif
			compute_node(ctx, &f->children[0]);
			awcet_paramloop(ctx, &f->children->aw, f);
			break;
//#ifdef DEBUG
		default:
			printf("compute_node: unknown node type %d\n", f->kind);
			exit(1);;
//#endif
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

void awcet_paramloop(evalctx_t * ctx, awcet_t * source, formula_t * dest)
{
	int i, j;
	int bound = compute_loop_bound(ctx, dest->condition);
	long long loop_wcet = 0;

#ifdef DEBUG
	printf("Computed loop bound: %d\n", bound);
#endif

	// same as a regular loop
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
		// should correct problem crash when eta_count > 0
		dest->aw.eta = (long long int*) malloc(dest->aw.eta_count * sizeof(long long));
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

void awcet_intmult(evalctx_t * ctx, awcet_t * source, formula_t * dest)
{
	int i;
	(void)(ctx);
	for (i = 0; i < source->eta_count; i++) {
		dest->aw.eta[i] = source->eta[i] * dest->opdata.coef;
	}
	dest->aw.others = source->others * dest->opdata.coef;	
}

/**
 * Check the condition to execute the tree
 * @param cdt the condition list
 * @param condition_size the number of conditions to check
 */
int check_condition(evalctx_t* ctx, condition_t* cdts, int condition_size){
	for(int i = 0; i < condition_size; i++){
		condition_t* cdt = cdts+i;
		const int left = cdt->int_value;
		int right = 0;
		const int terms_size = cdt->terms_number;
		term_t* terms = cdt->terms;
		for(int j = 0; j < terms_size; j++){
			term_t* term = terms+j;
			int value = term->value;
			int coef = term->coef;
			// if parameter we get the parameter value
			if(term->kind == BOOL_PARAM){
				value = ctx->bparam_valuation(term->value);
			}
			right += coef*value;
		}
		// compare right and left values
		switch(cdt->kind){
			case BOOL_LEQ:
#ifdef DEBUG
				printf("Checking that %d <= %d\n", left, right);
#endif
				if(!(left <= right))
					return 0;
				break;
			case BOOL_EQ:
#ifdef DEBUG
				printf("Checking that %d == %d\n", left, right);
#endif
				if(!(left == right))
					return 0;
				break;
			default:
				printf("Error, unrecognized bool condition type: %d", cdt->kind);
				exit(1);
		}
	} 
	// all conditionals are checked as true
	return 1;
}

int compute_loop_bound(evalctx_t* ctx, condition_t* cdt){
	// Should only be called in the case of a parametric loop bound
	if(cdt->kind != BOOL_BOUND){
		printf("Error, compute_loop_bound called without BOOL_BOUND kind");
		exit(1);
	}
	// compute the bound
	int bound = 0;
	for(int i=0;i<cdt->terms_number;i++){
		term_t* term = cdt->terms + i;
		int value = term->value;
		int coef = term->coef;
		if(term->kind == BOOL_PARAM)
			value = ctx->bparam_valuation(value);
		bound += coef * value;
	}
	bound = bound < 0 ? 0 : bound; // a loop bound cannot be < 0, but the evaluation of the expression can
	return bound;
}

void awcet_boolmult(evalctx_t* ctx, formula_t* source, awcet_t* dest){
	// source = boolmult formula
	// dest = WCET
	// processing condition, [0] always represent the condition and [1] the WCET formula
	if(source->children[0].kind != BOOL_CONDITIONS){
		printf("Error : Boolmult first child not a boolean condition but of type: %d", source->children[0].kind);
		exit(1);
	}
	condition_t* cdts = source->children[0].condition;
	int condition_size = source->children[0].opdata.children_count;
	int result = check_condition(ctx, cdts, condition_size);
	// if condition is true then WCET of formula
	if(result == 1){
		formula_t fchild = source->children[1];
		compute_node(ctx, &fchild);
		// copy child formula WCET
		dest->eta_count = fchild.aw.eta_count;
		dest->eta = fchild.aw.eta;
		dest->others = fchild.aw.others;
		dest->loop_id = fchild.aw.loop_id;
	}
	// else theta
	else{
		// bot WCET == {0}
		dest->eta_count = 0;
		dest->eta = NULL;
		dest->others = 0;
		dest->loop_id = LOOP_TOP;
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

long long evaluate(formula_t *f, loopinfo_t *li, param_valuation_t pv, bparam_valuation_t bpv, void *data)
{
	evalctx_t ctx;
	ctx.li = li;
	ctx.param_valuation = pv;
	ctx.bparam_valuation = bpv;
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
		case KIND_INTMULT:
			compute_eta_count(f->children);
			f->aw.eta_count = f->children[0].aw.eta_count;
			break;
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
		default:
			printf("error : unrecognized formula kind (compute_eta_count) : %d\n", f->kind);
	}
}

void writePWF(formula_t *f, FILE *out, long long *bounds) { 
	switch (f->kind) {
		case KIND_CONST:
			fprintf(out, "(l:%d;{", f->aw.loop_id < 0 ? 0 : f->aw.loop_id);
			for (int i = 0; i < f->aw.eta_count; i++) {
				fprintf(out, "%lld", f->aw.eta[i]);
				fprintf(out, ",");
			}
			fprintf(out, "%lld", f->aw.others);
			fprintf(out, "}) ");
			break;
		case KIND_AWCET:
			fprintf(out, "p:%d", f->param_id);
			break;
		case KIND_SEQ:
			fprintf(out, "(");
			for (int i = 0; i < f->opdata.children_count; i++) {
				if (i > 0) fprintf(out, " + ");
				writePWF(f->children + i, out, bounds);
				
			}
			if (f->opdata.children_count == 0)
				fprintf(out, "__top;{0}");
			fprintf(out, ")");
			break;
		case KIND_ALT:
			fprintf(out, "(");
			for (int i = 0; i < f->opdata.children_count; i++) {
				if (i > 0) fprintf(out, " U ");
				writePWF(f->children + i, out, bounds);
			}
			if (f->opdata.children_count == 0)
				fprintf(out, "__top;{0}");
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

				if(strcmp(f->bool_expr, "-1") != 0){ // parametric loop
					fprintf(out, ", (__top;{0}), l:%d)^(%s)", f->opdata.loop_id, f->bool_expr);
				}
				else { // non parametric
					if (f->param_id) {
						fprintf(out, ", (__top;{0}), l:%d)^p:%d", f->opdata.loop_id, f->param_id);
					} else {
						fprintf(out, ", (__top;{0}), l:%d)^%lld", f->opdata.loop_id, bound);
					}
				}
			}
			break;
			
		case KIND_ANN:
			fprintf(out, "(");
			writePWF(f->children, out, bounds);
			fprintf(out, "|(l:%d,%d))", f->opdata.ann.loop_id, f->opdata.ann.count);
			break;

		case KIND_BOOLMULT:
			fprintf(out, "(");
			writePWF(f->children, out, bounds);
			fprintf(out, " * ");
			writePWF(f->children+1, out, bounds);
			fprintf(out, ")");
			break;
		case KIND_STR:
			fprintf(out, "(%s)",f->bool_expr);
			break;
		default:
			printf("error : unrecognized formula kind (writePWF) : %d\n", f->kind);
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

