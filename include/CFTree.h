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
	class DAG
	{
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

	class CFTreeLeaf;
	class CFTreeAlt;
	class CFTreeLoop;
	class CFTreeSeq;
	class CFTree;

	Identifier<DAGHNode*> DAG_HNODE("otawa::cftree:DAG_HNODE");
	Identifier<DAGBNode*> DAG_BNODE("otawa::cftree:DAG_BNODE");

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

class DAGNode;
class DAGBNode;
class DAGHNode;

extern Identifier<DAGBNode*> DAG_BNODE;
extern Identifier<DAGHNode*> DAG_HNODE;

//declaration propriete CFTRee
// extern Identifier<CFTree*> CFTREE;


// TODO : declarer ici les classes pour manipuler le CFTree

} }

#endif
