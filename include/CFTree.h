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
#include <set>
#include <map>
#include <list>

#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/cfg/PostDominance.h>
#include <otawa/otawa.h>

#include "PWCET.h"

#include <otawa/cache/features.h>
#include <otawa/prog/Inst.h>
#include <otawa/ilp/AbstractSystem.h>

#include <chrono>
#include <thread>
#include <mutex>

#include <otawa/etime/features.h>

// IP activates infeasible paths
//#define IP 0
// H1 and H2 activates heuristics 1 and 2
// 1 gives better precision but is slower than 2 (more aggressive)
#define H1 0
#define H2 0

// pipeline
#define PIPELINE 0
// cache
#define ICACHE 0

#define CSV_DELIM ";" //< csv delimiter ton split strings

using namespace otawa::cache;
using namespace otawa::ilp;

namespace otawa
{
	namespace cftree
	{
		/* infeasible paths implement */
		typedef std::set<long> PseudoPath;
		typedef std::set<long> Constraint;

		#define CONSTRAINTS_FILE "/home/sandro/Documents/Test/ipgenerator/bench.txt"
		#define LINE_DELIMITER "\n"
		std::set<long> convertToSet(std::vector<long> v){
			std::set<long> s;
			for (const long &i: v) {
				s.insert(i);
			}
			return s;
		}
		std::set<Constraint> getConstraintsFromFile(){
			std::ifstream file;
			file.open(CONSTRAINTS_FILE);
			std::string line_delimiter = LINE_DELIMITER;
			std::set<Constraint> constraints;
			
			// get each line of the file separated
			std::vector<std::string> lines;
			std::string str;
			while (std::getline(file, str))
			{
				// Line contains string of length > 0 then save it in vector
				if(str.size() > 0){}
					lines.push_back(str);
			}

			// get each constraint by splitting lines
			for(auto i = lines.begin(); i != lines.end(); i++){
				std::string line = *i;
				std::stringstream ss(line);
				std::string word;
				std::vector<long> splittedLine;

				while (ss >> word) {
					int node = std::stoi(word);
					splittedLine.push_back(node);
					// cout << node << " ";
				}
				//cout << endl;

				splittedLine.pop_back();

				Constraint c = convertToSet(splittedLine);

				if(c.size() > 0){
					// cout << "limit: " << limit << endl;
					constraints.insert(c);
				}
			}

			return constraints;
		}

		class DAGBNode;
		class DAGVNode;
		class DAGHNode;
		class DAGNode;
		class DAG;

		class CFTreeLeaf;
		class CFTreeAlt;
		class CFTreeConditionalAlt;
		class CFTreeLoop;
		class CFTreeSeq;
		class CFTree;

		class CFTCollection;

		class CFTCacheMutator;

		class BranchCondition;
		class ConditionParser;

		class LoopBound;
		class LoopBoundParser;

		class CFTCollection
		{
		private:
			std::vector<CFTree *> cfts;

		public:
			CFTCollection(std::vector<CFTree *> v);

			using iterator = typename std::vector<CFTree *>::iterator;
			using const_iterator = const typename std::vector<CFTree *>::iterator;

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

		/* infeasible paths implement */
		private:
			std::map<PseudoPath, CFTree*> pseudoTrees;
			std::set<Constraint>* constraints = new std::set<Constraint>();
			std::set<PseudoPath> ppaths;
			std::mutex mtx;

		protected:
			CFTree* parent = nullptr;

		public:
			virtual CFTreeLeaf *toLeaf() = 0; /* abstract */
			virtual CFTreeAlt *toAlt() = 0;	  /* abstract */
			virtual CFTreeLoop *toLoop() = 0; /* abstract */
			virtual CFTreeSeq *toSeq() = 0;	  /* abstract */
			virtual CFTreeConditionalAlt* toConditionalAlt() = 0; /* abstract */

			virtual void replace(CFTree* oldTree, CFTree* newTree) = 0; /* abstract, infeasible paths implement */

			void exportToDot(const elm::string &);
			/**
			 * Enables to export the WCET as a parametric formula
			 * @param loopexit tell if we are in a loop exit tree, this is used to tighten the WCET when using the pipeline during the analysis
			 * @param lastBlock tell if we are in the last block (usefull for alternatives)
			 */
			void exportToAWCET(formula_t *, struct param_func *, bool loopexit = false, bool lastBlock = false);
			void exportToC(io::Output &);

