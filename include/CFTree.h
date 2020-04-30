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

#ifndef CFTREE_H
#define CFTREE_H 1
#include <typeinfo>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/cfg/PostDominance.h>
#include <otawa/otawa.h>

#include "PWCET.h"

namespace otawa { namespace cftree {

	class DAGBNode;
	class DAGVNode;
	class DAGHNode;
	class DAGNode;
	class DAG;


	class CFTreeLeaf;
	class CFTreeAlt;
	class CFTreeLoop;
	class CFTreeSeq;
	class CFTree;

	class CFTCollection;

	class CFTCollection
	{
		private :
			std::vector<CFTree*> cfts;

		public :
			CFTCollection(std::vector<CFTree*> v);

			using iterator = typename std::vector<CFTree*> ::iterator;
			using const_iterator = const typename std::vector<CFTree*> ::iterator;

			iterator begin();
			const_iterator begin() const;

			iterator end();
			const_iterator end() const;

			bool empty() const;

			void erase(iterator);
			void clear();
	};

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
			void exportToDot(const elm::string &);
			void exportToAWCET(formula_t*, struct param_func *);
			void exportToC(io::Output&);
	};

	class CFTreeLeaf : public CFTree
	{

			Block *block;

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();

			CFTreeLeaf(Block *b);
			Block *getBlock();
			int getBlockId() const;

	};

	class CFTreeAlt : public CFTree{

		std::vector<CFTree*> alts;

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();
			
			CFTreeAlt(std::vector<CFTree*> new_alts);
			void addAlt(CFTree *s);
			std::vector<CFTree *>::const_iterator childIter();
			std::vector<CFTree *>::const_iterator childEnd();
			CFTree* getI(size_t ind);
			size_t size() { return alts.size(); }

	};

	class CFTreeSeq : public CFTree{

		std::vector<CFTree*> childs;

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();

			CFTreeSeq(std::vector<CFTree*> new_childs);
			void addChild(CFTree *s);
			std::vector<CFTree *>::const_iterator childIter();
			std::vector<CFTree *>::const_iterator childEnd();
			CFTree* getI(size_t ind);
			size_t size() { return childs.size(); }
	};

	class CFTreeLoop : public CFTree{

			BasicBlock *header;
			CFTree* bd; // body
			CFTree* ex; // exit
			int n;

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();

			CFTreeLoop(BasicBlock *h, int bound, CFTree* n_bd, CFTree* n_ex);

			BasicBlock *getHeader();
			int getHeaderId() const;
			CFTree *getBody();
			CFTree *getExit();
			void changeBound(int bound);

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
			virtual ~DAGNode() { };

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
			virtual ~DAGHNode() { };

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
			virtual ~DAGVNode() { };
			DAGHNode *toHNode();
			DAGVNode *toVNode();
			DAGBNode *toBNode();

			DAGVNode(int _type);
			int getType() const;
	};

	class DAGBNode : public DAGNode {
		private:
			Block *block;
			CFG *callee;
			bool _synth;

	  public:
			// implements abstrat
			virtual ~DAGBNode() { };
			DAGHNode *toHNode();
			DAGVNode *toVNode();
			DAGBNode *toBNode();

			bool isSynth();

			DAGBNode(Block *b);
			Block *getBlock();
			CFG *getCallee();
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

			DAGNode *n_virt_next;
			DAGNode *n_virt_exit;

		public:
			// Iterator for nodes in DAG
			std::vector<DAGNode*>::const_iterator iter();
			std::vector<DAGNode*>::const_iterator end();

			void visit(std::function<void(DAGNode*)>&, bool recursive=false);

			// Getter and setter nodes
			void addNode(DAGNode *s);
			void setStart(DAGNode *s);
			void setNext(DAGNode *s);

			// Getter for particulars nodes
			DAGNode *getStart();
			DAGNode *getElement(size_t i);

			DAGNode *getVirtNext();
			DAGNode *getVirtExit();
			void setVirtNext(DAGNode *s);
			void setVirtExit(DAGNode *s);


			// Iterator for nodes with an exit edge
			std::vector<DAGNode*>::const_iterator iter_e();
			std::vector<DAGNode*>::const_iterator end_e();

			// Check if a node "elem" is in the DAG or not
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

class CFTreeExtractor : public Processor {
public:
	static p::declare reg;
	explicit CFTreeExtractor(p::declare &r = reg);

protected:
	void processWorkSpace(WorkSpace * /*ws*/) override;
	void configure(const PropList &props) override;
private:
	CFTree* processCFG(CFG *cfg);
	DAG* toDAG(CFG *cfg, BasicBlock *l_h);

};

extern p::feature EXTRACTED_CFTREE_FEATURE;

//declaration propriete CFTRee
extern Identifier<CFTree*> CFTREE;
extern Identifier<DAGHNode*> DAG_HNODE;
extern Identifier<DAGBNode*> DAG_BNODE;
extern Identifier<int> ANN_TIME;
extern Identifier<int> ANN_COUNT;



} }

#endif
