#ifndef CFTREE_H
#define CFTREE_H 1
#include <typeinfo>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/otawa.h>

namespace otawa { namespace cftree {

	class DAGBNode;
	class DAGVNode;
	class DAGHNode;
	class DAGNode;
	class DAG;

	Identifier<DAGHNode*> DAG_HNODE("otawa::cftree:DAG_HNODE");
	Identifier<DAGBNode*> DAG_BNODE("otawa::cftree:DAG_BNODE");

	class CFTreeLeaf;
	class CFTreeAlt;
	class CFTreeLoop;
	class CFTreeSeq;
	class CFTree;


	class CFTree
	{
		/*
		a Control-Flow Tree, which also represents the possible ex-
		ecution paths of a program but, thanks to its tree structure, is more prone to recursive WCET
		analysis than a CFG. A Control-flow Tree is similar to Abstract Syntax Trees used in program-
		ming languages compilation, except that it represents the structure of binary code. As such, it will
		be quite natural to represent the WCET of a CFT as an arithmetic expression
		*/

		public:
			virtual CFTreeLeaf *toLeaf() = 0; /* abstract */
			virtual CFTreeAlt *toAlt() = 0;/* abstract */
			virtual CFTreeLoop *toLoop() = 0;/* abstract */
			virtual CFTreeSeq *toSeq() = 0;/* abstract */
	};

	class CFTreeLeaf : public CFTree
	{

			BasicBlock *block;

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();

			CFTreeLeaf(BasicBlock *b);
			BasicBlock *getBlock();
			int getBlockId() const;

	};


	class DAGNode
	{
		private:
			std::vector<DAGNode*> pred; // les noeuds précédents du sommet
			std::vector<DAGNode*> succ; // les noeuds suivants du sommet

		public:
			// abstract
			virtual DAGVNode *toVNode() = 0;
			virtual DAGHNode *toHNode() = 0;
			virtual DAGBNode *toBNode() = 0;

			// Add directed edge between two nodes
			void addSucc(DAGNode *s);
			void addPred(DAGNode *s);

			// Iterator for successor and predecessor
			std::vector<DAGNode*>::const_iterator succIter();
			std::vector<DAGNode*>::const_iterator predIter();
			std::vector<DAGNode*>::const_iterator succEnd();
			std::vector<DAGNode*>::const_iterator predEnd();

			// Getter for successor and predecessor
			std::vector<DAGNode*> getSucc();
			std::vector<DAGNode*> getPred();
	};

	class DAGHNode : public DAGNode
	{
		/*
		Hierarchical node represent inner loop in DAG.
		*/
		private:
			BasicBlock *header;
			DAG* sub_dag;

		public:

			// implements abstrat
			DAGHNode *toHNode();
			DAGVNode *toVNode();
			DAGBNode *toBNode();

			// Constructor
			DAGHNode(BasicBlock *b);


			void setDAG(DAG *dag);
			BasicBlock *getHeader();
			DAG *getDag();
			int getLoopId() const;
	};

	class DAGVNode : public DAGNode {
		private:
			#define VNODE_EXIT 0
			#define VNODE_NEXT 1

			int type;

		public:
			// implements abstrat
			DAGHNode *toHNode();
			DAGVNode *toVNode();
			DAGBNode *toBNode();

			DAGVNode(int _type);
			int getType() const;
	};

	class DAGBNode : public DAGNode {
		private:
			BasicBlock *block;

	  public:
			// implements abstrat
			DAGHNode *toHNode();
			DAGVNode *toVNode();
			DAGBNode *toBNode();

			DAGBNode(BasicBlock *b);
			BasicBlock *getBlock();
			int getBlockId() const;
	};


	class DAG
	{
		/*
		a Directed Acyclic Graph (DAG) represents the loop body. In this
		DAG, inner loops are replaced by hierarchical nodes, which
		themselves correspond to separate DAGs.
		*/
		private:
			// Set of DAG nodes
			std::vector<DAGNode*> all;

			// Entry node for DAG
			DAGNode *n_start;

			// Exits nodes for DAG
			std::vector<DAGNode*> n_ends;

			// Virtual node next
			DAGNode *n_next;

		public:
			// Iterator for nodes in DAG
			std::vector<DAGNode*>::const_iterator iter();
			std::vector<DAGNode*>::const_iterator end();

			// Getter and setter nodes
			void addNode(DAGNode *s);
			void setStar(DAGNode *s);
			void addEnd(DAGNode *s);
			void setNext(DAGNode *s);

			// Getter for particulars nodes
			DAGNode *getStart();
			DAGNode *getNext();
			DAGNode *getElement(unsigned i);
			DAGNode *getiEnd(int i);

			// Iterator for nodes with an exit edge
			std::vector<DAGNode*>::const_iterator iter_e();
			std::vector<DAGNode*>::const_iterator end_e();

			// Check if a node "elem" is in the DAG or not
			bool find_node(DAGNode *elem);
			~DAG();
	};

// method to print the dag
	io::Output &operator<<(io::Output &o, const DAGVNode &n);
	io::Output &operator<<(io::Output &o, DAGNode &n);
	io::Output &operator<<(io::Output &o, const DAGBNode &n);
	io::Output &operator<<(io::Output &o, const DAGHNode &n);
	io::Output &operator<<(io::Output &o, DAG &n);

// method to print the cft
	io::Output &operator<<(io::Output &o, CFTree &n);
	io::Output &operator<<(io::Output &o, CFTreeSeq &n);
	io::Output &operator<<(io::Output &o, CFTreeLeaf &n);
	io::Output &operator<<(io::Output &o, CFTreeAlt &n);
	io::Output &operator<<(io::Output &o, CFTreeLoop &n);

// method to describe a cft in a .dot
	std::string write_tree(CFTree &n, unsigned int *lab);
	std::string write_tree(CFTreeSeq &n, unsigned int *lab);
	std::string write_tree(CFTreeLeaf &n, unsigned int *lab);
	std::string write_tree(CFTreeAlt &n, unsigned int *lab);
	std::string write_tree(CFTreeLoop &n, unsigned int *lab);


class CFTreeExtractor : public Processor {
public:
	static p::declare reg;
	explicit CFTreeExtractor(p::declare &r = reg);

protected:
	void processWorkSpace(WorkSpace * /*ws*/) override;
	void configure(const PropList &props) override;
private:
	void processCFG(CFG *cfg);
};

extern p::feature EXTRACTED_CFTREE_FEATURE;

extern Identifier<DAGBNode*> DAG_BNODE;
extern Identifier<DAGHNode*> DAG_HNODE;

//declaration propriete CFTRee
// extern Identifier<CFTree*> CFTREE;


// TODO : declarer ici les classes pour manipuler le CFTree

} }

#endif