			/* infeasible paths implement */
			void putPPath(PseudoPath pseudoPath, CFTree* pseudoTree);
			std::map<PseudoPath, CFTree*> getPPaths();
			void exportToFeasibleWCET(formula_t *, struct param_func *);

			std::set<Constraint>* getConstraints() {return constraints;};
			void putConstraint(Constraint c){constraints->insert(c);};

			CFTree* getParent();
			void setParent(CFTree* parent);

			std::set<PseudoPath> getPseudoPaths();
			void setPseudoPaths(std::set<PseudoPath> ppaths);

		};

		class CFTreeLeaf : public CFTree
		{

			CFTreeLeaf* original;
			std::vector<CFTreeLeaf*> avatars;
			Block *block;

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();
			CFTreeConditionalAlt* toConditionalAlt(); /* conditional alts management */
			void replace(CFTree* oldTree, CFTree* newTree); /* abstract, infeasible paths implement */

			CFTreeLeaf(Block *b);
			Block *getBlock();
			int getBlockId() const;
			void addAvatar(CFTreeLeaf* avatar){ avatars.push_back(avatar); }
			std::vector<CFTreeLeaf*> getAvatars(){ return avatars; }
			void freeAvatars(){ avatars.clear(); }
			void setOriginal(CFTreeLeaf* leaf){ original = leaf; }
			CFTreeLeaf* getOriginal(){ return original; }
		};

		class CFTreeAlt : public CFTree
		{

			std::vector<CFTree *> alts;

		public:
			// implements abstract
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();
			CFTreeConditionalAlt* toConditionalAlt(); /* conditional alts management */
			void replace(CFTree* oldTree, CFTree* newTree); /* abstract, infeasible paths implement */

			CFTreeAlt(std::vector<CFTree *> new_alts);
			void addAlt(CFTree *s);
			std::vector<CFTree *>::const_iterator childIter();
			std::vector<CFTree *>::const_iterator childEnd();
			CFTree *getI(size_t ind);
			size_t size() { return alts.size(); }
		};

		/**
		 * Represent conditional alt nodes
		 * This class is really similar to CFTreeAlt, but with conditions on each child
		 */
		class CFTreeConditionalAlt : public CFTree {
			private:
				std::vector<CFTree*> alts; //< The children nodes of the alt
				std::map<int,std::string> conditions; //< Map the index of the children vector with the condition associated with the tree
			public:
				// implements abstract
				CFTreeLeaf* toLeaf();
				CFTreeAlt* toAlt();
				CFTreeLoop* toLoop();
				CFTreeSeq* toSeq();
				CFTreeConditionalAlt* toConditionalAlt();
				void replace(CFTree* oldTree, CFTree* newTree); /* abstract, infeasible paths implement */

				/**
				 * Constructor
				 * @param new_alts the alternatives to put as children
				 * @param new_conditions the conditions to put for each child
				 */
				CFTreeConditionalAlt(std::vector<CFTree*> new_alts, std::map<int,std::string> new_conditions);

				/**
				 * Add a new alternative to the children list
				 * @param alt the new alternative
				 * @param condition the condition corresponding to the new alternative
				 */
				void addAlt(CFTree* alt, std::string condition);

				/**
				 * Children accessor
				 * @return the children of the nodes
				 */
				std::vector<CFTree*> getChildren();

				/**
				 * Conditions accessor
				 * @return the conditions associated to the children
				 */
				std::map<int,std::string> getConditions();

		};

		class CFTreeSeq : public CFTree
		{

			std::vector<CFTree *> childs;
			std::vector<CFTree *> unconstrained_childs;

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();
			CFTreeConditionalAlt* toConditionalAlt(); /* conditional alts management */
			void replace(CFTree* oldTree, CFTree* newTree); /* abstract, infeasible paths implement */

			CFTreeSeq(std::vector<CFTree *> new_childs);
			void addChild(CFTree *s);
			std::vector<CFTree *>::const_iterator childIter();
			std::vector<CFTree *>::const_iterator childEnd();
			CFTree *getI(size_t ind);
			size_t size() { return childs.size(); }

			std::vector<CFTree*> getChilds(){return childs;}
			void setChilds(std::vector<CFTree*> new_childs){childs = new_childs;}

