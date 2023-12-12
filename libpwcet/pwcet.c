/* ----------------------------------------------------------------------------
   Copyright (C) 2022, Université de Lille, Lille, FRANCE

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
#include "pwcet.h"

// /* get a loop_bound */
// static int loop_bound(loopinfo_t * li, int loop_id)
// {
// 	return (li->bnd) (loop_id);
// }

/* function that will define the loop_hierarchy in the instantiator */
extern int loop_hierarchy(int inner, int outer);

int __idiv(int n, int d){
	int res = 0;

	/* bound: n*/
	while(n > d){
		n-=d;
		res++;
	}
	return res;
}

/* check if a loop is inside another one */
static int loop_inner(loopinfo_t * li, int inner_id, int outer_id)
{
	if ((inner_id == outer_id) || (inner_id == -1))
		return 0;
	if (outer_id == -1)
		return 1;
	//return (li->hier) (inner_id, outer_id);
	return loop_hierarchy(inner_id,outer_id);
}

awcet* prune(awcet* w){
	// in practice we just change `others` and `eta_count`, which is faster than creating a new awcet
	if(MAX_WCET_SIZE > 0 && w->eta_count >= MAX_WCET_SIZE){
		w->eta_count = MAX_WCET_SIZE-1;
		w->others = w->eta[MAX_WCET_SIZE-1];
	}
	return w;
}

awcet* sum(awcet** ws, int ws_size, loopinfo_t *li){
	int i, j;
	int inner_loop = -1;
	long long others = 0;
	// we store the result in one of the WCETs
	awcet* result = ws[0];

	/* bound: number of operands  */
	for(i = 0; i < ws_size; i++){
		if(inner_loop == -1 || loop_inner(li, ws[i]->loop_id, inner_loop)){
			inner_loop = ws[i]->loop_id;
		}
		others += ws[i]->others;
		if(result->eta_count < ws[i]->eta_count)
			result = ws[i];
	}
	result->loop_id = inner_loop;
	result->others = others;

	/* bound: MAX_WCET_SIZE-1 */
	for(i = 0; i < result->eta_count; i++){
		long long res_i = 0;

		/* bound: number of operands */
		for(j = 0; j < ws_size; j++){
			res_i += i < ws[j]->eta_count ? ws[j]->eta[i] : ws[j]->others;
		}
		result->eta[i] = res_i;
	}
	return result;
}

awcet* max(awcet** ws, int ws_size, awcet* result, loopinfo_t *li){
	int i, temp_max_source, dest_idx;
	int inner_loop = -1;
	long long temp_max;
	long long others = -1;
	//static int tab[ALT_MAX];
	int tab[ALT_MAX];

	/* bound: number of operands */
	for(i = 0; i < ws_size; i++){
		if(inner_loop == -1 || loop_inner(li, ws[i]->loop_id, inner_loop)){
			inner_loop = ws[i]->loop_id;
		}
		if(others < ws[i]->others){
			others = ws[i]->others;
		}
	}

	/* bound: ALT_MAX */
	for(i = 0; i < ALT_MAX; i++)
		tab[i] = 0;

	result->loop_id = inner_loop;
	result->others = others;
	result->eta_count = 0;
	dest_idx = 0;

	/* bound: MAX_WCET_SIZE */
	while(1){
		temp_max = -1;

		/* bound: number of operands */
		for(i = 0; i < ws_size; i++){
			if(tab[i] == ws[i]->eta_count)
				continue;
			if(ws[i]->eta[tab[i]] <= result->others)
				continue;
			if(ws[i]->eta[tab[i]] > temp_max){
				temp_max = ws[i]->eta[tab[i]];
				temp_max_source = i;
			}
		}
		if(temp_max == -1)
			break;
		if(MAX_WCET_SIZE <= 0 || result->eta_count < MAX_WCET_SIZE-1){
			result->eta[dest_idx] = temp_max;
			dest_idx++;
			tab[temp_max_source]++;
			result->eta_count++;
		}
		else{
			result->others = temp_max;
			break;
		}
	}
	return result;
}

awcet* iterate(awcet* w1, int loop_id, int bound){
	int i, j;

	// trivial case of 0 iterations
	if(bound <= 0){
		w1->eta_count = 0;
		w1->loop_id = LOOP_TOP;
		w1->others = 0;
		return w1;
	}
	//int bound = loop_bound(li, w1->loop_id);
	// same loop headers
	if(loop_id == w1->loop_id){
		long long wcet = 0;

		// eta

		/* bound: MAX_WCET_SIZE-1 */
		for(i = 0; i < bound && i < w1->eta_count; i++){
			wcet += w1->eta[i];
		}

		// others
		if(i < bound){
			wcet += w1->others * (bound - i);
		}
		w1->eta_count = 0;
		w1->loop_id = LOOP_TOP;
		w1->others = wcet;
		return w1;
	}
	// different loop headers
	else{
		//int eta_count = w1->eta_count / bound;
		int eta_count = __idiv(w1->eta_count,bound);
		long long others = w1->others * bound;
		//if(w1->eta_count % bound){
		if(w1->eta_count - bound * eta_count){
			++eta_count;
		}
		long long eta[eta_count];

		/* bound: MAX_WCET_SIZE-1 */
		for(i = 0; i < eta_count; i++){
			long long wcet = 0;

			/* bound: bound */
			for(j = 0; j < bound; j++){
				if((j + i * bound) < w1->eta_count){
					wcet += w1->eta[j + i * bound];
				}
				else{
					wcet += w1->others;
				}
			}
			eta[i] = wcet;
		}
		w1->eta_count = eta_count;
		w1->others = others;

		/* bound: MAX_WCET_SIZE-1 */
		for(i = 0; i < eta_count; i++){
			w1->eta[i] = eta[i];
		}
		return w1;
	}
}

awcet* int_multiply(awcet* w1, int coef){
	int i;

	/* bound: MAX_WCET_SIZE-1 */
	for(i = 0; i < w1->eta_count; i++){
		w1->eta[i] *= coef;
	}
	w1->others *= coef;
	return w1;
}

awcet* annot(awcet* w1, int loop_id, int limit, long long* eta, loopinfo_t* li){
	if(limit == -1){
		return w1;
	}
	else{
		/* make sure the loop is bounded */
		int changed_limit = 0;
		if(limit > MAX_WCET_SIZE-1){
			limit = MAX_WCET_SIZE-1;
			changed_limit = 1;
		}

		int i;
		int inner_ann_takeover = 0;

		if((w1->eta_count != 0) && (loop_inner(li, loop_id, w1->loop_id))){
			if((ANN_CONFLICT_PRIORITY == ANN_INNER) && (limit < w1->eta_count)){
				inner_ann_takeover = 1;
			}
			else{
				return w1;
			}
		}

		/* bound: MAX_WCET_SIZE-1 */
		for(i = 0; i < limit; i++){
			if(i < w1->eta_count){
				eta[i] = w1->eta[i];
			}
			else{
				eta[i] = w1->others;
			}
		}
		// save changes into w1
		int old_eta_count = w1->eta_count;
		w1->eta_count = limit;
		if(changed_limit){
			if(old_eta_count >= MAX_WCET_SIZE)
				w1->others = w1->eta[MAX_WCET_SIZE-1];
		}
		else{
			w1->others = 0;
		}
		w1->eta = eta;

		if((old_eta_count == 0) || (inner_ann_takeover == 1)){
			w1->loop_id = loop_id;
		}
		return w1;
	}
}
