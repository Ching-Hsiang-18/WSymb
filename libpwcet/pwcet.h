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
#ifndef PWCET_H
#define PWCET_H 1

// ======================
// Macro definitions
// ======================

/* Parameter flag */
#define PARAM_FLAG 0x40000000

/* main loop */
#define LOOP_TOP -1

/* maximum alternatives number */
#define ALT_MAX 1024

/* annotations priority management*/
#define ANN_INNER 0
#define ANN_OUTER 1
#define ANN_CONFLICT_PRIORITY ANN_INNER

/* maximum WCET size */
#define MAX_WCET_SIZE 0

// ======================
// Needed definitions
// ======================

/* keeps informations about loops */
typedef int (loophierarchy_t) (int l1, int l2);
typedef int (loopbounds_t) (int l1);
struct loopinfo_s {
	loophierarchy_t *hier;
	loopbounds_t *bnd;
};
typedef struct loopinfo_s loopinfo_t;

struct awcet_s{
	int loop_id;
	int eta_count;
	long long* eta;
	long long others;
};
typedef struct awcet_s awcet;

// ======================
// Operators definitions
// ======================

/**
 * Limits the size of WCETs to MAX_WCET_SIZE
 * This enables to bound all the loops of the instantiation program
 * @param w the WCET to prune
 */
awcet* prune(awcet* w);

/**
 * WCET sum
 * @param ws the WCETs to sum
 * @param ws_size the number of WCETs to sum
 * @param li information about loops
 * @return an awcet with the summation
 */
awcet* sum(awcet** ws, int ws_size, loopinfo_t* li); // for sequences

/**
 * WCET max
 * @param ws the WCETs to compare
 * @param ws_size the number of WCET to compare
 * @param result the resulting WCET, which contains enough space in eta
 * @param li informations about loop
 * @return an awcet with the maximum
 */
awcet* max(awcet** ws, int ws_size, awcet* result, loopinfo_t* li); // for alternatives

/**
 * WCET iteration over a multiset
 * @param w1 the WCET to iterate
 * @param loop_id the id of the loop we iterate on
 * @param bound the loop bound
 * @return the same awcet modified with the paper's formula for loops
 */
awcet* iterate(awcet* w1, int loop_id, int bound); // for loops

/**
 * WCET of an integer product
 * @param w1 the WCET to multiply
 * @param coef the coefficient to multiply each element of the awcet
 * @param the same multiset but with k * x for each value inside of it
 */
awcet* int_multiply(awcet* w1, int coef);

/**
 * WCET of the annotation system
 * @param w1 the WCET to apply the annotation to
 * @param loop_id the loop_id of the annotation
 * @param limit the maximum number of execution of the tree of w1 WCET
 * @param eta the new eta with enough places
 * @param li information about loops
 * @return the same WCET but with a different eta and a different loop_id
 */
awcet* annot(awcet* w1, int loop_id, int limit, long long* eta, loopinfo_t* li);

#endif