			std::vector<CFTree*> getUnconstrainedChilds(){return unconstrained_childs;}
			void setUnconstrainedChilds(std::vector<CFTree*> new_childs){unconstrained_childs = new_childs;}
		};

		class CFTreeLoop : public CFTree
		{

			BasicBlock *header;
			CFTree *bd; // body
			CFTree *ex; // exit
			int n;

			bool parametric;
			std::string expression; // parametric loop bound

		public:
			// implements abstrat
			CFTreeLeaf *toLeaf();
			CFTreeAlt *toAlt();
			CFTreeLoop *toLoop();
			CFTreeSeq *toSeq();
			CFTreeConditionalAlt* toConditionalAlt(); /* conditional alts management */
			void replace(CFTree* oldTree, CFTree* newTree); /* abstract, infeasible paths implement */

			CFTreeLoop(BasicBlock *h, int bound, CFTree *n_bd, CFTree *n_ex);
			/**
			 * Constructor of loops with parametric loop bounds
			 * @param h the header of the loop
			 * @param expression the loop bound expression
			 * @param n_bd the body of the loop
			 * @param n_ex the tree to exit the loop
			 */
			CFTreeLoop(BasicBlock *h, std::string expression, CFTree* n_bd, CFTree* n_ex);

			BasicBlock *getHeader();
			int getHeaderId() const;
			CFTree *getBody();
			CFTree *getExit();
			void changeBound(int bound);
			int getBound() { return n; }

			/**
			 * Tells if it has a parametric loop bound
			 */
			bool isParametric();

			/**
			 * Get parametric expression for the loop bound
			 */
			std::string getParametricBound();
		};

		class DAGNode
		{
		private:
			std::vector<DAGNode *> pred; // les noeuds précédents du sommet
			std::vector<DAGNode *> succ; // les noeuds suivants du sommet

		public:
			// abstract
			virtual DAGVNode *toVNode() = 0;
			virtual DAGHNode *toHNode() = 0;
			virtual DAGBNode *toBNode() = 0;
			virtual ~DAGNode(){};

			// Add directed edge between two nodes
			void addSucc(DAGNode *s);
			void addPred(DAGNode *s);

			// Iterator for successor and predecessor
			std::vector<DAGNode *>::const_iterator succIter();
			std::vector<DAGNode *>::const_iterator predIter();
			std::vector<DAGNode *>::const_iterator succEnd();
			std::vector<DAGNode *>::const_iterator predEnd();

			// Getter for successor and predecessor
			std::vector<DAGNode *> getSucc();
			std::vector<DAGNode *> getPred();
		};

		class DAGHNode : public DAGNode
		{
			/*
		Hierarchical node represent inner loop in DAG.
		*/
		private:
			BasicBlock *header;
			DAG *sub_dag;

		public:
			virtual ~DAGHNode(){};

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

		class DAGVNode : public DAGNode
		{
		private:
#define VNODE_EXIT 0
#define VNODE_NEXT 1

			int type;

		public:
			// implements abstrat
			virtual ~DAGVNode(){};
			DAGHNode *toHNode();
			DAGVNode *toVNode();
			DAGBNode *toBNode();

			DAGVNode(int _type);
			int getType() const;
		};

		class DAGBNode : public DAGNode
		{
		private:
			Block *block;
			CFG *callee;
			bool _synth;

		public:
			// implements abstrat
			virtual ~DAGBNode(){};
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
			std::vector<DAGNode *> all;

			// Entry node for DAG
			DAGNode *n_start;

			// Exits nodes for DAG
			std::vector<DAGNode *> n_ends;

			// Virtual node next
			DAGNode *n_next;

			DAGNode *n_virt_next;
			DAGNode *n_virt_exit;

		public:
			// Iterator for nodes in DAG
			std::vector<DAGNode *>::const_iterator iter();
			std::vector<DAGNode *>::const_iterator end();

			void visit(std::function<void(DAGNode *)> &, bool recursive = false);

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
			std::vector<DAGNode *>::const_iterator iter_e();
			std::vector<DAGNode *>::const_iterator end_e();

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

		class CFTreeExtractor : public Processor
		{
		public:
			static p::declare reg;
			explicit CFTreeExtractor(p::declare &r = reg);

		protected:
			void processWorkSpace(WorkSpace * /*ws*/) override;
			void configure(const PropList &props) override;

		private:
			CFTree *processCFG(CFG *cfg);
			DAG *toDAG(CFG *cfg, BasicBlock *l_h);
		};

