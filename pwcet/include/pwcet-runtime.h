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

#ifndef PWCET_RUNTIME_H
#define PWCET_RUNTIME_H 1
#define PARAM_FLAG 0x40000000

typedef int (loophierarchy_t) (int l1, int l2);
typedef int (loopbounds_t) (int l1);
struct loopinfo_s {
	loophierarchy_t *hier;
	loopbounds_t *bnd;
};
typedef struct loopinfo_s loopinfo_t;

/* Abstract WCET representation */
struct awcet_s {
#define LOOP_TOP 	-1
	int loop_id;
	int eta_count;
	long long *eta;
	long long others;
};
typedef struct awcet_s awcet_t;

/* Structure representing a context annotation */
struct annotation_s {
	int loop_id;
	int count;
};
typedef struct annotation_s annotation_t;


/* Operator data (interpretation depends on operator type) */
union opdata_u {
	int children_count;			/* Number of children for Alt and Seq operators */
	int loop_id;				/* Loop bound for Loop operators */
	annotation_t ann;			/* Annotation for Ann operators */
	int coef;				/* Coefficient for Multiply by Integer */
};
typedef union opdata_u opdata_t;

/* WCET formula operator type */
#define KIND_ALT 	0
#define KIND_SEQ 	1
#define KIND_LOOP 	2
#define KIND_ANN 	3
#define KIND_CONST	4
#define KIND_AWCET  	5
#define KIND_INTMULT	6
#define KIND_BOOLMULT	7
#define KIND_STR	8
#define KIND_PARAM_LOOP 15

/* Boolean operator type */
#define BOOL_CONST 9
#define BOOL_PARAM 10
#define BOOL_CONDITIONS 11
#define BOOL_LEQ 12
#define BOOL_EQ 13
#define BOOL_BOUND 14

#define IDENT_NONE 0

struct term_s {
	int kind;
	int coef;
	int value;
};
typedef struct term_s term_t;

/* Condition representation */
struct condition_s {
	int kind;
	int int_value; // parameter id or constant
	int terms_number; // the number of terms
	term_t *terms; // terms for leq / eq
};
typedef struct condition_s condition_t;

/* WCET formula representation */
struct formula_s {
	int kind;
	/* 
	 * Identifier for parameters, set to IDENT_NONE if unused.
	 * If param_id != IDENT_NONE, then the parameter type depends on the kind field:
	 * - KIND_AWCET: The parameter represents a full abstract WCET
	 * - KIND_LOOP: The parameter represents a parametric loop bound
	 * - KIND_ANN: The parameter represents a parametric annotation
	 */
	int param_id;
	opdata_t opdata;
	awcet_t aw;	
	struct formula_s *children;
	struct condition_s *condition;
	char bool_expr[1000];
};
typedef struct formula_s formula_t;
union param_value_u {
        int bound;
        annotation_t ann;
        awcet_t aw;
};
typedef union param_value_u param_value_t;


typedef void (param_valuation_t) (int param_id, param_value_t * param_val, void *data);
typedef int bparam_value_t;
typedef bparam_value_t (bparam_valuation_t) (int bparam_id);

long long evaluate(formula_t *f, loopinfo_t *li, param_valuation_t pv, bparam_valuation_t bpv, void *data);

#endif
