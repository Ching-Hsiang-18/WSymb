#ifndef PWCET_RUNTIME_H
#define PWCET_RUNTIME_H 1

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
	int *eta;
	int others;
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
};
typedef union opdata_u opdata_t;

/* WCET formula operator type */
#define KIND_ALT 	0
#define KIND_SEQ 	1
#define KIND_LOOP 	2
#define KIND_ANN 	3
#define KIND_CONST	4
#define KIND_AWCET  5

#define IDENT_NONE 0

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
};
typedef struct formula_s formula_t;
union param_value_u {
        int bound;
        annotation_t ann;
        awcet_t aw;
};
typedef union param_value_u param_value_t;


typedef void (param_valuation_t) (int param_id, param_value_t * param_val, void *data);

int evaluate(formula_t *f, loopinfo_t *li, param_valuation_t pv, void *data);

#endif