		extern p::feature EXTRACTED_CFTREE_FEATURE;

		//declaration propriete CFTRee
		extern Identifier<CFTree *> CFTREE;
		extern Identifier<DAGHNode *> DAG_HNODE;
		extern Identifier<DAGBNode *> DAG_BNODE;
		extern Identifier<int> ANN_TIME;
		extern Identifier<int> ANN_COUNT;


		/// ******************************
		/// CACHE MUTATOR CLASS
		/// Allows to represents the cache into a CFTree
		/// ******************************
		class CFTCacheMutator
		{
		public:
			CFTCacheMutator();
			CFTree *transform(CFTree *tree);

		private:
			CFTreeSeq *transformLeaf(CFTreeLeaf *leaf);
			CFTree *transformLBlock(cache::LBlock *lBlock, Block* block);
		};

		///
		/// This allows to create a fake instruction that will never be used but which is required to extend the BasicBlock Object
		///
		class NullInst : public Inst
		{
		public:
			NullInst(){};
			inline bool isBranch() { return true; };
			kind_t kind() override { return otawa::Inst::IS_UNKNOWN; }
			address_t address() const override { return Address::null; }
			t::uint32 size() const { return 0; }
		};

		///
		/// This enum allows to get more information about the created basic cache blocs
		///
		enum CacheBlocType
		{
			HIT,
			MISS
		};

		///
		/// Transforms the enum into a string
		///
		std::string getCacheBlocType(CacheBlocType t){
			if(t == HIT){
				return "HIT";
			}
			if(t == MISS){
				return "MISS";
			}
			return "UNDEFINED";
		}

		std::string getCacheBlocCategory(category_t t){
			switch (t)
			{
			case ALWAYS_HIT:
				return "AH";
				break;
			case ALWAYS_MISS:
				return "AM";
				break;
			case FIRST_HIT:
				return "FH";
				break;
			case FIRST_MISS:
				return "FM";
				break;
			case NOT_CLASSIFIED:
				return "NC";
				break;
			case INVALID_CATEGORY:
				return "IC";
				break;
			default:
				return "error";
				break;
			}
		}

		///
		/// Allows to create a basic block foreach cache hit or miss
		/// The goal of this is to be able to use the block into a tree but also to use it like a regular basic block
		///
		class BasicCacheBlock : public BasicBlock
		{
		private:
			static int blockCount;
			int blockId;
			int bbid;
			int lBlock;
			CacheBlocType type;
			static int penalty;
			category_t contextCategory;
			int loop_id;

		public:
			BasicCacheBlock(const Array<Inst *> &insts, int lBlockId, int basicBlockId, CacheBlocType cacheType, category_t lBlockCategory, int loopId) : BasicBlock(insts)
			{
				blockId = blockCount++;
				lBlock = lBlockId;
				type = cacheType;
				contextCategory = lBlockCategory;
				loop_id = loopId;
				bbid = basicBlockId;
				//cout << "\t\tCreated BasicCacheBloc with id : " << blockId << endl;
			}
			inline int id(void) const { return blockId; }
			int getLBlockId() const { return lBlock; }
			CacheBlocType getType() const { return type; }
			static int getPenalty() { return penalty; }
			static void setPenalty(int value) {penalty = value;}
			category_t getContextCategory() const { return contextCategory; }
			int getBasicBlockId() const{ return bbid;}
			int getLoopId() const{return loop_id;}
		};

		/// ************************
		/// PSEUDO PATHS
		/// ************************
		void printPseudoPath(PseudoPath ppath);
		void printPseudoPaths(std::set<PseudoPath> ppaths);
		std::set<long> merge(std::set<std::set<long>> nodeSets);

		///
		/// get constraints containing a specific node
		///
		class ConstraintCloner{
			public:
				std::set<Constraint> getConstraintsContaining(int id, std::set<Constraint> cs);
				std::set<Constraint> replace(long oldId, long newId, std::set<Constraint> cs);
		};

		///
		/// Allows to attach constraints to the right node
		///
		class LCAFinder{
			private:
				int idIndex;
				std::map<CFTreeLeaf*, int> id;
				std::set<int> seen;
				std::map<int, CFTreeLeaf*> leaves;
				std::set<CFTreeLeaf*> allCopies;
				CFTree* currentParent = nullptr;
				CFTree* findLCA(CFTree* tree1, CFTree* tree2);

