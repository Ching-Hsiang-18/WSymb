#ifndef CFTREE_H
#define CFTREE_H 1
#include <typeinfo>
#include <vector>
#include <algorithm>
#include <assert.h>

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

	class CFTreeLeaf;
	class CFTreeAlt;
	class CFTreeLoop;
	class CFTreeSeq;
	class CFTree;

	Identifier<DAGHNode*> DAG_HNODE("otawa::cftree:DAG_HNODE");
	Identifier<DAGBNode*> DAG_BNODE("otawa::cftree:DAG_BNODE");

	io::Output &operator<<(io::Output &o, const DAGVNode &n);
	io::Output &operator<<(io::Output &o, DAGNode &n);
	io::Output &operator<<(io::Output &o, const DAGBNode &n);
	io::Output &operator<<(io::Output &o, const DAGHNode &n);
	io::Output &operator<<(io::Output &o, DAG &n);

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


} }

#endif