				CFTreeLeaf* copy(CFTreeLeaf* toReplace, CFTree* parent);
				int putNewLeaf(CFTreeLeaf* leaf){id.emplace(leaf, idIndex--); allCopies.insert(leaf); return idIndex+1; }
			public:
				LCAFinder(){idIndex = -1;}
				std::set<Constraint> fillNodeParents(CFTree* tree, std::set<long> nodes, std::set<Constraint> cs);
				CFTree* findLCA(Constraint c);
				int getLeafId(CFTreeLeaf* leaf){ if(id.find(leaf) != id.end()) return id.find(leaf)->second; else return 0;}
				void replaceLeafId(CFTreeLeaf* leaf, CFTreeLeaf* newLeaf);
				void empty(){ leaves.clear(); seen.clear(); id.clear(); allCopies.clear(); }
				std::set<CFTreeLeaf*> copies(){return allCopies;}
		};
		LCAFinder finder;
#ifdef IP
		std::set<Constraint> allConstraints = IP != 0 ? getConstraintsFromFile() : std::set<Constraint>();
#endif
		///
		/// Allows to build all pseudo paths for a tree
		///
		bool ppathsFound = false;
		class PseudoPathsBuilder{
			private:
				std::set<PseudoPath> getAltPaths(std::set<Constraint> cs, std::vector<otawa::cftree::CFTree *>::const_iterator children, std::vector<CFTree *>::const_iterator end);
				std::set<PseudoPath> getSeqPaths(std::set<Constraint> cs, std::vector<otawa::cftree::CFTree *>::const_iterator children, std::vector<CFTree *>::const_iterator end);
				std::set<PseudoPath> removeIPaths(std::set<Constraint> cs, std::set<PseudoPath> ppaths);
			public:
				PseudoPathsBuilder(){};
				std::set<PseudoPath> getPPaths(std::set<Constraint> cs, CFTree* node);
		};

		/// ************************
		/// PSEUDO TREES
		/// ************************
		int infeasibleTree=0;

		///
		/// Allows to build pseudo trees corresponding to pseudo paths
		///
		class PseudoTreeBuilder{
			private:
				std::vector<CFTree*> pseudoSeq(PseudoPath ppath, std::vector<otawa::cftree::CFTree *>::const_iterator childs, std::vector<otawa::cftree::CFTree *>::const_iterator end, std::set<Constraint> cs);
				std::vector<CFTree*> findChilds(PseudoPath ppath, std::vector<otawa::cftree::CFTree *>::const_iterator childs, std::vector<otawa::cftree::CFTree *>::const_iterator end, std::set<Constraint> cs);
				std::vector<CFTree*> pseudoAlt(PseudoPath ppath, std::vector<otawa::cftree::CFTree *>::const_iterator childs, std::vector<otawa::cftree::CFTree *>::const_iterator end, std::set<Constraint> cs);
				bool remove(PseudoPath ppath, CFTree* node, std::set<Constraint> cs);
			public:
				PseudoTreeBuilder(){};
				CFTree* pseudoTree(PseudoPath ppath, CFTree* node, std::set<Constraint> cs);
		};

		class NoPathException : public otawa::Exception{
			public:
				NoPathException() : otawa::Exception("Error : could not find a feasible path"){}
		};

		class ParallelizationException : public otawa::Exception{
			public:
				ParallelizationException() : otawa::Exception("Error : parallelization has deleted some paths"){}
		};

		class InvalidConstraintException : public otawa::Exception{
			public:
				InvalidConstraintException(elm::string message) : otawa::Exception(message){}
		};

		/* Performance measurements */
		class Timer{
			private:
				std::chrono::_V2::system_clock::time_point begin;
				std::chrono::microseconds duration;
			public:
				void start();
				void stop();
				void printDuration();
		};

		/**
		 * Represent conditions interface
		 * We use it to import polyhedra analysis results
		 */
		class BranchCondition{
			private:
				address_t address; //< Address of the basic block containing the branching instruction
				bool taken; //< Tell if this is the case when we branch or not
				std::string conditions; //< The condition as a string
			public:
				/**
				 * Constructor
				 * @param branchingBlockAddress the address of the basic block containing the instruction
				 * @param taken tell if the condition is when we take the branch or not
				 * @param conditions the conditions as a string
				 */
				BranchCondition(address_t branchingBlockAddress, bool taken, std::string conditions);
				/**
				 * Accessors
				 */
				address_t getAddress();
				bool isTaken();
				std::string getConditions();
		};
		
		/**
		 * A simple parser to parse input conditions
		 */
		class ConditionParser{
			public:
				/**
				 * Read constraints from file
				 * @param fileName the file to read
				 * @return a map containint the conditions for each branching instruction
				 */
				std::map<address_t, std::vector<std::map<bool,BranchCondition*>*>*> readFromFile(const char* fileName);
				/**
				 * Push conditions on the CFG
				 * It means that we put all conditions on the successors of branching nodes
				 * It enables to only change minor details when building the CFT from DAG
				 * @param coll the CFG collection
				 * @param conditionsMap the map containing the conditions
				 */
				void pushConditionsOnCFGs(const CFG* entry, std::map<address_t, std::vector<std::map<bool,BranchCondition*>*>*>* conditionsMap, std::map<int, LoopBound>* loopBounds);
				/**
				 * Find the condition of a branch if exist
				 * @param branch the branch where we need to find the condition
				 * @return the condition if any was found, nullptr otherwise
				 */
				BranchCondition* findConditionOfBranch(CFTree* branch);
			private:
				/**
				 * Filtering conditions in order to remove potential duplications and rthus better execution times later
				 * @param cdts the conditions as a vector
				 */
				std::map<address_t, std::vector<std::map<bool,BranchCondition*>*>*> filterCdts(std::vector<BranchCondition*> cdts);
		};

		/**
		 * Identifier to store conditions on basic blocks
		 * The basic block containing the condition should be the destination of an edge
		 */
		extern Identifier<BranchCondition*> CONDITION;
		/**
		 * Identifier to map the function parameter (first int) to the output formula id (second int)
		 * This identifier should be used on the CFG entry block of each function containing conditional alternative
		 */
		// extern Identifier<std::map<int,int>> FUNCTION_PARAM_MAPPING;
	
		/**
		 * Loop bound representation
		 */
		class LoopBound{
			private:
				int loopId; //< the id of the loop
				bool parametric; //< tells if the bound is parametric or static
				int constant; //< the bound if non parametric
				std::string expression; //< the bound if parametric
			public:
				/**
				 * Static constructor
				 * @param loopId the id of the loop header
				 * @param constant the loop bound as an integer
				 */
				LoopBound(int loopId, int constant);

				/**
				 * Parametric constructor
				 * @param loopId the id of the loop header
				 * @param expression the parametric loop bound expression
				 */
				LoopBound(int loopId, std::string expression);
				
				/**
				 * Tells if the bound is parametric or static
				 * @returns true is parametric, false otherwise
				 */
				bool isParametric();

				/**
				 * Returns the loop bound as a constant
				 * @returns the loop bound as an integer
				 */
				int getConstant();

				/**
				 * Returns the parametric loop bound
				 * @returns the loop bound as an expression
				 */
				std::string getExpression();
		};

		/**
		 * A simple parser to read the loop bounds from an external file
		 */
		class LoopBoundParser{
			public:
				/**
				 * Reads the loop bounds from LOOP_BOUNDS_FILE and stores it in LOOP_BOUNDS
				 */
				std::map<int,LoopBound> readFromFile(const char* fileName);
		};

		/**
		 * Stores a loop bound on the header of the loop
		 */
		extern Identifier<LoopBound> LOOP_BOUND;

		/**
		 * Tell if a block is located just after an Alt
		 */
		extern Identifier<bool> IS_AFTER_ALT;

		/**
		 * Tell if a block is located at the end of an alternative of an ALT
		 */
		extern Identifier<bool> IS_LAST_IN_ALT;

		/**
		 * Tells if a block is a pipeline edge representation
		 */
		extern Identifier<bool> IS_PIPELINE_EDGE;

		/**
		 * Stores the edge that the block is representing
		 */
		extern Identifier<Edge*> EDGE_ALIAS;
		extern Identifier<bool> IS_AA_LOOPEXIT;

		/**
		 * Enables to use the PFL file from anywhere in the plugin
		 */
		struct param_func* PFL = nullptr;

		/**
		 * Parametric function WCET management
		 */
		class FParamManager {
			public:
				bool isParam(Block* b);
		};
	} // namespace cftree
} // namespace otawa

#endif
