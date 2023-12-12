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

#include "include/CFTree.h"
#include "include/PWCET.h"
#include <otawa/ipet.h>
#include <otawa/flowfact/features.h>

#include <otawa/cache/features.h>
#include <otawa/cache/cat2/features.h>

#include <otawa/ilp/Var.h>

#include <string>

namespace otawa
{

	namespace cftree
	{
		// definition propriete cftree

		//--------------------
		//	CLASS CFTREELEAF
		//--------------------

		CFTreeLeaf *CFTreeLeaf::toLeaf()
		{
			return (CFTreeLeaf *)this;
		}
		CFTreeAlt *CFTreeLeaf::toAlt() { return NULL; }	  /* abstract */
		CFTreeLoop *CFTreeLeaf::toLoop() { return NULL; } /* abstract */
		CFTreeSeq *CFTreeLeaf::toSeq() { return NULL; }	  /* abstract */
		CFTreeConditionalAlt *CFTreeLeaf::toConditionalAlt() { return NULL; }

		CFTreeLeaf::CFTreeLeaf(Block *b)
		{
			block = b;
		}

		Block *CFTreeLeaf::getBlock() { return block; }
		int CFTreeLeaf::getBlockId() const { return block->id(); }

		//--------------------
		//	CLASS CFTREEALT
		//--------------------
		CFTreeAlt::CFTreeAlt(std::vector<CFTree *> new_alts)
		{
			alts = new_alts;
		}

		CFTreeAlt *CFTreeAlt::toAlt()
		{
			return (CFTreeAlt *)this;
		}
		CFTreeLeaf *CFTreeAlt::toLeaf() { return NULL; } /* abstract */
		CFTreeLoop *CFTreeAlt::toLoop() { return NULL; } /* abstract */
		CFTreeSeq *CFTreeAlt::toSeq() { return NULL; }	 /* abstract */
		CFTreeConditionalAlt *CFTreeAlt::toConditionalAlt() { return NULL; }

		void CFTreeAlt::addAlt(CFTree *s)
		{
			alts.push_back(s);
		}

		std::vector<CFTree *>::const_iterator CFTreeAlt::childIter()
		{
			return alts.begin();
		}

		std::vector<CFTree *>::const_iterator CFTreeAlt::childEnd()
		{
			return alts.end();
		}

		CFTree *CFTreeAlt::getI(size_t ind)
		{
			assert(ind < alts.size());
			return alts[ind];
		}

		//--------------------
		// CLASS CFTREECONDITIONALALT
		//--------------------
		CFTreeLeaf *CFTreeConditionalAlt::toLeaf() { return NULL; }
		CFTreeAlt *CFTreeConditionalAlt::toAlt() { return NULL; }
		CFTreeSeq *CFTreeConditionalAlt::toSeq() { return NULL; }
		CFTreeLoop *CFTreeConditionalAlt::toLoop() { return NULL; }
		CFTreeConditionalAlt *CFTreeConditionalAlt::toConditionalAlt()
		{
			return (CFTreeConditionalAlt *)this;
		}

		CFTreeConditionalAlt::CFTreeConditionalAlt(std::vector<CFTree *> new_alts, std::map<int, std::string> new_conditions)
		{
			alts = new_alts;
			conditions = new_conditions;
		}

		void CFTreeConditionalAlt::addAlt(CFTree *alt, std::string condition)
		{
			int id = alts.size();
			alts.push_back(alt);
			conditions.emplace(id, condition);
		}

		std::vector<CFTree *> CFTreeConditionalAlt::getChildren()
		{
			return alts;
		}

		std::map<int, std::string> CFTreeConditionalAlt::getConditions()
		{
			return conditions;
		}

		//--------------------
		//	CLASS CFTREESEQ
		//--------------------

		CFTreeSeq *CFTreeSeq::toSeq()
		{
			return (CFTreeSeq *)this;
		}
		CFTreeLeaf *CFTreeSeq::toLeaf() { return NULL; } /* abstract */
		CFTreeLoop *CFTreeSeq::toLoop() { return NULL; } /* abstract */
		CFTreeAlt *CFTreeSeq::toAlt() { return NULL; }	 /* abstract */
		CFTreeConditionalAlt *CFTreeSeq::toConditionalAlt() { return NULL; }

		CFTreeSeq::CFTreeSeq(std::vector<CFTree *> new_childs)
		{
			childs = new_childs;
		}

		void CFTreeSeq::addChild(CFTree *s)
		{
			childs.push_back(s);
		}

		std::vector<CFTree *>::const_iterator CFTreeSeq::childIter()
		{
			return childs.begin();
		}

		std::vector<CFTree *>::const_iterator CFTreeSeq::childEnd()
		{
			return childs.end();
		}

		CFTree *CFTreeSeq::getI(size_t ind)
		{
			assert(ind < childs.size());
			return childs[ind];
		}

		//--------------------
		//	CLASS CFTREELOOP
		//--------------------

		CFTreeLoop *CFTreeLoop::toLoop()
		{
			return (CFTreeLoop *)this;
		}
		CFTreeLeaf *CFTreeLoop::toLeaf() { return NULL; } /* abstract */
		CFTreeSeq *CFTreeLoop::toSeq() { return NULL; }	  /* abstract */
		CFTreeAlt *CFTreeLoop::toAlt() { return NULL; }	  /* abstract */
		CFTreeConditionalAlt *CFTreeLoop::toConditionalAlt() { return NULL; }

		CFTreeLoop::CFTreeLoop(BasicBlock *h, int bound, CFTree *n_bd, CFTree *n_ex)
		{
			header = h;
			n = bound;
			parametric = false;
			bd = n_bd;
			ex = n_ex;
		}

		CFTreeLoop::CFTreeLoop(BasicBlock *h, std::string expression, CFTree *n_bd, CFTree *n_ex)
		{
			header = h;
			n = -1;
			this->expression = expression;
			parametric = true;
			bd = n_bd;
			ex = n_ex;
		}

		bool CFTreeLoop::isParametric(){
			return parametric;
		}

		std::string CFTreeLoop::getParametricBound(){
			return expression;
		}

		BasicBlock *CFTreeLoop::getHeader() { return header; }
		int CFTreeLoop::getHeaderId() const { return header->id(); }

		CFTree *CFTreeLoop::getBody()
		{
			return bd;
		}

		CFTree *CFTreeLoop::getExit()
		{
			return ex;
		}

		void CFTreeLoop::changeBound(int bound)
		{
			n = bound;
		}

		//--------------------
		//				CLASS DAG
		//--------------------

		std::vector<DAGNode *>::const_iterator DAG::iter()
		{
			return all.begin();
		}

		std::vector<DAGNode *>::const_iterator DAG::end()
		{
			return all.end();
		}
		void DAG::addNode(DAGNode *s)
		{
			all.push_back(s);
		}

		void DAG::setStart(DAGNode *s)
		{
			n_start = s;
		}

		void DAG::setVirtNext(DAGNode *s)
		{
			n_virt_next = s;
		}
		void DAG::setVirtExit(DAGNode *s)
		{
			n_virt_exit = s;
		}

		DAGNode *DAG::getStart()
		{
			return n_start;
		}

		DAGNode *DAG::getVirtNext()
		{
			return n_virt_next;
		}
		DAGNode *DAG::getVirtExit()
		{
			return n_virt_exit;
		}

		DAGNode *DAG::getElement(size_t i)
		{
			return all[i];
		}

		void DAG::visit(std::function<void(DAGNode *)> &h, bool recursive)
		{
			for (std::vector<DAGNode *>::const_iterator it = all.begin(); it != all.end(); it++)
			{
				DAGNode *n = *it;
				h(n);
				if ((n->toHNode() != nullptr) && recursive)
				{
					n->toHNode()->getDag()->visit(h, recursive);
				}
			}
		}

		//----------------------
		//		CLASS DAGNODE
		//----------------------

		void DAGNode::addSucc(DAGNode *s)
		{
			if (std::find(succ.begin(), succ.end(), s) == succ.end())
			{
				succ.push_back(s);
			}
		}

		void DAGNode::addPred(DAGNode *s)
		{
			if (std::find(pred.begin(), pred.end(), s) == pred.end())
			{
				pred.push_back(s);
			}
		}

		std::vector<DAGNode *>::const_iterator DAGNode::succIter()
		{
			return succ.begin();
		}

		std::vector<DAGNode *>::const_iterator DAGNode::predIter()
		{
			return pred.begin();
		}

		std::vector<DAGNode *>::const_iterator DAGNode::succEnd()
		{
			return succ.end();
		}

		std::vector<DAGNode *>::const_iterator DAGNode::predEnd()
		{
			return pred.end();
		}

		std::vector<DAGNode *> DAGNode::getSucc()
		{
			return succ;
		}

		std::vector<DAGNode *> DAGNode::getPred()
		{
			return pred;
		}

		//----------------------
		//		CLASS DAGHNODE
		//----------------------

		DAGHNode *DAGHNode::toHNode()
		{
			return (DAGHNode *)this;
		}
		DAGVNode *DAGHNode::toVNode() { return NULL; }
		DAGBNode *DAGHNode::toBNode() { return NULL; }

		DAGHNode::DAGHNode(BasicBlock *b)
		{
			header = b;
			DAG_HNODE(header) = this;
		}

		void DAGHNode::setDAG(DAG *dag)
		{
			sub_dag = dag;
		}

		BasicBlock *DAGHNode::getHeader() { return header; }
		DAG *DAGHNode::getDag() { return sub_dag; }
		int DAGHNode::getLoopId() const { return header->id(); }

		//----------------------
		//		CLASS DAGVNODE
		//----------------------

		DAGVNode *DAGVNode::toVNode()
		{
			return (DAGVNode *)this;
		}
		DAGHNode *DAGVNode::toHNode() { return NULL; }
		DAGBNode *DAGVNode::toBNode() { return NULL; }

		DAGVNode::DAGVNode(int _type) { type = _type; }
		int DAGVNode::getType() const { return type; }

		//----------------------
		//		CLASS DAGBNODE
		//----------------------

		DAGBNode *DAGBNode::toBNode()
		{
			return (DAGBNode *)this;
		}
		DAGHNode *DAGBNode::toHNode() { return NULL; }
		DAGVNode *DAGBNode::toVNode() { return NULL; }

		DAGBNode::DAGBNode(Block *b)
		{
			_synth = b->isSynth();
			block = b;
			if (_synth)
				callee = b->toSynth()->callee();

			DAG_BNODE(block) = this;
		}

		Block *DAGBNode::getBlock()
		{
			return block;
		}

		bool DAGBNode::isSynth()
		{
			return _synth;
		}

		CFG *DAGBNode::getCallee()
		{
			ASSERT(isSynth());
			return callee;
		}

		int DAGBNode::getBlockId() const { return block->id(); }

		DAG::~DAG()
		{
			for (/* std::vector<DAGNode*>::iterator */ auto it = iter(); it != end(); it++)
			{
				DAGNode *n = (*it);
				delete n;
			}
		}

		//----------------------
		//		CLASS PLUGIN
		//----------------------

		class Plugin : public ProcessorPlugin
		{ // plugin qui va transformer le CFG
		public:
			Plugin() : ProcessorPlugin("otawa::cftree", Version(1, 0, 0), OTAWA_PROC_VERSION) {}
		};

		otawa::cftree::Plugin otawa_cftree_plugin;
		ELM_PLUGIN(otawa_cftree_plugin, OTAWA_PROC_HOOK);

		p::declare CFTreeExtractor::reg = p::init("otawa::cftree::CFTreeExtractor", Version(1, 0, 0))
											  .require(COLLECTED_CFG_FEATURE)
											  //				   .require(CHECKED_CFG_FEATURE)
											  .require(POSTDOMINANCE_FEATURE)
											  .require(LOOP_INFO_FEATURE)
											  .provide(EXTRACTED_CFTREE_FEATURE);

		p::feature EXTRACTED_CFTREE_FEATURE("otawa::cftree::EXTRACTED_CFT_FEATURE", new Maker<CFTreeExtractor>());

		CFTreeExtractor::CFTreeExtractor(p::declare &r) : Processor(r) {}

		void CFTreeExtractor::configure(const PropList &props)
		{
			// configure CFTreeExtractor avec la PropList
			Processor::configure(props);
		}

		//---------------------------------------
		//				PRETTYPRINTER FOR DAG
		//---------------------------------------

		io::Output &operator<<(io::Output &o, const DAGVNode &n)
		{
			if (n.getType() == VNODE_EXIT)
			{
				return o << "EXIT";
			}
			else if (n.getType() == VNODE_NEXT)
			{
				return o << "NEXT";
			}
			else
				ASSERT(false);
		}

		io::Output &operator<<(io::Output &o, const DAGBNode &n)
		{
			return (o << "B_" << n.getBlockId());
		}
		io::Output &operator<<(io::Output &o, const DAGHNode &n)
		{
			return (o << "LH_" << n.getLoopId());
		}

		io::Output &operator<<(io::Output &o, DAGNode &n)
		{
			if (n.toVNode())
				o << *n.toVNode();
			if (n.toBNode())
				o << *n.toBNode();
			if (n.toHNode())
				o << *n.toHNode();
			return o;
		}

		io::Output &operator<<(io::Output &o, DAG &n)
		{ // POUR DEBUG
			o << "\n";
			o << "start " << *n.getStart() << "\n";
			for (auto it = n.iter(); it != n.end(); it++)
			{
				DAGNode *n = (*it);
				// o << (n->getSucc()).size() << endl;
				o << *n << " -> ";
				for (auto it2 = n->succIter(); it2 != n->succEnd(); it2++)
				{
					DAGNode *n2 = (*it2);
					o << *n2 << ", ";
				}
				o << "\n";
			}
			return o;
		}

		//---------------------------------------
		//				PRETTYPRINTER FOR CFG
		//---------------------------------------

		io::Output &operator<<(io::Output &o, CFTreeLeaf &n)
		{
			return (o << "CFTreeLeaf(B_" << n.getBlockId()) << ")";
		}

		io::Output &operator<<(io::Output &o, CFTreeAlt &n)
		{
			o << "\n";
			o << "CFTreeAlt( \n";
			for (auto it = n.childIter(); it != n.childEnd(); it++)
			{
				CFTree *n = (*it);
				o << "\t" << *n << "\n";
			}
			o << ") \n";
			return o;
		}

		io::Output &operator<<(io::Output &o, CFTreeLoop &n)
		{
			o << "\n";
			o << "CFTreeLoop(B_" << (n.getHeaderId()) << " \n";
			o << "CFTreeLoop.body : " << *(n.getBody()) << "\n";
			o << "CFTreeLoop.body : " << *(n.getExit()) << "\n";
			o << ") \n";
			return o;
		}

		io::Output &operator<<(io::Output &o, CFTreeSeq &n)
		{ // POUR DEBUG
			o << "\n";
			// o << "start "<< *n.getStart() << "\n";
			o << "CFTreeSeq( \n";
			for (auto it = n.childIter(); it != n.childEnd(); it++)
			{
				CFTree *n = (*it);
				o << "\t" << *n << "\n";
			}
			o << ") \n";
			return o;
		}

		io::Output &operator<<(io::Output &o, CFTree &n)
		{
			if (n.toLeaf())
				o << *n.toLeaf();
			if (n.toAlt())
				o << *n.toAlt();
			if (n.toLoop())
				o << *n.toLoop();
			if (n.toSeq())
				o << *n.toSeq();
			return o;
		}

		//---------------------------
		//				DOT FOR CFT
		//---------------------------
		static std::string write_tree(CFTreeLoop &n, unsigned int *lab);
		static std::string write_tree(CFTreeAlt &n, unsigned int *lab);
		static std::string write_tree(CFTreeConditionalAlt &n, unsigned int *lab);
		static std::string write_tree(CFTreeSeq &n, unsigned int *lab);
		static std::string write_tree(CFTreeLeaf &n, unsigned int *lab);
		static std::string write_tree(CFTree &n, unsigned int *lab);

		static std::string write_tree(CFTreeLoop &n, unsigned int *lab)
		{
			std::stringstream ss;
			unsigned int loop_lab = *lab;
			ss << "l" << loop_lab << " [label = \"Loop(b" << n.getHeaderId() << ")\"]; \n";

			// body
			CFTree *ch = n.getBody();
			unsigned int old_lab = ++(*lab);
			ss << write_tree(*ch, lab);
			ss << "l" << loop_lab << " -> { l" << old_lab << " }; \n";

			unsigned int exit_lab = ++(*lab);
			ss << "l" << exit_lab << " [label = \" Exit\"]; \n";
			ss << "l" << loop_lab << " -> { l" << exit_lab << " }; \n";
			// exit
			ch = n.getExit();
			old_lab = ++(*lab);
			ss << write_tree(*ch, lab);
			ss << "l" << exit_lab << " -> { l" << old_lab << " }; \n";
			return ss.str();
		}

		static std::string write_tree(CFTreeAlt &n, unsigned int *lab)
		{
			std::stringstream ss;
			unsigned int alt_lab = *lab;
			ss << "l" << alt_lab << " [label = \" Alt\"]; \n";
			for (auto it = n.childIter(); it != n.childEnd(); it++)
			{
				CFTree *ch = (*it);
				unsigned int old_lab = ++(*lab);
				ss << write_tree(*ch, lab);
				ss << "l" << alt_lab << " -> { l" << old_lab << " }; \n";
			}
			return ss.str();
		}

		static std::string write_tree(CFTreeConditionalAlt &n, unsigned int *lab)
		{
			std::stringstream ss;
			unsigned int alt_lab = *lab;
			std::vector<CFTree *> children = n.getChildren();
			ss << "l" << alt_lab << " [label = \"CAlt\"]; \n";
			for (long unsigned int i = 0; i != children.size(); i++)
			{
				CFTree *ch = children[i];
				unsigned int old_lab = ++(*lab);
				ss << write_tree(*ch, lab);
				if(n.getConditions().at(i) != "")
					ss << "l" << alt_lab << " -> { l" << old_lab << " } [label=\"" << n.getConditions().at(i) << "\"]; \n";
				else
					ss << "l" << alt_lab << " -> { l" << old_lab << " }; \n";
			}
			return ss.str();
		}

		static std::string write_tree(CFTreeSeq &n, unsigned int *lab)
		{
			std::stringstream ss;
			unsigned int seq_lab = *lab;
			ss << "l" << seq_lab << " [label = \" Seq\"]; \n";
			for (auto it = n.childIter(); it != n.childEnd(); it++)
			{
				CFTree *ch = (*it);
				unsigned int old_lab = ++(*lab);
				ss << write_tree(*ch, lab);
				ss << "l" << seq_lab << " -> { l" << old_lab << " }; \n";
			}
			return ss.str();
		}

		static std::string write_tree(CFTreeLeaf &n, unsigned int *lab)
		{
			std::stringstream ss;
			ss << "l" << *lab << " [label = \"";
			if (n.getBlock() == nullptr)
			{
				ss << "NONE";
			}
			else if (n.getBlock()->isSynth())
			{
				const char *fname = n.getBlock()->toSynth()->callee()->label().toCString();
				ss << "CALL_" << fname;
			}
			else
			{ // cache implement
				// ss << "B_" << n.getBlockId();
				Block *b = n.getBlock();
				if (b->cfg() == nullptr && !IS_PIPELINE_EDGE(n.getBlock()))
				{
					// infeasible paths implement
#ifdef IP
					if (n.getOriginal())
					{
						ss << "AV_" << n.getOriginal()->getBlockId();
					}
					else
					{
#endif
						BasicCacheBlock *bcb = static_cast<BasicCacheBlock *>(b);
						ss << "LB_" << bcb->getBasicBlockId() << "-" << bcb->getLBlockId() << "_" << getCacheBlocCategory(bcb->getContextCategory()) << "-" << getCacheBlocType(bcb->getType());
#ifdef IP
					}
#endif
				}
				else
				{
					if(IS_PIPELINE_EDGE(n.getBlock()))
						ss << "B_PIPELINE";
					else{
						if(IS_AFTER_ALT(n.getBlock()))
							ss << "(AA_)";
						else if(IS_LAST_IN_ALT(n.getBlock()))
							ss << "(LA_)";
						ss << "B_" << n.getBlockId();
					}
				}
			}
			ss << "\"]; \n";
			return ss.str();
		}

		static std::string write_tree(CFTree &n, unsigned int *lab)
		{
			if (n.toLeaf())
				return write_tree(*n.toLeaf(), lab);
			if (n.toAlt())
				return write_tree(*n.toAlt(), lab);
			if (n.toLoop())
				return write_tree(*n.toLoop(), lab);
			if (n.toSeq())
				return write_tree(*n.toSeq(), lab);
			if (n.toConditionalAlt())
				return write_tree(*n.toConditionalAlt(), lab);
			return "";
		}

		static io::Output &write_code(io::Output &, CFTreeLoop &n, unsigned int indent);
		static io::Output &write_code(io::Output &, CFTreeAlt &n, unsigned int indent);
		static io::Output &write_code(io::Output &, CFTreeConditionalAlt &n, unsigned int indent);
		static io::Output &write_code(io::Output &, CFTreeSeq &n, unsigned int indent);
		static io::Output &write_code(io::Output &, CFTreeLeaf &n, unsigned int indent);
		static io::Output &write_code(io::Output &, CFTree &n, unsigned int indent);

		static io::Output &write_code(io::Output &o, CFTreeLoop &n, unsigned int indent)
		{
			unsigned int i;
			for (i = 0; i < indent; i++)
				o << " ";
			o << "for (loop" << n.getHeader()->id() << ") {\n";
			o = write_code(o, *n.getBody(), indent + 2);
			for (i = 0; i < indent; i++)
				o << " ";
			o << "}\n";

			o = write_code(o, *n.getExit(), indent);
			return o;
		}

		static io::Output &write_code(io::Output &o, CFTreeAlt &n, unsigned int indent)
		{
			bool first = true;
			bool has_none = false;
			unsigned int count = 0;
			unsigned int i;
			for (auto it = n.childIter(); it != n.childEnd(); it++)
			{
				CFTree *t = (*it);
				if (t->toLeaf() && t->toLeaf()->getBlock() == nullptr)
				{
					has_none = true;
					continue;
				}
				for (i = 0; i < indent; i++)
					o << " ";
				if (first)
				{
					o << "if (...) {\n";
				}
				else
				{
					if (has_none || count < (n.size() - 1))
					{
						o << "} else if (...) {\n";
					}
					else
					{
						o << "} else {\n";
					}
				}
				o = write_code(o, *t, indent + 2);
				first = false;
				count++;
			}
			for (i = 0; i < indent; i++)
				o << " ";
			o << "}\n";
			return o;
		}

		static io::Output &write_code(io::Output &o, CFTreeConditionalAlt &n, unsigned int indent)
		{
			bool first = true;
			bool has_none = false;
			unsigned int count = 0;
			unsigned int i;
			std::vector<CFTree *> children = n.getChildren();
			auto childIter = children.begin();
			auto childEnd = children.end();
			for (auto it = childIter; it != childEnd; it++)
			{
				CFTree *t = (*it);
				if (t->toLeaf() && t->toLeaf()->getBlock() == nullptr)
				{
					has_none = true;
					continue;
				}
				for (i = 0; i < indent; i++)
					o << " ";
				if (first)
				{
					o << "if (...) {\n";
				}
				else
				{
					if (has_none || count < (n.getChildren().size() - 1))
					{
						o << "} else if (...) {\n";
					}
					else
					{
						o << "} else {\n";
					}
				}
				o = write_code(o, *t, indent + 2);
				first = false;
				count++;
			}
			for (i = 0; i < indent; i++)
				o << " ";
			o << "}\n";
			return o;
		}

		static io::Output &write_code(io::Output &o, CFTreeSeq &n, unsigned int indent)
		{
			for (auto it = n.childIter(); it != n.childEnd(); it++)
			{
				CFTree *t = (*it);
				o = write_code(o, *t, indent);
			}
			return o;
		}

		static io::Output &write_code(io::Output &o, CFTreeLeaf &n, unsigned int indent)
		{
			for (unsigned int i = 0; i < indent; i++)
				o << " ";
			if (n.getBlock() == nullptr)
			{
				o << "NONE";
				return o;
			}
			if (n.getBlock()->isSynth())
			{
				o << n.getBlock()->toSynth()->callee()->label() << "(...);\n";
			}
			else
			{
				// cache implement
				// o << "B" << n.getBlock()->toBasic()->id() << ";\n";
				Block *b = n.getBlock();
				if (b->cfg() == nullptr && !IS_PIPELINE_EDGE(b))
				{
					BasicCacheBlock *bcb = static_cast<BasicCacheBlock *>(b);
					o << "BCB" << bcb->id() << ";\n";
				}
				else
				{
					if(IS_PIPELINE_EDGE(n.getBlock()))
						o << "BPIPE;\n";
					else
						o << "B" << n.getBlock()->toBasic()->id() << ";\n";
				}
			}
			return o;
		}

		static io::Output &write_code(io::Output &o, CFTree &n, unsigned int indent)
		{
			if (n.toLeaf())
				return write_code(o, *n.toLeaf(), indent);
			if (n.toAlt())
				return write_code(o, *n.toAlt(), indent);
			if (n.toLoop())
				return write_code(o, *n.toLoop(), indent);
			if (n.toSeq())
				return write_code(o, *n.toSeq(), indent);
			if (n.toConditionalAlt())
				return write_code(o, *n.toConditionalAlt(), indent);
			abort();
		}

		//=================================================

		//-------------------------------
		//	METHODS TO CONSTRUCT DAG
		//-------------------------------

		/*
	Returns the list of blocks contained in the loop that has b as header
*/
		void getAllBlocksLoop(DAG *dag, std::vector<DAGBNode *> *blocks, std::vector<DAGHNode *> *lh_blocks, CFG *cfg, BasicBlock *l_h)
		{
			for (CFG::BlockIter iter(cfg->blocks()); iter(); iter++)
			{
				Block *bb = *iter;						// bloc qu'on veut ajouter
				Block *bb2 = ENCLOSING_LOOP_HEADER(bb); // pointeur
				if ((bb2 == l_h) || (bb == l_h))
				{ // on ajoute les blocs qui sont directement dans la boucle l_h
					if (bb->isBasic() || bb->isSynth())
					{
						Block *b = bb;
						if (LOOP_HEADER(bb) && b != l_h)
						{ // si un autre loop_header
							// cout << "Je rentre ici" << endl;

							DAGHNode *n = new DAGHNode(b->toBasic()); // headers are always basic
							dag->addNode(n);
							lh_blocks->push_back(n);
						}
						else
						{ // sinon on va l'ajouter au DAG
							// cout << "cree dagnode pour bb: " << b << endl;
							DAGBNode *n = DAG_BNODE(b);
							if (n == NULL)
							{ // Si il n'a jamais été ajouté
								n = new DAGBNode(b);
								dag->addNode(n);
							}
							for (Block::EdgeIter it2(b->outs()); it2(); it2++)
							{ // on cherche les successeurs du bloc
								Edge *edge = *it2;
								if (LOOP_EXIT_EDGE(edge) == NULL)
								{
									if (edge->sink()->isBasic() || edge->sink()->isSynth())
									{ // si pointe vers un autre bloc basic

										Block *bbs = edge->sink();
										// cout << "lien de b1 : " << b << "vers b2 : " << bbs << endl;
										if (LOOP_HEADER(bbs) /*  && bbs != l_h */)
											continue;

										DAGNode *ns = DAG_BNODE(bbs);
										if (ns == NULL)
										{
											ns = new DAGBNode(bbs);
											dag->addNode(ns);
										}
										n->addSucc(ns);
										ns->addPred(n);
									}
								}
							}
							blocks->push_back(n);
						}
					}
				}
				//		}
			}
		}

		/*

*/
		void setEntryDAG(CFG *cfg, DAG *dag)
		{
			for (CFG::BlockIter iter(cfg->blocks()); iter(); iter++)
			{
				Block *bb = *iter;
				if (bb->isEntry())
				{
					for (Block::EdgeIter it(bb->outs()); it(); it++)
					{
						Edge *edge = *it;
						Block *start = edge->target();
						BasicBlock *b_start = *start->toBasic();
						DAGBNode *n = DAG_BNODE(b_start);
						dag->setStart(n);
					}
				}
			}
		}

		/*
	Returns the Edge(s) that iterated on the l_h block and add edge
*/
		void addEdgesNext(DAG *dag, DAGBNode *n, BasicBlock *l_h, DAGVNode *next)
		{
			Block *bb = n->getBlock();
			for (Block::EdgeIter it(bb->outs()); it(); it++)
			{
				Edge *edge = *it;
				Block *target = edge->target();
				if (target->isBasic() || target->isSynth())
				{
					if (target == l_h)
					{
						n->addSucc(next);
						next->addPred(n);
						// Edge::Edge next_edge;
						//  add (Block *v, Block *w, Edge *e)
					}
				}
				else if (target->isExit())
				{
					ASSERT(l_h == NULL);
					if (l_h == NULL)
					{
						n->addSucc(next);
						next->addPred(n);
					}
				}
			}
		}

		/*
	Returns the Edges(s) that exit of l_h loop and add edge
*/
		void addEdgesExit(DAG *dag, DAGBNode *n, BasicBlock *l_h, DAGVNode *exit)
		{
			Block *bb = n->getBlock();
			for (Block::EdgeIter it(bb->outs()); it(); it++)
			{
				Edge *edge = *it;
				if (LOOP_EXIT_EDGE(edge) != nullptr)
				{ // si il y a un edge qui sort de la loop
					n->addSucc(exit);
					exit->addPred(n);
				}
				else if (l_h == NULL)
				{
					Block *target = edge->target();
					if (target->isExit())
					{
						n->addSucc(exit);
						exit->addPred(n);
					}
				}
			}
		}

		bool contains(DAG *d, Block *b, bool recursive = false)
		{
			bool res = false;
			std::function<void(DAGNode *)> fn = [b, &res](DAGNode *n)
			{
				if (n->toBNode())
				{
					Block *bb = n->toBNode()->getBlock();
					if (bb == b)
						res = true;
				}
			};
			d->visit(fn, recursive);
			return res;
		}

		bool loop_contains(BasicBlock *h, Block *b)
		{
			if (b == h)
				return true;
			Block *current = b;
			while (ENCLOSING_LOOP_HEADER(current) != nullptr)
			{
				current = ENCLOSING_LOOP_HEADER(current);
				if (current == b)
					return true;
			}
			return false;
		}

		DAG *CFTreeExtractor::toDAG(CFG *cfg /* CFG a dag-ifier */, BasicBlock *l_h /* boucle a dag-ifier, ou NULL si racine du CFG */)
		{
			// D'abord on cree un DAG vide avec juste les noeuds virtuels EXIT et NEXT
			DAG *dag = new DAG();
			DAGVNode *next = new DAGVNode(VNODE_NEXT); // noeud NEXT en cours de construction
			DAGVNode *exit = new DAGVNode(VNODE_EXIT); // noeud EXIT
			dag->addNode(next);
			dag->addNode(exit);
			dag->setVirtNext(next);
			dag->setVirtExit(exit);

			// blocks : tous les blocs non-headers qui sont directement dans l_h
			std::vector<DAGBNode *> blocks; // Bd moins les headers de boucle

			// lh_blocks : tous les blocs headers qui sont directement dans l_h
			std::vector<DAGHNode *> lh_blocks; // lh' tous les headers de boucle

			// std::vector<DAG*> sub_cfg;
			otawa::cftree::getAllBlocksLoop(dag, &blocks, &lh_blocks, cfg, l_h);

			for (unsigned i = 0; i < lh_blocks.size(); i++)
			{ // construit les dags des sous boucles directs
				// sub_cfg.push_back(toDAG(cfg, lh_blocks[i]->getHeader()));
				lh_blocks[i]->setDAG(toDAG(cfg, lh_blocks[i]->getHeader()));
			}

			// Maintenant on a dans lh_blocks[i]->getDag() tous les sous-dags directs.

			/* Relier les basic blocks du DAG actuel aux noeuds virtuels NEXT et EXIT */
			for (unsigned i = 0; i < blocks.size(); i++)
			{
				addEdgesNext(dag, blocks[i], l_h, next);
				addEdgesExit(dag, blocks[i], l_h, exit);
			}

			/* Relier les superblocks du DAG actuel aux noeuds virtuels NEXT et EXIT */
			for (unsigned i = 0; i < lh_blocks.size(); i++)
			{
				DAG *d = lh_blocks[i]->getDag();
				std::function<void(DAGNode *)> fn = [d, exit, l_h, next](DAGNode *n)
				{
					if (n->toBNode())
					{
						Block *bb = n->toBNode()->getBlock();
						for (Block::EdgeIter it(bb->outs()); it(); it++)
						{
							if (l_h == nullptr)
							{
								if (it->target()->isExit())
								{
									n->addSucc(next);
									next->addPred(n);

									n->addSucc(exit);
									exit->addPred(n);
								}
							}
							else
							{
								if (it->target() == l_h)
								{
									n->addSucc(next);
									next->addPred(n);
								}
								Edge *e = *it;
								BasicBlock *edgehdr = LOOP_EXIT_EDGE(e) ? LOOP_EXIT_EDGE(e)->toBasic() : nullptr;
								if (loop_contains(edgehdr, l_h))
								{
									n->addSucc(exit);
									exit->addPred(n);
								}
							}
						}
					}
				};
				d->visit(fn, true);
			}

			// On a relie les NEXT/EXIT du DAG courant.

			// Maintenant il faut mettre les inedge/outedges des superblocks.
			// /!\ On peut avoir une edge entre deux superblocks

			for (unsigned i = 0; i < lh_blocks.size(); i++)
			{
				// Add edges that starts from lh_blocks[i]. Can start from any node in superblock
				std::function<void(DAGNode *)> fn = [&lh_blocks, i, &blocks, &cfg](DAGNode *n)
				{
					if (n->toBNode())
					{
						Block *bb = n->toBNode()->getBlock();
						for (Block::EdgeIter it(bb->outs()); it(); it++)
						{
							Edge *e = *it;
							Block *target = e->target();
							// add edges that go to basic blocks
							for (unsigned int j = 0; j < blocks.size(); j++)
							{
								if (blocks[j]->getBlock() == target)
								{
									lh_blocks[i]->addSucc(blocks[j]);
									blocks[j]->addPred(lh_blocks[i]);
								}
							}
							// add edges that go to super blocks. Can go only to header.
							for (unsigned int j = 0; j < lh_blocks.size(); j++)
							{
								if (i != j && contains(lh_blocks[j]->getDag(), target))
								{
									if (target != lh_blocks[j]->getHeader())
									{
										cerr << "CFG " << cfg->label()
											 << " contains an irreducible loop, edge="
											 << e->source()->id() << "->"
											 << e->target()->id() << " header=" << lh_blocks[j]->getHeader()->id() << "\n";
										ASSERT(false);
									}
									lh_blocks[i]->addSucc(lh_blocks[j]);
									lh_blocks[j]->addPred(lh_blocks[i]);
								}
							}
						}
					}
				};
				lh_blocks[i]->getDag()->visit(fn, true);

				// Add edges that go to lh_blocks[i]
				BasicBlock *bb = lh_blocks[i]->getHeader(); // edges necessarily go to the header
				for (BasicBlock::EdgeIter it(bb->ins()); it(); it++)
				{
					Edge *e = *it;
					Block *source = e->source();
					// Since the CFG contains no irreducible loops, if l_h!=nullptr, then source is contained in l_h
					// add edges that come from basic blocks
					for (unsigned int j = 0; j < blocks.size(); j++)
					{
						if (blocks[j]->getBlock() == source)
						{
							lh_blocks[i]->addPred(blocks[j]);
							blocks[j]->addSucc(lh_blocks[i]);
						}
					}
					// add edges that come from super blocks. Can come from any block inside the superblock
					for (unsigned int j = 0; j < lh_blocks.size(); j++)
					{
						if (i != j && contains(lh_blocks[j]->getDag(), source))
						{
							lh_blocks[i]->addPred(lh_blocks[j]);
							lh_blocks[j]->addSucc(lh_blocks[i]);
						}
					}
				}
			}

			if (l_h != NULL)
			{
				DAGBNode *n = DAG_BNODE(l_h);
				ASSERT(n != nullptr);
				dag->setStart(n);
			}
			else
			{
				setEntryDAG(cfg, dag);
			}
			return dag;
		}

		/*
Vérifie qu'un noeud est dominant à un autre
*/
		int DAGNodeDominate(DAGNode *b1, DAGNode *b2)
		{

			if (b1 == b2)
				return 1;

			if (b1->toVNode())
				return 0;

			Block *bb1 = b1->toBNode() ? b1->toBNode()->getBlock() : b1->toHNode()->getHeader();

			if (b2->toVNode())
			{
				for (auto pred = b2->predIter(); pred != b2->predEnd(); pred++)
				{
					if (!DAGNodeDominate(b1, *pred))
						return 0;
				}
				return 1;
			}
			else
			{

				Block *bb2 = b2->toBNode() ? b2->toBNode()->getBlock() : b2->toHNode()->getHeader();
				if (Dominance::dominates(bb1, bb2))
				{
					return 1;
				}
			}
			return 0;
		}

		/*
Retourne les blocks qui sont présents dans tous les chemins entre start et end
*/
		void forcedPassedNodes(DAG *dag, DAGNode *start, DAGNode *end, std::vector<DAGNode *> *fpn)
		{
			// std::vector<DAGNode*> *fpn;

			// pour tous les noeuds du dag
			for (auto it = dag->iter(); it != dag->end(); it++)
			{
				DAGNode *n = (*it);
				if (start != n)
				{
					if (DAGNodeDominate(start, n) && DAGNodeDominate(n, end))
					{
						fpn->push_back(n);
					}
				}
			}
		}

		void pouet()
		{
		}

		/*
Retourne le noeud qui est dominé par tous les autres
*/
		DAGNode *getDominated(std::vector<DAGNode *> fpn)
		{
			unsigned int i;

			ASSERT(fpn.size() > 0);

			DAGNode *last = fpn[0];
			for (i = 1; i < fpn.size(); i++)
				if (DAGNodeDominate(last, fpn[i]))
					last = fpn[i];

			for (i = 0; i < fpn.size(); i++)
				ASSERT(DAGNodeDominate(fpn[i], last));

			return last;
		}

		CFTree *toCFT(DAG *dag, DAGNode *start, DAGNode *end, int all)
		{
			std::vector<CFTree *> ch;
			std::vector<DAGNode *> fpn;

			forcedPassedNodes(dag, start, end, &fpn);

			if (start == end)
			{
				if (start->toBNode())
				{
					Block *bb = start->toBNode()->getBlock();
					return (new CFTreeLeaf(bb));
				}
			}

			while (fpn.size() > 0)
			{
				DAGNode *c = getDominated(fpn);												// l'element dominé
				std::vector<DAGNode *>::iterator it = std::find(fpn.begin(), fpn.end(), c); // retrouver l'index de l'élement dominé
				fpn.erase(it);																// N <- N \ c

				if (c->toBNode())
				{
					Block *bb = c->toBNode()->getBlock();
					ch.insert(ch.begin(), new CFTreeLeaf(bb));
				}
				// Si on a une loop interne, on construit le CFTree associé
				else if (c->toHNode())
				{
					DAGHNode *lh = c->toHNode();
					DAG *sub_dag = lh->getDag();
					CFTree *bd = toCFT(lh->getDag(), sub_dag->getStart(), sub_dag->getVirtNext(), 1);
					CFTree *ex = toCFT(lh->getDag(), sub_dag->getStart(), sub_dag->getVirtExit(), 1);

					ASSERT(sub_dag->getStart()->toBNode());
					if ((sub_dag->getStart())->toBNode())
					{
						// bb is a loop header, therefore it is always basic
						BasicBlock *bb = ((sub_dag->getStart())->toBNode())->getBlock()->toBasic();
						// check if the loop is parametric to change loop generation
						LoopBound lb = LOOP_BOUND(bb);
						if(lb.isParametric())
							ch.insert(ch.begin(), new CFTreeLoop(bb, lb.getExpression(), bd, ex));
						else
							ch.insert(ch.begin(), new CFTreeLoop(bb, 0, bd, ex));
					}
				}

				// Si c a plusieurs prédécesseurs, càd qu'il y a un if avant
				if (c->getPred().size() > 1)
				{
					DAGNode *ncd;
					if (fpn.size() > 0)
						ncd = getDominated(fpn);
					else
						ncd = start;

					// On ajoute à la suite le bloc dominé
					/*
			if (c->toBNode() && false){
				Block *bb =  c->toBNode()->getBlock();
				ch.insert(ch.begin(), new CFTreeLeaf(bb));
			}
			*/
					Block *cBlock = c->toBNode() != nullptr ? c->toBNode()->getBlock() : nullptr;
					// retrieve function params
					// Block *cfgEntry = cBlock->cfg()->entry();
					// std::map<int, int> function_args = FUNCTION_PARAM_MAPPING(cfgEntry);

					std::vector<CFTree *> br;

					// On construit le CFTreeAlt
					// null condition
					std::string cdtNull = "<null>";
					int predNumber = 0;
					FParamManager fpm;
					for (auto pred = c->predIter(); pred != c->predEnd(); pred++)
					{
						// pipeline annotation
						if((cBlock != nullptr && *pred != ncd && !fpm.isParam(cBlock)) || (c->toHNode() && *pred != ncd)){
							auto p = (*pred)->toBNode();
							// add an annotation on the block if the block is pred of alt exit node
							// could be nullptr, be aware !
							if(p != nullptr){
								Block* pb = p->getBlock();
								//cout << "BB " << pb->id() << " IS LAST IN BLOCK" << endl;
								if(!pb->isSynth()){
									// regular basic block
									IS_LAST_IN_ALT(pb) = true;
								}
								else{
									// function call
									// we need to get the last block of the function
									SynthBlock* sb = pb->toSynth();
									Block* exit = sb->callee()->exit(); // get the virtual exit node
									auto edges = exit->inEdges();
									std::set<Edge*> wl;
									std::set<Edge*> nextWl;
									// default list
									for(auto e = edges.begin(); e != edges.end(); e++){
										wl.insert(*e);
									}
									// process working list
									//cout << "WL SIZE: " << wl.size() << endl;
									while(wl.size() > 0){
										for(auto edge = wl.begin(); edge != wl.end(); edge++){
											Block* source = (*edge)->source();
											if(source->isBasic()){
												IS_LAST_IN_ALT(source) = true;
												//cout << source->id() << " IS LAST IN ALT" << endl;
											}
											else{
												auto predEdges = source->inEdges();
												for(auto e = predEdges.begin(); e != predEdges.end(); e++){
													nextWl.insert(*e);
												}
											}
										}
										wl = nextWl;
										//cout << "WL SIZE: " << wl.size() << endl;
									}
								}
							}
							else{
								if(DAGHNode* hn = (*pred)->toHNode()){
									// set the loop header as the last element in an ALT node
									Block* header = hn->getHeader();
									IS_LAST_IN_ALT(header) = true;
									//cout << header->id() << endl;
									// detect the correct edge to reduce pessimism
									auto es = header->outEdges();
									for(auto e = es.begin(); e != es .end(); e++){
										if((*e)->sink() == cBlock){
											EDGE_ALIAS(header) = *e;
											break;
										}
									}
								}
								else{
									cout << "WARN: Unsupported DAGNode type used" << endl;
								}
							}
						}
						if (*pred == ncd)
						{	
#ifdef PIPELINE
							if((cBlock != nullptr && !fpm.isParam(cBlock)) || (!fpm.isParam(cBlock) && c->toHNode())){
								Edge* edge;
								if(DAGHNode* hn = c->toHNode()){
									// need a special processing to account the alt before the loop
									IS_AFTER_ALT(hn->getHeader()) = true;
									IS_AA_LOOPEXIT(hn->getHeader()) = true;
									// get the edge representing the entry of the loop
									auto ies = hn->getHeader()->inEdges();
									//cout << "BB |" << cBlock->id() << "|" << endl;
									int pn = 0;
									auto it = ies.begin();
									while(pn != predNumber){
										it++;
										pn++;
									}
									edge = *it;
								}
								else{
									// find the corresponding edge
									auto ies = cBlock->inEdges();
									//cout << "BB |" << cBlock->id() << "|" << endl;
									int pn = 0;
									auto it = ies.begin();
									while(pn != predNumber){
										it++;
										pn++;
									}
									edge = *it;
								}
								// create a fake basic block
								// create fake instructions
								NullInst nullInst;
								NullInst nullInstArray[1];
								nullInstArray[0] = nullInst;
								Inst *buffer[1] = {nullInstArray};
								Inst **bufferBis = buffer;
								const Array<Inst *> insts(1, bufferBis);

								BasicBlock* fb = new BasicBlock(insts);
								ipet::TIME(fb) = 0;
								IS_LAST_IN_ALT(fb) = true;
								// create a fake predecessor
								IS_PIPELINE_EDGE(fb) = true;
								EDGE_ALIAS(fb) = edge;
								br.push_back(new CFTreeLeaf(fb));
							}
							else{
								//cout << "POSSIBLE PESSIMISM BECAUSE CBLOCK IS NOT A BASIC BLOCK" << endl;
								/*auto oes = (*pred)->toBNode()->getBlock()->outEdges();
								for(auto oe = oes.begin(); oe != oes.end(); oe++){
									auto edge = *oe;
									// create a fake basic block
									// create fake instructions
									NullInst nullInst;
									NullInst nullInstArray[1];
									nullInstArray[0] = nullInst;
									Inst *buffer[1] = {nullInstArray};
									Inst **bufferBis = buffer;
									const Array<Inst *> insts(1, bufferBis);

									BasicBlock* fb = new BasicBlock(insts);
									ipet::TIME(fb) = 0;
									IS_LAST_IN_ALT(fb) = true;
									IS_PIPELINE_EDGE(fb) = true;
									EDGE_ALIAS(fb) = edge;
									br.push_back(new CFTreeLeaf(fb));
								}*/
								br.push_back(new CFTreeLeaf(nullptr));
							}
#else
							br.push_back(new CFTreeLeaf(nullptr));
#endif

							// try to retrieve condition when a null element exists
							// Block* cBlock = c->toBNode()->getBlock();
							if (cBlock != nullptr)
							{
								BranchCondition *bc = CONDITION(cBlock);
								if (bc != nullptr)
								{
									cdtNull = bc->getConditions();
								}
							}
						}
						else
						{
							br.push_back(toCFT(dag, ncd, *pred, 0));
						}
						predNumber++;
					}
					if(cBlock != nullptr && !fpm.isParam(cBlock)){
						// annotation to make sure we know it is located at the end of an ALT node
						IS_AFTER_ALT(cBlock) = true;
						//cout << cBlock->id() << " IS AFTER ALT" << endl;
					}
					if (br.size() == 1)
					{
						ch.insert(ch.begin(), br[0]);
					}
					else
					{
						// check if in an alt node we have a condition
						ConditionParser parser;
						std::map<int, std::string> cdts;
						for (long unsigned int i = 0; i != br.size(); i++)
						{
							BranchCondition *cdt = parser.findConditionOfBranch(br[i]);
							std::string cdtString;
							if (cdt == nullptr)
							{
								// cdtNull contains the condition when the leaf is null
								//cdtString = cdtNull;
								// we can safely remove that since in practice the other branch will always be greater
							}
							else
							{
								cdtString = cdt->getConditions();
							}
							if (cdtString != "<null>")
							{
								// emplace resulting condition
								cdts.emplace(i, cdtString);
							}
							else{
								// null condition
								cdts.emplace(i, "");
							}
						}
						// generate a CFTreeAlt if there is no condition
						if (cdts.size() == 0)
							ch.insert(ch.begin(), new CFTreeAlt(br));
						// generate a CFTreeConditionalAlt otherwise
						else
							ch.insert(ch.begin(), new CFTreeConditionalAlt(br, cdts));
					}
					// save function params
					// FUNCTION_PARAM_MAPPING(cfgEntry) = function_args;
				}
			}

			ASSERT(!all || (start->getPred().size() == 0));
			if (all)
			{
				Block *bb = start->toBNode()->getBlock();
				ch.insert(ch.begin(), new CFTreeLeaf(bb));
			}

			if (ch.size() == 1)
			{
				return ch[0];
			}
			else
			{
				CFTreeSeq *t_ch = new CFTreeSeq(ch);
				return t_ch;
			}
		}

		// void printCurrentNode(CFTree* node){
		// 	if(node->toLeaf()){
		// 		cout << "LEAF" << endl;
		// 		return;
		// 	}
		// 	if(node->toSeq()){
		// 		cout << "SEQ" << endl;
		// 		return;
		// 	}
		// 	if(node->toLoop()){
		// 		cout << "LOOP" << endl;
		// 		return;
		// 	}
		// 	if(node->toAlt()){
		// 		cout << "ALT" << endl;
		// 		return;
		// 	}
		// }

		void buildPTrees(CFTree *current, std::set<PseudoPath> paths)
		{
			PseudoTreeBuilder builder;
			for (auto i = paths.begin(); i != paths.end(); i++)
			{
				CFTree *pseudoTree = builder.pseudoTree(*i, current, *(current->getConstraints()));
				// StringBuffer buf;
				// buf << id++ << "-cftree.dot";
				// pseudoTree->exportToDot(buf.toString());
				current->putPPath(*i, pseudoTree);
			}
		}

		void parallelPTrees(CFTree *current, std::set<PseudoPath> paths)
		{
			int nb_threads = paths.size() >= 8 ? 8 : paths.size();
			std::size_t const size = paths.size() / nb_threads;
			std::vector<std::thread> threads;
			auto beg = paths.begin();

			for (auto i = 0; i < nb_threads; i++)
			{
				// split the set
				std::set<PseudoPath> currentPaths;
				auto end = beg;
				if (i < nb_threads - 1)
					std::advance(end, size);
				else
					end = paths.end();
				currentPaths.insert(beg, end);

				// run the thread
				threads.push_back(std::thread(buildPTrees, current, currentPaths));

				// increment i
				if (i != nb_threads - 1)
					std::advance(beg, size);
			}

			// wait for threads
			for (auto i = 0; i < nb_threads; i++)
			{
				threads[i].join();
			}
		}

		long unsigned int getMaxConstraintSize(std::set<Constraint> cs)
		{
			long unsigned int res = 0;
			for (auto i = cs.begin(); i != cs.end(); i++)
			{
				if ((*i).size() > res)
				{
					res = (*i).size();
				}
			}
			return res;
		}

		///
		/// gets heavy constraints
		/// @param maxSize the maximum size allowed for constraints
		///
		std::set<Constraint> getHeavyConstraints(std::set<Constraint> cs, long unsigned int maxSize, int limit = -1)
		{
			std::set<Constraint> toRemove;

			// get heavy constraints
			for (auto i = cs.begin(); i != cs.end(); i++)
			{
				if ((*i).size() > maxSize)
				{
					toRemove.insert(*i);
					if (limit > 0)
						limit--;
				}
				if (limit == 0)
					return toRemove;
			}

			return toRemove;
		}

		void CFTree::exportToAWCET(formula_t *f, struct param_func *pfl, bool loopexit, bool lastBlock)
		{
#ifdef IP
			Timer timer;
			// on attache les contraintes
			if (IP && allConstraints.size() > 0)
			{
				cout << "1. Attaching constraints... ";
				timer.start();
				std::set<long> nodeList = merge(allConstraints);
				std::set<Constraint> newConstraints = finder.fillNodeParents(this, nodeList, allConstraints);
				allConstraints.insert(newConstraints.begin(), newConstraints.end());

				// cout << "Constraints: " << endl;
				// printPseudoPaths(allConstraints);

				std::set<Constraint> attached;
				for (auto i = allConstraints.begin(); i != allConstraints.end(); i++)
				{
					CFTree *nodeToAttach = finder.findLCA(*i);
					if (nodeToAttach != nullptr)
					{
						nodeToAttach->putConstraint(*i);
						attached.insert(*i);
					}
				}
				for (auto i = attached.begin(); i != attached.end(); i++)
				{
					auto place = allConstraints.find(*i);
					if (place != allConstraints.end())
					{
						allConstraints.erase(place);
					}
				}
				timer.stop();
				timer.printDuration();
				cout << endl;
			}
			// si sur le CFT courant des contraintes sont attachées
			// printCurrentNode(this);
			if (IP == 1 && constraints->size() > 0)
			{
				// constrained tree

				// heuristic to remove constraints when the system is too heavy
				std::set<long> nodeList = merge(*constraints);
				// cout << "number of nodes: " << nodeList.size() << endl;
				// cout << "number of constraints: " << constraints.size() << endl;

				if (H1 == 1 && nodeList.size() > 25)
				{
					size_t baseSize = nodeList.size();
					while (nodeList.size() > baseSize * 3 / 4)
					{
						int maxSize = getMaxConstraintSize(*constraints) - 1;
						// cout << "removing constraints with more than " << maxSize << " nodes" << endl;
						std::set<Constraint> toRemove = getHeavyConstraints(*constraints, maxSize, constraints->size() / 8 + constraints->size() % 8);
						// cout << "Removing " << toRemove.size() << " constraints" << endl;
						for (auto i = toRemove.begin(); i != toRemove.end(); i++)
						{
							auto place = constraints->find(*i);
							if (place != constraints->end())
							{
								constraints->erase(place);
							}
						}
						nodeList = merge(*constraints);
					}
				}
				// cout << "number of nodes: " << nodeList.size() << endl;
				// cout << "number of constraints (after heuristic): " << constraints.size() << endl;
				if (constraints->size() == 0)
					exportToAWCET(f, pfl, loopexit);
				// end of heuristic

				PseudoPathsBuilder pseudoPathsBuilder;
				std::set<Constraint> *cs = getConstraints();

				cout << "2. Building pseudo paths... ";
				timer.start();
				std::set<PseudoPath> paths = pseudoPathsBuilder.getPPaths(*cs, this);
				timer.stop();
				timer.printDuration();
				cout << endl;

				// cout << "Pseudo Paths set (" << infeasibleTree << "): ";
				// printPseudoPaths(paths);

				cout << "Number of pseudo paths(" << infeasibleTree << "): " << paths.size() << endl;
				if (paths.size() == 0)
					throw NoPathException();

				// once we found ppaths, we must avoïd the heuristic to be executed again
				// if we don't, we may experience memory issues during pseudo trees building
				ppathsFound = true;

				// heuristic to duplicate only constrained nodes
				cout << "3. Heuristic to avoïd node duplication... ";
				timer.start();
				std::vector<CFTree *> unConstrainedChilds;
				if (CFTreeSeq *seq = this->toSeq())
				{
					std::vector<CFTree *> currentChilds = seq->getChilds();
					for (auto i = seq->childIter(); i != seq->childEnd(); i++)
					{
						std::set<PseudoPath> pps = pseudoPathsBuilder.getPPaths(*cs, *i);
						if (pps.size() == 1)
						{
							PseudoPath pp = *(pps.begin());
							if (pp.size() == 0)
							{
								unConstrainedChilds.push_back(*i);
							}
						}
					}
					for (auto i = unConstrainedChilds.begin(); i != unConstrainedChilds.end(); i++)
					{
						currentChilds.erase(std::remove(currentChilds.begin(), currentChilds.end(), *i), currentChilds.end());
					}
					seq->setChilds(currentChilds);
					seq->setUnconstrainedChilds(unConstrainedChilds);
				}
				timer.stop();
				timer.printDuration();
				cout << endl;

				cout << "4. Building pseudo trees... ";
				timer.start();
				// PseudoTreeBuilder pseudoTreeBuilder;
				// int id = 0;
				// for(auto i = paths.begin(); i != paths.end(); i++){
				// 	CFTree* pseudoTree = pseudoTreeBuilder.pseudoTree(*i, this, constraints);
				// 	// StringBuffer buf;
				// 	// buf << id++ << "-cftree.dot";
				// 	// pseudoTree->exportToDot(buf.toString());
				// 	this->putPPath(*i, pseudoTree);
				// }
				parallelPTrees(this, paths);
				timer.stop();
				timer.printDuration();
				cout << endl;

				if (getPPaths().size() != paths.size())
				{
					throw ParallelizationException();
				}

				cout << "5. Exporting the feasible tree... ";
				timer.start();
				exportToFeasibleWCET(f, pfl);
				timer.stop();
				timer.printDuration();
				cout << endl;
			}
			else
			{
#endif
				// unconstrained tree
				if (CFTreeLeaf *n = toLeaf())
				{
					Block *b = n->getBlock();
					if (b == nullptr)
					{
						f->aw.eta_count = 0;
						f->aw.eta = nullptr;
						f->aw.others = 0;
						return;
					}
					if (b->isSynth())
					{
						CFG *callee = b->toSynth()->callee();
						/* Test if callee is parametric */
						int i = 0;
						bool parametric = false;
						while (pfl && pfl[i].funcname != nullptr)
						{
							if (!strcmp(pfl[i].funcname, callee->name().toCString()))
							{
								parametric = true;
								f->kind = KIND_AWCET;
								f->param_id = pfl[i].param_id;
								break;
							}
							i++;
						}
						if (!parametric)
						{
							CFTree *ch = CFTREE(callee);
							ch->exportToAWCET(f, pfl, loopexit, true);
						}
					}
					else if (b->isBasic())
					{ // cache implement
						// infeasible paths implement
#ifdef IP
						if (b->cfg() == nullptr && finder.copies().find(n) == finder.copies().end())
#else
						if(b->cfg() == nullptr && !IS_PIPELINE_EDGE(b))
#endif
						{ // nullptr means that it's a new block, associated with cache
							BasicCacheBlock *bcb = static_cast<BasicCacheBlock *>(b);
							f->kind = KIND_CONST;
							f->aw.loop_id = bcb->getLoopId();

							// manage annotations
							if (bcb->getType() == HIT)
							{
								// always 0 when it hits, because miss time will be bigger
								f->aw.eta_count = 0;
								f->aw.eta = nullptr;
								f->aw.others = 0;
							}
							else
							{
								switch (bcb->getContextCategory())
								{
								case INVALID_CATEGORY:
								case NOT_CLASSIFIED:
								case ALWAYS_MISS:
									f->aw.eta_count = 0;
									f->aw.eta = nullptr;
									f->aw.others = BasicCacheBlock::getPenalty(); // should be cache penalty value
									break;

								case ALWAYS_HIT:
									f->aw.eta_count = 0;
									f->aw.eta = nullptr;
									f->aw.others = 0; // will never change because cache hit is 0 too
									break;

								case FIRST_HIT: // overestimation to ALWAYS_MISS
									f->aw.eta_count = 0;
									//f->aw.eta = (long long *)malloc(sizeof(long long) * f->aw.eta_count);
									f->aw.others = BasicCacheBlock::getPenalty(); // should be cache penalty value all other times
									// for (int i = 0; i < f->aw.eta_count; i++)
									// 	f->aw.eta[i] = 0; // the first lopp iteration will be 0
									break;

								case FIRST_MISS:
									f->aw.eta_count = 1;
									f->aw.eta = (long long *)malloc(sizeof(long long) * f->aw.eta_count);
									f->aw.others = 0; // all other times
									for (int i = 0; i < f->aw.eta_count; i++)
										f->aw.eta[i] = BasicCacheBlock::getPenalty(); // first time
									break;

								default:
									cerr << "Error : missing context category for a block" << endl;
									break;
								}
							}
						}
						else
						{
							BasicBlock *bb = b->toBasic();
							f->kind = KIND_CONST;
							f->aw.loop_id = -1;
#ifdef PIPELINE
							// get the execution time of edges (execution context)
							int max_wcet = 0;
							auto edges = bb->inEdges();
							bool hasNonBasic = false; //< virtual or synth blocks can also be predecessors
							std::map<bool,int> loopExecutionTimes; //< special treatment to reduce loop pessimism (if entry ---(x)--> header > back_edge ---(y)--> header only)
							
							// only if not an ALT successor, to remove pipeline pessimism
							if(!IS_AFTER_ALT(bb) || (IS_AFTER_ALT(bb) && LOOP_HEADER(bb))){
								FParamManager fpm;
								for(auto i = edges.begin(); i != edges.end(); i++){
									Edge* edge = *i;
									if(!edge->source()->isBasic() || !edge->sink()->isBasic())
										hasNonBasic = true;
									// the next formula should look like the one in sources rergarding WCET in presence of pipeline, see otawa/src/etime/EdgeTimeBuilder.cpp
									int wcet = etime::LTS_TIME(edge);
									//cout << "LTS_TIME (BB " << bb->id() <<"): " << etime::LTS_TIME(edge) << endl;
									for(auto hts : etime::HTS_CONFIG.all(edge)){
										//cout << "HTS_TIME (BB " << bb->id() << "): " << hts.fst << endl;
										wcet += hts.fst;
									}
									max_wcet = wcet > max_wcet ? wcet : max_wcet;
									// in the case of a loop header, we can optimize by adding an execution time for all iteration but the first one
									if(LOOP_HEADER(bb)){
										bool entry = Dominance::dominates(edge->sink(), edge->source()) ? false : true; // false = back_edge, true = entry edge
										if(loopExecutionTimes.find(entry) != loopExecutionTimes.end()){
											int old_wcet = loopExecutionTimes.at(entry);
											if(old_wcet < wcet){ // only keep the greatest value
												loopExecutionTimes.erase(entry);
												loopExecutionTimes.emplace(entry, wcet);
											}
										}
										else{
											// insert new value in map
											loopExecutionTimes.emplace(entry, wcet);
										}
									}
								}
								// case of synthetic or virtual blocks, thus the value may not be on the edges we looked at
								if(max_wcet == 0 || hasNonBasic){
									for(auto i = edges.begin(); i != edges.end(); i++){
										Edge* edge = *i;
										if(!edge->source()->isBasic()){
											// BB ---(value)--> synth ---(-1)--> BB
											Block* synth = edge->source();

											if(fpm.isParam(synth))
												continue;
											
											// Case after a function return, we need to link the exit block and the following block in the calling CFG
											// BB ---(value)--> exit ---(return)--> BB
											if(synth->isCall()){
												// We must get the exit node of the callee to retrieve the value
												Block* exit = synth->toSynth()->callee()->exit();
												auto edges2 = exit->inEdges();
												for(auto j = edges2.begin(); j != edges2.end(); j++){
													Edge* edge2 = *j;
													int wcet = etime::LTS_TIME(edge2);
													//cout << "\tLTS_TIME (BB " << bb->id() <<"): " << etime::LTS_TIME(edge2) << endl;
													for(auto hts : etime::HTS_CONFIG.all(edge2)){
														//cout << "\tHTS_TIME (BB " << bb->id() << "): " << hts.fst << endl;
														wcet += hts.fst;
													}
													max_wcet = wcet > max_wcet ? wcet : max_wcet;
												}
											}
											else{
												// Case after a function entry, we need to link the first block of the function with the last before the call 
												//  BB ---(value)--> Call ---|--> VIRTUAL ---(-1)--> BB
												// Since we don't know which caller called the function, it is currently really pessimistic because we always consider the worst case to be safe
												if(synth->isVirtual()){
													//cout << "\tcfg: " << synth->cfg()->name() << " (call " << synth->cfg()->callCount() << "), BB is in " << bb->cfg() << endl;
													auto callers = synth->cfg()->callers();
													for(auto k = callers.begin(); k != callers.end(); k++){
														Block* caller = *k;
														auto edges2 = caller->inEdges();
														for(auto j = edges2.begin(); j != edges2.end(); j++){
															Edge* edge2 = *j;
															int wcet = etime::LTS_TIME(edge2);
															//cout << "\tLTS_TIME (BB " << bb->id() <<"): " << etime::LTS_TIME(edge2) << endl;
															for(auto hts : etime::HTS_CONFIG.all(edge2)){
																//cout << "\tHTS_TIME (BB " << bb->id() << "): " << hts.fst << endl;
																wcet += hts.fst;
															}
															max_wcet = wcet > max_wcet ? wcet : max_wcet;
														}
													}
												}
												else{
													cout << "Error: pipeline effect not implemented for the current analyzed block" << endl;
												}
											}
										}
										else{
											if(!edge->sink()->isBasic()){
												cout << "Error: pipeline effect failed because an edge sink is not basic" << endl;
												exit(1);
											}
										}
									}
								}
							}
							// if last bloc of an alternative
							// we need to add the successors edges to max_wcet, in order to reduce pessimism
							// we only add the pipeline time one time on the loop header, it should be ok in terms of mathematics

							// debug to see if we only add the last BB edge one time !
							// if(bb != nullptr && bb->cfg() != nullptr)
							// 	cout << bb->id() << endl;
							if(IS_LAST_IN_ALT(bb) && !LOOP_HEADER(bb) && lastBlock){
								// same debug as before
								// if(LOOP_HEADER(bb))
								// 	cout << "Loop " << bb->id() << " has a reduced pessimism" << endl;
								auto edges = bb->outEdges(); // redefine edges in this scope to be successors
								long long max_wcet_loc = 0;
								for(auto i = edges.begin(); i != edges.end(); i++){
									Edge* edge = *i;
									//if(!edge->source()->isBasic() || !edge->sink()->isBasic())
									//	hasNonBasic = true;
									// the next formula should look like the one in sources rergarding WCET in presence of pipeline, see otawa/src/etime/EdgeTimeBuilder.cpp
									int wcet = etime::LTS_TIME(edge);
									//cout << "LTS_TIME (BB " << bb->id() <<"): " << etime::LTS_TIME(edge) << endl;
									for(auto hts : etime::HTS_CONFIG.all(edge)){
										//cout << "HTS_TIME (BB " << bb->id() << "): " << hts.fst << endl;
										wcet += hts.fst;
									}
									max_wcet_loc = wcet > max_wcet_loc ? wcet : max_wcet_loc;	
								}
								max_wcet += max_wcet_loc;
								//cout << "found WCET for BB " << ": " << max_wcet_loc << "(" << max_wcet << ")" << endl;	
								if(IS_PIPELINE_EDGE(bb)){
									// manage blocks representing pipeline effect (replacing nullptr in non existing else blocks)
									auto real_edge = EDGE_ALIAS(bb);
									int wcet = etime::LTS_TIME(real_edge);
									//cout << "LTS_TIME (BB " << bb->id() <<"): " << etime::LTS_TIME(edge) << endl;
									for(auto hts : etime::HTS_CONFIG.all(real_edge)){
										//cout << "HTS_TIME (BB " << bb->id() << "): " << hts.fst << endl;
										wcet += hts.fst;
									}
									max_wcet = wcet > max_wcet ? wcet : max_wcet;
									//cout << "FOUND WCET FOR PIPELINE BLOCK: " << max_wcet << "(" << wcet << ")" << endl;
								}
							}
							f->aw.others = ipet::TIME(bb) >= 0 ? ipet::TIME(bb) + max_wcet : max_wcet; // without the cache, ipet::TIME(bb) = -1, but with, it is not, thus we need this to get the right WCET
#else
							f->aw.others = ipet::TIME(bb);
#endif							

							if (ANN_COUNT(bb) == 0)
							{
								f->aw.eta_count = ANN_COUNT(bb);
								f->aw.eta = (long long *)malloc(sizeof(long long) * f->aw.eta_count);
								for (int i = 0; i < f->aw.eta_count; i++)
									f->aw.eta[i] = ANN_TIME(bb);
							}
							else
							{
								f->aw.eta_count = 0;
								f->aw.eta = nullptr;
							}
#ifdef PIPELINE
							// set real WCET in the case of loops
							if(!hasNonBasic && LOOP_HEADER(bb)){

								// in the case entry overhead > back_edge overhead, we can optimize
								//cout << "SIZE OF MAP: " << loopExecutionTimes.size() << endl;
								if(loopExecutionTimes.size() == 2){
									int entry = loopExecutionTimes.at(true);
									int back_edge = loopExecutionTimes.at(false);
									//cout << "WCETs: " << entry << " " << back_edge << endl;
									if(loopexit){

										// add the effect of the pipeline if the loop header is the last node in an ALT
										long long max_wcet_loc = 0;
										if(IS_LAST_IN_ALT(bb)){
											Edge* edge = EDGE_ALIAS(bb);
											// the next formula should look like the one in sources rergarding WCET in presence of pipeline, see otawa/src/etime/EdgeTimeBuilder.cpp
											// if the header is not the last node of the alt (possible in some cases)
											// then it does not represent any edge
											if(edge != nullptr){
												int wcet = etime::LTS_TIME(edge);
												//cout << "LTS_TIME (BB " << bb->id() <<"): " << etime::LTS_TIME(edge) << endl;
												for(auto hts : etime::HTS_CONFIG.all(edge)){
													//cout << "HTS_TIME (BB " << bb->id() << "): " << hts.fst << endl;
													wcet += hts.fst;
												}
												max_wcet_loc = wcet > max_wcet_loc ? wcet : max_wcet_loc;
											}
										}
										// The loop exit contains the pipeline effect of the entry and the exit of the loop, to be sure to stay safe
										// this ensures that at least one time we execute the test with the entry and the exit pipeline effects
										// cout << "In loop exit" << endl;
										// The if is used to remove the entry edge time from the header time in the exit node when loop header is the successor of an alt
										if(IS_AA_LOOPEXIT(bb))
											entry = 0;
										entry += max_wcet_loc;
										f->aw.others = ipet::TIME(bb) >= 0 ? ipet::TIME(bb) + entry : entry;
									}
									else{
										// The loop backedges WCET
										// cout << "Not in loop exit" << endl;
										f->aw.others = ipet::TIME(bb) >= 0 ? ipet::TIME(bb) + back_edge : back_edge;
									}
								}
							}
							// else if (hasNonBasic && LOOP_HEADER(bb)){
							// 	cout << "BB " << bb->id() << " has a non basic WCET" << endl;
							// }
#endif
						}
					}
					else
					{
						cerr << "warning: bb " << b << " is not basic and not synth" << endl;
					}
				}
				if (CFTreeAlt *n = toAlt())
				{
					f->kind = KIND_ALT;
					int nchild = 0;
					for (auto it = n->childIter(); it != n->childEnd(); it++)
					{
						nchild++;
					}
					f->opdata.children_count = nchild;
					f->children = (formula_t *)calloc(sizeof(formula_t), nchild);
					int i = 0;
					for (auto it = n->childIter(); it != n->childEnd(); it++)
					{
						CFTree *ch = (*it);
						if (ch->toLeaf() && ch->toLeaf()->getBlock() == nullptr)
							continue;
						ch->exportToAWCET(f->children + i, pfl, loopexit, true);
						i++;
					}
				}
				if (CFTreeConditionalAlt *n = toConditionalAlt())
				{
					f->kind = KIND_ALT;
					std::vector<CFTree *> children = n->getChildren();
					int nchild = children.size();
					f->opdata.children_count = nchild;
					f->children = (formula_t *)calloc(sizeof(formula_t), nchild);
					for (long unsigned int i = 0; i != children.size(); i++)
					{
						CFTree *ch = children[i];
						std::string condition = n->getConditions()[i];
						if (ch->toLeaf() && ch->toLeaf()->getBlock() == nullptr)
								continue;
						if(condition != ""){
							/* direct child formula is a boolean product */
							formula_t *fchild = f->children + i;						  // each child formula is another formula
							fchild->kind = KIND_BOOLMULT;								  // define as a boolean product
							fchild->opdata.children_count = 2;							  //< boolean expression + CFT formula = 2
							fchild->children = (formula_t *)calloc(sizeof(formula_t), 2); //< boolean expression + CFT formula = 2

							/* first child of the boolean product is always the boolean expression */
							formula_t *boolean_expr = fchild->children; //< the first element is the boolean expression
							boolean_expr->kind = KIND_STR;
							strcpy(boolean_expr->bool_expr, condition.c_str()); // TODO : process the string to change the id to a unique id

							// second child of the boolean product is the WCET formula of the tree
							ch->exportToAWCET(fchild->children + 1, pfl, loopexit, true); //< +1 = the second child
						}
						else{
							ch->exportToAWCET(f->children + i, pfl, loopexit, true); //< export as a regular tree without condition
						}
					}
				}
				if (CFTreeLoop *n = toLoop())
				{
					CFTree *b = n->getBody();
					CFTree *e = n->getExit();
					f->kind = KIND_SEQ;
					f->children = (formula_t *)calloc(sizeof(formula_t), 2);
					f->children[0].kind = KIND_LOOP;
					f->children[0].opdata.loop_id = n->getHeader()->id();
					f->children[0].children = (formula_t *)calloc(sizeof(formula_t), 1);
					b->exportToAWCET(f->children[0].children, pfl, loopexit);
					e->exportToAWCET(f->children + 1, pfl, true);
					f->opdata.children_count = 2;

					// default value for a non parametric loop
					strcpy(f->children[0].bool_expr, "-1");

					if (MAX_ITERATION(n->getHeader()) & PARAM_FLAG & !(n->isParametric()))
					{
						f->children[0].param_id = MAX_ITERATION(n->getHeader()) & ~PARAM_FLAG;
					}
					else{
						if(n->isParametric()){
							// set a real condition to the loop in the formula
							std::string bound = n->getParametricBound();
							strcpy(f->children[0].bool_expr, bound.c_str());
						}
					}
				}
				if (CFTreeSeq *n = toSeq())
				{
					f->kind = KIND_SEQ;
					int nchild = 0;
					for (auto it = n->childIter(); it != n->childEnd(); it++)
					{
						nchild++;
					}
					f->opdata.children_count = nchild;
					f->children = (formula_t *)calloc(sizeof(formula_t), nchild);
					int i = 0;
					for (auto it = n->childIter(); it != n->childEnd(); it++)
					{
						CFTree *ch = (*it);
						if(i == nchild-1) // explicit last child information
							ch->exportToAWCET(f->children + i, pfl, loopexit, true);
						else
							ch->exportToAWCET(f->children + i, pfl, loopexit);
						i++;
					}
				}
#ifdef IP
			}
#endif
		}

		void CFTree::exportToDot(const elm::string &str)
		{
			std::ofstream myfile;
			myfile.open(str.toCString());
			unsigned int lab = 0;
			std::stringstream ss;
			ss << "digraph BST { \n"
			   << write_tree(*this, &lab) << "}";
			myfile << ss.str();
			myfile.close();
		}
		void CFTree::exportToC(io::Output &out)
		{
			unsigned int indent = 2;
			write_code(out, *this, indent);
		}

		CFTree *CFTreeExtractor::processCFG(CFG *cfg)
		{
			bool reachable = false;
			for (CFG::BlockIter iter(cfg->blocks()); !reachable && iter(); iter++)
			{
				if ((*iter)->isExit())
				{
					for (Block::EdgeIter it((*iter)->ins()); !reachable && it(); it++)
						reachable = true;
				}
			}
			if (!reachable)
			{
				cout << "Not processing CFG: " << cfg->label() << " because EXIT is not reachable\n";
				return nullptr;
			}
			/*	cout << "Processing CFG: " << cfg->label() << "\n"; */
			DAG *dag = toDAG(cfg, NULL);
			CFTree *tree = toCFT(dag, dag->getStart(), dag->getVirtExit(), 1);

#ifdef ICACHE
			// cache implement
			//cout << "Instruction cache activated" << endl;
			CFTCacheMutator cacheMutator;
			tree = cacheMutator.transform(tree);
#endif

			CFTREE(cfg) = tree;

			return tree;
		}

		CFTCollection::CFTCollection(std::vector<CFTree *> v)
		{
			cfts = v;
		}

		void CFTreeExtractor::processWorkSpace(WorkSpace *ws)
		{
			const CFGCollection *coll = INVOLVED_CFGS(ws);
			for (CFGCollection::Iter iter(*coll); iter(); iter++)
			{
				CFG *currentCFG = *iter;
				processCFG(currentCFG);
			}
		}

		Identifier<DAGHNode *> DAG_HNODE("otawa::cftree:DAG_HNODE");
		Identifier<DAGBNode *> DAG_BNODE("otawa::cftree:DAG_BNODE");
		Identifier<CFTree *> CFTREE("otawa::cftree:CFTREE");
		Identifier<int> ANN_TIME("otawa::cftree:ANN_TIME", 0);
		Identifier<int> ANN_COUNT("otawa::cftree:ANN_COUNT", 0);

		/// *******************
		/// CACHE MUTATOR CLASS
		/// *******************
		///
		/// Constructor
		///
		CFTCacheMutator::CFTCacheMutator(){};

		///
		/// Transform a tree into another tree to include cache behavior
		///
		CFTree *CFTCacheMutator::transform(CFTree *tree)
		{
			// si leaf, on appelle transofmLeaf (condition d'arrêt)
			// LEAF
			if (CFTreeLeaf *leaf = tree->toLeaf())
			{
				return transformLeaf(leaf);
			}
			// sinon appel récurcif sur les sous-arbres
			// ALT
			if (CFTreeAlt *alt = tree->toAlt())
			{
				std::vector<CFTree *> alts;
				for (unsigned i = 0; i < alt->size(); i++)
				{
					CFTree *subTree = alt->getI(i);
					alts.push_back(transform(subTree));
				}
				CFTreeAlt *newAlt = new CFTreeAlt(alts);
				return newAlt;
			}
			// CONDITIONAL ALT
			if (CFTreeConditionalAlt *alt = tree->toConditionalAlt())
			{
				std::vector<CFTree *> alts;
				for (unsigned i = 0; i < alt->getChildren().size(); i++)
				{
					CFTree *subTree = alt->getChildren()[i];
					alts.push_back(transform(subTree));
				}
				std::map<int,std::string> cdts = std::map<int,std::string>(alt->getConditions());
				CFTreeConditionalAlt *newAlt = new CFTreeConditionalAlt(alts,cdts);
				return newAlt;
			}
			// LOOPS
			if (CFTreeLoop *loop = tree->toLoop())
			{
				BasicBlock *bb = loop->getHeader();
				CFTree *body = loop->getBody();
				CFTree *exit = loop->getExit();

				body = transform(body);
				exit = transform(exit);

				CFTreeLoop *newLoop;
				if(!loop->isParametric()){
					int bound = loop->getBound();
					newLoop = new CFTreeLoop(bb, bound, body, exit);
				}
				else{
					std::string bound = loop->getParametricBound();
					newLoop = new CFTreeLoop(bb, bound, body, exit);
				}
				return newLoop;
			}
			// SEQ
			if (CFTreeSeq *sequence = tree->toSeq())
			{
				std::vector<CFTree *> childs;

				for (unsigned i = 0; i < sequence->size(); i++)
				{
					childs.push_back(transform(sequence->getI(i)));
				}

				CFTreeSeq *newSequence = new CFTreeSeq(childs);
				return newSequence;
			}
			cerr << "Error : A problem occured while finding the type of the tree" << endl;
			abort();
		}

		///
		/// This function allows to transform a leaf into a sequence containing cache penalties
		/// @param leaf the leaf to be splitted
		/// @return a sequence of node to replace the given leaf
		///
		CFTreeSeq *CFTCacheMutator::transformLeaf(CFTreeLeaf *leaf)
		{
			int i = 0;
			Block *block = leaf->getBlock();
			std::vector<CFTree *> childs;

			if (block != nullptr && block->cfg() != nullptr) // second condition = pipeline edge ignore
			{
				// cout << "\nBasicBlock " << id << endl;
				AllocArray<LBlock *> *array = BB_LBLOCKS(block);
				// cout << "Nombre de L-Blocks : " << array->length() << endl;

				// manage l blocks
				if (array != nullptr)
				{
					for (i = 0; i < array->length(); i++)
					{
						CFTree *tree = transformLBlock(array->get(i), block);
						if (tree != nullptr)
						{
							childs.push_back(tree);
						}
					}
				}
			}

			// pushing the leaf after the cache block is important for the pipeline implementation (last block of sequence, etc...)
			childs.push_back(leaf);

			// create the sequence
			CFTreeSeq *sequence = new CFTreeSeq(childs);

			return sequence;
		}

		///
		/// transform a LBlock according to its category
		/// @param lBlock the L-Block to transform
		///
		CFTree *CFTCacheMutator::transformLBlock(LBlock *lBlock, Block* block)
		{
			int basicBlockId = block->id();
			category_t cat = CATEGORY(lBlock);
			// cout << "\n\tLBlock " << lBlock->id() << " : " << cat << endl;

			// get the loop header
			Block *header = CATEGORY_HEADER(lBlock);
			if(header != nullptr && (cat == FIRST_MISS || cat == FIRST_HIT) && !LOOP_HEADER(header)){
				// header is not a loop id, thus we try to get another loop header
				//cout << "BB " << header->id() << " is not a loop header";
				header = ENCLOSING_LOOP_HEADER(block);
				if(header == nullptr){
					//cout << ", cannot find an enclosing loop header, use ALWAYS MISS category instead" << endl;
					cat = ALWAYS_MISS;
				}
				//else
				//	cout << ", using BB " << header->id() << " instead" << endl;
			}
			int loop_id = header != nullptr ? header->id() : -1;
			// cout << "Loop id : " << loop_id << endl;

			// create a fake array of instructions
			NullInst nullInst;
			NullInst nullInstArray[1];
			nullInstArray[0] = nullInst;
			Inst *buffer[1] = {nullInstArray};
			Inst **bufferBis = buffer;

			Array<Inst *> instsHit(1, bufferBis);
			Array<Inst *> instsMiss(1, bufferBis);

			switch (cat)
			{
			case ALWAYS_HIT:
			{
				return nullptr;
				break;
			}

			case ALWAYS_MISS:
			{
				BasicCacheBlock *block = new BasicCacheBlock(instsMiss, lBlock->id(), basicBlockId, MISS, cat, loop_id);
				CFTreeLeaf *leaf = new CFTreeLeaf(block);
				return leaf;
				break;
			}

			case FIRST_HIT:
			{
				BasicCacheBlock *blockHit = new BasicCacheBlock(instsHit, lBlock->id(), basicBlockId, HIT, cat, loop_id);
				CFTreeLeaf *leafHit = new CFTreeLeaf(blockHit);

				BasicCacheBlock *blockMiss = new BasicCacheBlock(instsMiss, lBlock->id(), basicBlockId, MISS, cat, loop_id);
				CFTreeLeaf *leafMiss = new CFTreeLeaf(blockMiss);

				std::vector<CFTree *> *alts = new std::vector<CFTree *>();
				alts->push_back(leafHit);
				alts->push_back(leafMiss);
				CFTreeAlt *alt = new CFTreeAlt(*alts);
				return alt;
				break;
			}

			case FIRST_MISS:
			{
				BasicCacheBlock *blockMiss = new BasicCacheBlock(instsMiss, lBlock->id(), basicBlockId, MISS, cat, loop_id);
				CFTreeLeaf *leafMiss = new CFTreeLeaf(blockMiss);
				return leafMiss;
				break;
			}

			case INVALID_CATEGORY:
			case NOT_CLASSIFIED:
			{
				BasicCacheBlock *blockMiss = new BasicCacheBlock(instsMiss, lBlock->id(), basicBlockId, MISS, cat, loop_id);
				CFTreeLeaf *leafMiss = new CFTreeLeaf(blockMiss);
				return leafMiss;
				break;
			}
			default:
				cerr << "ERROR : Case " << cat << " not existing i cache management" << endl;
				abort();
				break;
			}
		}

		int BasicCacheBlock::blockCount = 0;
		int BasicCacheBlock::penalty = 0;

		/// ************************
		/// LCA
		/// ************************

		CFTree *CFTree::getParent()
		{
			return parent;
		}

		void CFTree::setParent(CFTree *parent)
		{
			this->parent = parent;
		}

		std::set<PseudoPath> CFTree::getPseudoPaths()
		{
			return ppaths;
		}
		void CFTree::setPseudoPaths(std::set<PseudoPath> pseudoPaths)
		{
			this->ppaths = pseudoPaths;
		}

		///
		/// Dumps a leaf into a new leaf with dumped block and unique ID
		///
		CFTreeLeaf *LCAFinder::copy(CFTreeLeaf *toReplace, CFTree *parent)
		{

			// dump the leaf
			Block *b = toReplace->getBlock();
			Block *newBlock;
			if (b->isBasic())
			{
				BasicBlock *bb = b->toBasic();
				// copy instructions
				Inst **tab = (Inst **)malloc(sizeof(Inst *) * bb->count());
				int id = 0;
				for (auto i = bb->begin(); i != bb->end(); i++)
				{
					tab[id] = *i;
				}
				Array<Inst *> *arr = new Array<Inst *>(bb->count(), tab);
				newBlock = new BasicBlock(*arr);
			}
			else
			{
				SynthBlock *sb = b->toSynth();
				newBlock = new SynthBlock(sb->isCall());
			}

			ipet::TIME(newBlock) = ipet::TIME(b);
			ANN_COUNT(newBlock) = ANN_COUNT(b);
			ANN_TIME(newBlock) = ANN_TIME(b);

			// fix double free error
			ENCLOSING_LOOP_HEADER(newBlock) = ENCLOSING_LOOP_HEADER(b);

			CFTreeLeaf *leaf = new CFTreeLeaf(newBlock);
			leaf->setOriginal(toReplace);
			/*int newId = */ putNewLeaf(leaf);

			// cout << "AV_" << newId << " TIME: " << ipet::TIME(newBlock) << ", ANN_COUNT: " << ANN_COUNT(newBlock) << ", ANN_TIME: " << ANN_TIME(newBlock) << endl;

			// replace into the parent
			parent->replace(toReplace, leaf);

			// return the new leaf
			return leaf;
		}

		///
		/// Find father/child relations between nodes of interest
		///
		std::set<Constraint> LCAFinder::fillNodeParents(CFTree *tree, std::set<long> nodes, std::set<Constraint> cs)
		{

			std::set<Constraint> newConstraints;

			if (CFTreeLeaf *leaf = tree->toLeaf())
			{

				if (leaf->getBlock() != nullptr && leaf->getBlock()->isSynth())
				{
					CFTree *tree = CFTREE(leaf->getBlock()->toSynth()->callee());
					newConstraints = fillNodeParents(tree, nodes, cs);
				}

				if (leaf->getBlock() != nullptr && leaf->getBlock()->cfg() != nullptr && nodes.find(leaf->getBlockId()) != nodes.end())
				{
					if (seen.find(leaf->getBlockId()) != seen.end())
					{
						CFTreeLeaf *oldLeaf = leaf;
						leaf = copy(leaf, currentParent);
						int leafId = getLeafId(leaf);
						oldLeaf->addAvatar(leaf);
						// cout << "Dumped BB " << oldLeaf->getBlockId() << ", new BB : " << leafId << endl;
						leaves.emplace(leafId, leaf);

						// create new constraints with the duplicated nodes
						ConstraintCloner cloner;
						std::set<Constraint> toDuplicate = cloner.getConstraintsContaining(oldLeaf->getBlockId(), cs);
						newConstraints = cloner.replace(oldLeaf->getBlockId(), leafId, cs);
					}
					else
					{
						leaves.emplace(leaf->getBlockId(), leaf);
						seen.insert(leaf->getBlockId());
					}
					leaf->setParent(currentParent);
				}
			}

			if (CFTreeSeq *seq = tree->toSeq())
			{
				seq->setParent(currentParent);
				currentParent = seq;
				for (auto i = seq->childIter(); i != seq->childEnd(); i++)
				{
					std::set<Constraint> s = fillNodeParents(*i, nodes, cs);
					newConstraints.insert(s.begin(), s.end());
				}
				currentParent = seq->getParent();
			}

			if (CFTreeAlt *alt = tree->toAlt())
			{
				alt->setParent(currentParent);
				currentParent = alt;
				for (auto i = alt->childIter(); i != alt->childEnd(); i++)
				{
					std::set<Constraint> s = fillNodeParents(*i, nodes, cs);
					newConstraints.insert(s.begin(), s.end());
				}
				currentParent = alt->getParent();
			}

			if (CFTreeLoop *loop = tree->toLoop())
			{
				loop->setParent(currentParent);
				currentParent = loop;
				std::set<Constraint> s = fillNodeParents(loop->getBody(), nodes, cs);
				newConstraints.insert(s.begin(), s.end());
				currentParent = loop->getParent();
			}

			return newConstraints;
		}

		///
		/// find the LCA between 2 nodes
		///
		CFTree *LCAFinder::findLCA(CFTree *tree1, CFTree *tree2)
		{
			std::list<CFTree *> parent1;

			if (tree1 == tree2)
			{
				return tree1;
			}

			CFTree *node = tree1;
			parent1.push_back(node);

			// comparison list
			while (node->getParent() != nullptr)
			{
				node = node->getParent();
				parent1.push_back(node);
			}

			// compare parents of n1 and n2
			node = tree2;
			while (node->getParent() != nullptr)
			{
				node = node->getParent();
				for (auto i = parent1.begin(); i != parent1.end(); i++)
				{
					if (node == *i)
					{
						return node;
					}
				}
			}

			// cout << "Error, cannot find a common parent between two trees" << endl;
			return nullptr;
		}

		///
		/// Get the LCA of a constraint
		///
		CFTree *LCAFinder::findLCA(Constraint c)
		{
			std::list<CFTree *> baseNodes;
			for (auto i = c.begin(); i != c.end(); i++)
			{
				CFTree *cft = leaves.find(*i)->second;
				if (cft != nullptr)
					baseNodes.push_back(cft);
			}

			// unique node -> attach to parent sequence
			if (baseNodes.size() == 1)
			{
				CFTree *tree = *(baseNodes.begin());
				CFTreeSeq *seq = tree->toSeq();
				while (!seq)
				{
					tree = tree->getParent();
					seq = tree->toSeq();
				}
				return tree;
			}

			// other constraint types
			CFTree *lca = *(baseNodes.begin());
			auto i = baseNodes.begin();
			i++;
			for (; i != baseNodes.end(); i++)
			{
				lca = findLCA(lca, *i);
				if (lca == nullptr)
					return lca;
			}
			return lca;
		}

		/// ************************
		/// PSEUDO PATHS
		/// ************************
		void printPseudoPath(PseudoPath ppath)
		{
			cout << "{";
			for (auto j = ppath.begin(); j != ppath.end(); j++)
			{
				if (j != ppath.begin())
				{
					cout << "," << *j;
				}
				else
				{
					cout << *j;
				}
			}
			cout << "}";
		}
		void printPseudoPaths(std::set<PseudoPath> ppaths)
		{
			for (auto i = ppaths.begin(); i != ppaths.end(); i++)
			{
				if (i != ppaths.begin())
				{
					cout << ", ";
				}
				printPseudoPath(*i);
			}
			cout << "\n";
		}

		///
		/// merge several pseudo paths or several constraints
		///
		std::set<long> merge(std::set<std::set<long>> nodeSets)
		{
			std::set<long> collector;
			for (auto i = nodeSets.begin(); i != nodeSets.end(); i++)
			{
				std::set<long> nodeSet = *i;
				for (auto j = nodeSet.begin(); j != nodeSet.end(); j++)
				{
					collector.insert(*j);
				}
			}
			return collector;
		}

		///
		/// get the list of pseudo paths of a node
		///
		std::set<PseudoPath> PseudoPathsBuilder::getPPaths(std::set<Constraint> cs, CFTree *node)
		{

			if (node->getPseudoPaths().size() > 0)
			{
				return node->getPseudoPaths();
			}

			// adding node constraints if any
			std::set<Constraint> *nodeCs = node->getConstraints();

			// heuristic
			std::set<long> nodeList = merge(*nodeCs);
			if (!ppathsFound && H2 == 1 && nodeList.size() > 25)
			{
				size_t baseSize = nodeList.size();
				while (nodeList.size() > baseSize * 3 / 4)
				{
					int maxSize = getMaxConstraintSize(*nodeCs) - 1;
					// cout << "removing constraints with more than " << maxSize << " nodes" << endl;
					std::set<Constraint> toRemove = getHeavyConstraints(*nodeCs, maxSize, nodeCs->size() / 8 + nodeCs->size() % 8);
					// cout << "Removing " << toRemove.size() << " constraints" << endl;
					for (auto i = toRemove.begin(); i != toRemove.end(); i++)
					{
						auto place = nodeCs->find(*i);
						if (place != nodeCs->end())
						{
							nodeCs->erase(nodeCs->find(*i));
						}
					}
					nodeList = merge(*nodeCs);
				}
				// node->setConstraints(nodeCs);
			}
			// end heuristic
			/*for(auto i = nodeCs.begin(); i != nodeCs.end(); i++){
				cs.insert(*i);
			}*/
			cs.insert(nodeCs->begin(), nodeCs->end());

			std::set<PseudoPath> ppaths;

			if (CFTreeLeaf *leaf = node->toLeaf())
			{
				PseudoPath ppath;

				// this leaf does not have a basic block
				if (leaf->getBlock() == nullptr)
				{
					ppaths.insert(ppath);
					node->setPseudoPaths(ppaths);
					return ppaths;
				}

				// manage inlining
				if (leaf->getBlock()->isSynth())
				{
					SynthBlock *sb = leaf->getBlock()->toSynth();
					CFTree *tree = CFTREE(sb->callee());
					ppaths = getPPaths(cs, tree);
					node->setPseudoPaths(ppaths);
					return ppaths;
				}

				std::set<long> csNodes = merge(cs);
				for (auto i = csNodes.begin(); i != csNodes.end(); i++)
				{
					// infeasible paths special IDs
					int leafSpecialID = finder.getLeafId(leaf);
					// the first condition ignores cache blocks
					if (leaf->getBlock()->cfg() != nullptr && leaf->getBlockId() == *i)
					{
						ppath.insert(leaf->getBlockId());
						break;
					}
					else
					{
						if (leafSpecialID != 0)
						{
							ppath.insert(leafSpecialID);
						}
					}
				}
				ppaths.insert(ppath);
				// printf("Pseudo paths for a Leaf\n");
				// printPseudoPaths(ppaths);
			}
			else if (CFTreeAlt *alt = node->toAlt())
			{
				ppaths = getAltPaths(cs, alt->childIter(), alt->childEnd());
				// printf("Pseudo paths for an Alternative\n");
				// printPseudoPaths(ppaths);
			}
			else if (CFTreeSeq *seq = node->toSeq())
			{
				ppaths = getSeqPaths(cs, seq->childIter(), seq->childEnd());
				// printf("Pseudo paths for a Sequence\n");
				// printPseudoPaths(ppaths);
			}
			else
			{
				CFTreeLoop *loop = node->toLoop();
				ppaths = getPPaths(cs, loop->getBody());
				std::set<PseudoPath> exitPaths = getPPaths(cs, loop->getExit());
				ppaths.insert(exitPaths.begin(), exitPaths.end());
				// printf("Pseudo paths for a Loop\n");
				// printPseudoPaths(ppaths);
			}

			// removing node constraints if any
			for (auto i = nodeCs->begin(); i != nodeCs->end(); i++)
			{
				cs.erase(*i);
			}
			// cs.erase(nodeCs.begin(), nodeCs.end());

			node->setPseudoPaths(ppaths);
			return ppaths;
		}

		///
		/// get ppaths of an alt node
		///
		std::set<PseudoPath> PseudoPathsBuilder::getAltPaths(std::set<Constraint> cs, std::vector<otawa::cftree::CFTree *>::const_iterator children, std::vector<CFTree *>::const_iterator end)
		{
			std::set<PseudoPath> ppaths;
			for (auto i = children; i != end; i++)
			{
				CFTree *child = *i;
				std::set<PseudoPath> childPPaths = getPPaths(cs, child);
				// if(CFTreeLoop* loop = (*i)->toLoop()){
				// 	std::set<PseudoPath> pseudoPaths = getPPaths(cs, loop->getExit());
				// 	// for(auto j = pseudoPaths.begin(); j != pseudoPaths.end(); j++){
				// 	// 	childPPaths.emplace(*j);
				// 	// }
				// 	childPPaths.insert(pseudoPaths.begin(), pseudoPaths.end());
				// }
				/*for(auto j = childPPaths.begin(); j != childPPaths.end(); j++){
					ppaths.insert(*j);
				}*/
				ppaths.insert(childPPaths.begin(), childPPaths.end());
			}
			return ppaths;
		}

		///
		/// get ppaths of a seq node
		///
		std::set<PseudoPath> PseudoPathsBuilder::getSeqPaths(std::set<Constraint> cs, std::vector<otawa::cftree::CFTree *>::const_iterator children, std::vector<CFTree *>::const_iterator end)
		{
			std::set<PseudoPath> ppaths;
			for (auto i = children; i != end; i++)
			{
				std::set<PseudoPath> childPPaths = getPPaths(cs, *i);
				// if(CFTreeLoop* loop = (*i)->toLoop()){
				// 	std::set<PseudoPath> pseudoPaths = getPPaths(cs, loop->getExit());
				// 	// for(auto j = pseudoPaths.begin(); j != pseudoPaths.end(); j++){
				// 	// 	childPPaths.emplace(*j);
				// 	// }
				// 	childPPaths.insert(pseudoPaths.begin(), pseudoPaths.end());
				// }
				std::set<PseudoPath> updatedPPaths;
				for (auto j = childPPaths.begin(); j != childPPaths.end(); j++)
				{
					if (i == children)
					{
						// printf("inserting pseudo path : ");
						// printPseudoPath(*j);
						updatedPPaths.insert(*j);
					}
					else
					{
						for (auto k = ppaths.begin(); k != ppaths.end(); k++)
						{
							std::set<PseudoPath> toMerge;
							toMerge.insert(*k);
							toMerge.insert(*j);
							// printf("merging :\n");
							// printPseudoPath(*k);
							// printPseudoPath(*j);
							PseudoPath combined = merge(toMerge);
							// printf("inserting pseudo path : ");
							// printPseudoPath(combined);
							updatedPPaths.insert(combined);
						}
					}
					// printf("size of paths set : %ld\n", updatedPPaths.size());
				}
				ppaths = removeIPaths(cs, updatedPPaths);
			}
			return ppaths;
		}

		///
		/// Removes infeasible paths from a pseudo path list
		///
		std::set<PseudoPath> PseudoPathsBuilder::removeIPaths(std::set<Constraint> cs, std::set<PseudoPath> ppaths)
		{
			std::set<PseudoPath> feasiblePaths;
			for (auto i = ppaths.begin(); i != ppaths.end(); i++)
			{
				bool remove = false;
				for (auto j = cs.begin(); j != cs.end(); j++)
				{
					bool jump = false;
					for (auto k = (*j).begin(); k != (*j).end(); k++)
					{
						if ((*i).find(*k) == (*i).end())
						{
							jump = true;
							break;
						}
					}
					if (jump)
					{
						continue;
					}
					else
					{
						remove = true;
						break;
					}
				}
				if (!remove)
				{
					feasiblePaths.insert(*i);
				}
			}
			return feasiblePaths;
		}

		/// ************************
		/// PSEUDO TREES
		/// ************************

		///
		/// Get the pseudo tree of a node
		///
		CFTree *PseudoTreeBuilder::pseudoTree(PseudoPath ppath, CFTree *node, std::set<Constraint> cs)
		{

			// adding node constraints if any
			std::set<Constraint> *nodeCs = node->getConstraints();
			/*for(auto i = nodeCs.begin(); i != nodeCs.end(); i++){
				cs.insert(*i);
			}*/
			cs.insert(nodeCs->begin(), nodeCs->end());

			CFTree *tree;

			if (CFTreeLeaf *leaf = node->toLeaf())
			{
				/*CFTreeLeaf* newLeaf = new CFTreeLeaf(leaf->getBlock());
				newLeaf->setOriginal(leaf->getOriginal());
				finder.replaceLeafId(leaf, newLeaf);
				tree = newLeaf;*/
				// inlining
				if (leaf->getBlock() != nullptr && leaf->getBlock()->isSynth())
				{
					tree = pseudoTree(ppath, CFTREE(leaf->getBlock()->toSynth()->callee()), cs);
				}
				else
				{
					tree = leaf;
				}
			}
			else if (CFTreeSeq *seq = node->toSeq())
			{
				std::vector<CFTree *> childs = pseudoSeq(ppath, seq->childIter(), seq->childEnd(), cs);
				CFTreeSeq *newSeq = new CFTreeSeq(childs);
				tree = newSeq;
			}
			else if (CFTreeLoop *loop = node->toLoop())
			{
				if (remove(ppath, loop, cs) == false)
				{
					CFTree *body = pseudoTree(ppath, loop->getBody(), cs);
					CFTree *exit = pseudoTree(ppath, loop->getExit(), cs);
					tree = new CFTreeLoop(loop->getHeader(), loop->getBound(), body, exit);
				}
				else
				{
					tree = pseudoTree(ppath, loop->getExit(), cs);
				}
			}
			else
			{
				CFTreeAlt *alt = node->toAlt();
				std::vector<CFTree *> uniqueChild = findChilds(ppath, alt->childIter(), alt->childEnd(), cs);
				if (uniqueChild.size() != 0)
				{
					if (uniqueChild.size() == 1)
						tree = uniqueChild[0];
					else
						tree = new CFTreeAlt(uniqueChild);
				}
				else
				{
					std::vector<CFTree *> childs = pseudoAlt(ppath, alt->childIter(), alt->childEnd(), cs);
					if (childs.size() == 1)
						tree = childs[0];
					else
						tree = new CFTreeAlt(childs);
				}
			}

			// removing node constraints if any
			for (auto i = nodeCs->begin(); i != nodeCs->end(); i++)
			{
				cs.erase(*i);
			}
			// cs.erase(nodeCs.begin(), nodeCs.end());

			return tree;
		}

		///
		/// pseudo tree of a sequence node
		///
		std::vector<CFTree *> PseudoTreeBuilder::pseudoSeq(PseudoPath ppath, std::vector<otawa::cftree::CFTree *>::const_iterator childs, std::vector<otawa::cftree::CFTree *>::const_iterator end, std::set<Constraint> cs)
		{
			std::vector<CFTree *> childsVector;
			for (auto i = childs; i != end; i++)
			{
				childsVector.push_back(pseudoTree(ppath, *i, cs));
			}
			return childsVector;
		}

		///
		/// find the unique child of an alt node if exists
		///
		std::vector<CFTree *> PseudoTreeBuilder::findChilds(PseudoPath ppath, std::vector<otawa::cftree::CFTree *>::const_iterator childs, std::vector<otawa::cftree::CFTree *>::const_iterator end, std::set<Constraint> cs)
		{
			std::vector<CFTree *> childsArray;
			PseudoPathsBuilder pseudoPathsBuilder;
			for (auto i = childs; i != end; i++)
			{
				std::set<PseudoPath> pseudoPaths = pseudoPathsBuilder.getPPaths(cs, *i);
				for (auto j = pseudoPaths.begin(); j != pseudoPaths.end(); j++)
				{
					if ((*j).size() == 0)
					{
						continue;
					}
					else
					{
						bool skip = false;
						for (auto k = (*j).begin(); k != (*j).end(); k++)
						{
							if (ppath.find(*k) == ppath.end())
							{
								skip = true;
								break;
							}
						}
						if (!skip)
						{
							childsArray.push_back(pseudoTree(ppath, *i, cs));
							// return childsArray;
						}
					}
				}
			}
			return childsArray;
		}

		///
		/// get the pseudo tree of an alternative without unique path
		///
		std::vector<CFTree *> PseudoTreeBuilder::pseudoAlt(PseudoPath ppath, std::vector<otawa::cftree::CFTree *>::const_iterator childs, std::vector<otawa::cftree::CFTree *>::const_iterator end, std::set<Constraint> cs)
		{
			std::vector<CFTree *> newAlts;
			for (auto i = childs; i != end; i++)
			{
				CFTreeLoop *loop = (*i)->toLoop();
				if (!remove(ppath, *i, cs) || loop != NULL)
				{
					newAlts.push_back(pseudoTree(ppath, *i, cs));
				}
			}
			return newAlts;
		}

		///
		/// tels if a node has to be removed from the tree
		///
		bool PseudoTreeBuilder::remove(PseudoPath ppath, CFTree *node, std::set<Constraint> cs)
		{
			PseudoPathsBuilder pseudoPathsBuilder;

			if (CFTreeLoop *loop = node->toLoop())
			{
				// cout << "remove loop ?" << endl;
				if (remove(ppath, loop->getBody(), cs))
				{
					// cout << "OUI" << endl;
					return true;
				}
			}

			std::set<PseudoPath> ppaths = pseudoPathsBuilder.getPPaths(cs, node);
			for (auto i = ppaths.begin(); i != ppaths.end(); i++)
			{
				if ((*i).size() == 0)
				{
					return false;
				}
				for (auto j = (*i).begin(); j != (*i).end(); j++)
				{
					if (ppath.find(*j) != ppath.end())
					{
						return false;
					}
				}
			}
			return true;
		}

		///
		/// set a pseudo tree corresponding to a pseudo path
		///
		void CFTree::putPPath(PseudoPath pseudoPath, CFTree *pseudoTree)
		{
			mtx.lock();
			pseudoTrees.emplace(pseudoPath, pseudoTree);
			mtx.unlock();
		}

		///
		/// get the pseudo paths and the corresponding pseudo trees
		///
		std::map<PseudoPath, CFTree *> CFTree::getPPaths()
		{
			return pseudoTrees;
		}

		///
		/// Computes the WCET of a tree using the WCET of all possible pseudo  paths
		///
		void CFTree::exportToFeasibleWCET(formula_t *f, struct param_func *pfl)
		{
			std::vector<CFTree *> childs;
			for (auto i = pseudoTrees.begin(); i != pseudoTrees.end(); i++)
			{
				childs.push_back((*i).second);
			}
			CFTree *feasibleTree;
			if (childs.size() > 1)
			{
				feasibleTree = new CFTreeAlt(childs);
			}
			else
			{
				feasibleTree = childs[0];
			}
			StringBuffer buf;
			buf << "test-arm-pseudo-cftree_" << infeasibleTree++ << ".dot";

			// heuristic to duplicate only constrained nodes
			if (CFTreeSeq *seq = this->toSeq())
			{
				std::vector<CFTree *> sChilds = seq->getUnconstrainedChilds();
				if (sChilds.size() != 0)
				{
					sChilds.push_back(feasibleTree);
					CFTree *optimzedTree = new CFTreeSeq(sChilds);
					feasibleTree = optimzedTree;
				}
			}

			feasibleTree->exportToDot(buf.toString());
			feasibleTree->exportToAWCET(f, pfl);
		}

		/// **************************
		/// Replace in child list
		/// **************************
		void CFTreeLeaf::replace(CFTree *oldTree, CFTree *newTree)
		{ /* does nothing to the tree as it does not have a child */
		}
		void CFTreeAlt::replace(CFTree *oldTree, CFTree *newTree)
		{
			for (long unsigned int i = 0; i < alts.size(); i++)
			{
				if (alts[i] == oldTree)
				{
					alts[i] = newTree;
					break;
				}
			}
		}
		void CFTreeConditionalAlt::replace(CFTree *oldTree, CFTree *newTree)
		{
			for (long unsigned int i = 0; i < alts.size(); i++)
			{
				if (alts[i] == oldTree)
				{
					alts[i] = newTree;
					break;
				}
			}
		}
		void CFTreeSeq::replace(CFTree *oldTree, CFTree *newTree)
		{
			for (long unsigned int i = 0; i < childs.size(); i++)
			{
				if (childs[i] == oldTree)
				{
					childs[i] = newTree;
					break;
				}
			}
		}
		void CFTreeLoop::replace(CFTree *oldTree, CFTree *newTree)
		{
			if (bd == oldTree)
				bd = newTree;
			else if (ex == oldTree)
				ex = newTree;
		}

		std::set<Constraint> ConstraintCloner::getConstraintsContaining(int id, std::set<Constraint> cs)
		{
			std::set<Constraint> nodeConstraints;
			for (auto i = cs.begin(); i != cs.end(); i++)
			{
				if ((*i).find(id) != (*i).end())
				{
					nodeConstraints.insert(*i);
				}
			}
			return nodeConstraints;
		}

		std::set<Constraint> ConstraintCloner::replace(long oldId, long newId, std::set<Constraint> cs)
		{
			std::set<Constraint> newCs;
			for (auto i = cs.begin(); i != cs.end(); i++)
			{
				Constraint c = *i;
				Constraint newC;
				for (auto j = c.begin(); j != c.end(); j++)
				{
					if (*j == oldId)
					{
						newC.insert(newId);
					}
					else
					{
						newC.insert(*j);
					}
				}
				newCs.insert(newC);
				/*cout << "inserted : ";
				printPseudoPath(newC);
				cout << endl;*/
			}
			return newCs;
		}

		///
		/// replace a leaf in the map
		///
		void LCAFinder::replaceLeafId(CFTreeLeaf *leaf, CFTreeLeaf *newLeaf)
		{
			auto place = id.find(leaf);
			if (place != id.end())
			{
				int leafId = place->second;
				id.erase(place);
				id.emplace(newLeaf, leafId);
				allCopies.insert(newLeaf);
				// cout << "replaced leaf for " << leafId << endl;
			}
		}

		/* time measures */
		void Timer::start()
		{
			begin = std::chrono::high_resolution_clock::now();
		}

		void Timer::stop()
		{
			std::chrono::_V2::system_clock::time_point end = std::chrono::high_resolution_clock::now();
			duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
		}

		void Timer::printDuration()
		{
			double time = (double)duration.count() / (double)1000000;
			cout << time << "s";
		}

		BranchCondition::BranchCondition(address_t branchingBlockAddress, bool taken, std::string conditions)
		{
			this->address = branchingBlockAddress;
			this->taken = taken;
			this->conditions = conditions;
		}

		address_t BranchCondition::getAddress()
		{
			return address;
		}

		bool BranchCondition::isTaken()
		{
			return taken;
		}

		std::string BranchCondition::getConditions()
		{
			return conditions;
		}

		std::map<address_t, std::vector<std::map<bool, BranchCondition *> *> *> ConditionParser::readFromFile(const char *fileName)
		{
			std::ifstream file;
			file.open(fileName);
			std::string line_delimiter = LINE_DELIMITER;
			std::set<Constraint> constraints;

			// get each line of the file separated
			std::vector<std::string> lines;
			std::string str;
			while (std::getline(file, str))
			{
				// Line contains string of length > 0 then save it in vector
				if (str.size() > 0)
				{
				}
				lines.push_back(str);
			}

			std::vector<BranchCondition *> cdts;

			// get each constraint by splitting lines
			for (auto i = lines.begin(); i != lines.end(); i++)
			{

				// first line contains titles, we also ignore empty lines
				if (i == lines.begin() || *i == "")
					continue;

				const char *line = (*i).c_str();
				char *l = const_cast<char *>(line);

				// addr
				char *element = strtok(l, CSV_DELIM);
				address_t addr = atoi(element);

				// taken
				element = strtok(NULL, CSV_DELIM);
				bool taken = false;
				if (strcmp(element, "true") == 0)
				{
					taken = true;
				}

				// conditions
				element = strtok(NULL, CSV_DELIM);
				std::string conditions(element);

				BranchCondition *cdt = new BranchCondition(addr, taken, conditions);
				cdts.push_back(cdt);
			}

			// parse constraints and return parsed constraints
			return filterCdts(cdts);
		}

		std::map<address_t, std::vector<std::map<bool, BranchCondition *> *> *> ConditionParser::filterCdts(std::vector<BranchCondition *> cdts)
		{
			std::map<address_t, std::vector<std::map<bool, BranchCondition *> *> *> ret;

			for (auto i = cdts.begin(); i != cdts.end(); i++)
			{
				BranchCondition *cdt = *i;

				std::vector<std::map<bool, BranchCondition *> *> *branches;
				// check if the branch have been analyzed for another condition
				if (ret.find(cdt->getAddress()) != ret.end())
				{
					branches = ret.at(cdt->getAddress());
				}
				else
				{
					branches = new std::vector<std::map<bool, BranchCondition *> *>();
					ret.emplace(cdt->getAddress(), branches);
				}

				std::map<bool, BranchCondition *> *branch;
				// check if a branching is incomplete or create a new branching
				if (branches->size() != 0)
				{
					branch = branches->at(branches->size() - 1);
					// if condition already exists, create a new element
					if (branch->find(cdt->isTaken()) != branch->end())
					{
						branch = new std::map<bool, BranchCondition *>();
						branches->push_back(branch);
					}
				}
				// if the map does not exist, create a new one
				else
				{
					branch = new std::map<bool, BranchCondition *>();
					branches->push_back(branch);
				}

				// set the value in the right element
				branch->emplace(cdt->isTaken(), cdt);
			}

			return ret;
		}

		void ConditionParser::pushConditionsOnCFGs(const CFG *entry, std::map<address_t, std::vector<std::map<bool, BranchCondition *> *> *> *conditionsMap, std::map<int, LoopBound>* loopBounds)
		{
			Block *b = entry->entry();
			std::set<Block *> workingList;
			workingList.insert(b);

			// avoid infinite loop
			std::set<Block *> potenatialLoopHeaders;

			// parcours CFG
			while (workingList.size() > 0)
			{
				std::set<Block *> nextWL;

				for (auto i = workingList.begin(); i != workingList.end(); i++)
				{
					Block *currentBlock = *i;

					// set the loop bound on the basic block
					// FIXME LOOP_HEADER does not work here ?!?
					//if(LOOP_HEADER(currentBlock)){

						auto index = loopBounds->find(currentBlock->id());
						if(index != loopBounds->end()){
							LOOP_BOUND(currentBlock) = (*index).second;
						}
					//}

					// add node to seen list
					if (currentBlock->countOuts() > 1 && currentBlock->isBasic())
					{
						potenatialLoopHeaders.insert(currentBlock);

						Inst *controlInst = currentBlock->toBasic()->control();

						// is branching block
						if (controlInst != nullptr)
						{
							if (conditionsMap->find(controlInst->address()) != conditionsMap->end())
							{
								std::vector<std::map<bool, otawa::cftree::BranchCondition *> *> *vec = conditionsMap->at(controlInst->address());
								if(vec->size() != 0){
									std::map<bool, otawa::cftree::BranchCondition *> *mapOfInterest = vec->at(0);
									auto iter = currentBlock->outs();
									if(!LOOP_HEADER(currentBlock)){
										while (!iter.ended())
										{
											Edge *e = iter.item();
											if(!BACK_EDGE(e) && mapOfInterest->find(e->isTaken()) != mapOfInterest->end()){
												BranchCondition *bc = mapOfInterest->at(e->isTaken());
												CONDITION(e->sink()) = bc;
												// std::cout << "attached " << bc->getConditions() << " to BB" << e->sink()->id() << endl;
											}
											iter.next();
										}
									}
									// else{
									// 	cout << "DETECTED " << currentBlock->id() << " AS A LOOP HEADER, IGNORING CONDITIONS" << endl;
									// }
									vec->erase(vec->begin()); //< allows to manage several function calls
								}
							}
							// else{
							// 	cout << "[INFO] No condition found for: " << controlInst->address() << endl;
							// 	if(conditionsMap->size() > 0){
							// 		cout << "   EXISTING CONDITIONS:" << endl;
							// 		for(auto i = conditionsMap->begin(); i != conditionsMap->end(); i++){
							// 			cout << "      " << (*i).first << endl;
							// 		}
							// 	}
							// }
						}
					}
					else
					{
						if (currentBlock->isSynth())
						{
							CFG *cfg = currentBlock->toSynth()->callee();
							if(cfg != nullptr){
								// cout << "CALL TO " << cfg->name() << endl;
								pushConditionsOnCFGs(cfg, conditionsMap, loopBounds);
								// cout << "RETURN FROM " << cfg->name() << endl;
							}
							else{
								cerr << "ERROR: CFG " << currentBlock->cfg()->name() << " calls a null CFG" << endl;
								exit(-1);
							}
							
						}
					}
					// search into successors
					if (!currentBlock->isExit())
					{
						for (auto j = currentBlock->succs().begin(); j != currentBlock->succs().end(); j++)
						{
							// if not in already treated nodes
							if (potenatialLoopHeaders.find(*j) == potenatialLoopHeaders.end())
							{
								nextWL.insert(*j);
							}
						}
					}
				}
				workingList = nextWL;
			}
		}

		BranchCondition *ConditionParser::findConditionOfBranch(CFTree *branch)
		{
			// read conditions
			if (CFTreeLeaf *leaf = branch->toLeaf())
			{
				if (leaf->getBlock() != nullptr)
					return CONDITION(leaf->getBlock());
				else
					return nullptr;
			}
			// take last condition found
			if (CFTreeSeq *seq = branch->toSeq())
			{
				std::vector<CFTree *> children = seq->getChilds();
				BranchCondition *lastCondition = nullptr;
				for (long unsigned int i = 0; i != children.size(); i++)
				{
					// new alt = new condition = not in current scope and condition should be found before having this alt
					if (children[i]->toAlt())
						break;
					if (children[i]->toConditionalAlt())
						break;
					// fond condition for non alt elements
					BranchCondition *bc = findConditionOfBranch(children[i]);
					if (bc != nullptr)
						lastCondition = bc;
				}
				return lastCondition;
			}
			if (CFTreeLoop *loop = branch->toLoop())
				return findConditionOfBranch(loop->getBody());
			// a new alt = new branchs = other condition scope so we never analyse them
			return nullptr;
		}

		Identifier<BranchCondition *> CONDITION("otawa::cftree::CONDITION", nullptr);
		Identifier<std::map<int, int>> FUNCTION_PARAM_MAPPING("otawa::cftree::FUNCTION_PARAM_MAPPING", std::map<int, int>());

	
	/**********************
	* Parametric loop bounds
	***********************/
	LoopBound::LoopBound(int loopId = -1, int constant = -1){
		this->loopId = loopId;
		this->parametric = false;
		this->constant = constant;
	}

	LoopBound::LoopBound(int loopId, std::string expression){
		this->loopId = loopId;
		this->parametric = true;
		this->expression = expression;
	}

	bool LoopBound::isParametric(){
		return this->parametric;
	}

	int LoopBound::getConstant(){
		if(this->parametric){
			cerr << "ERROR: Trying to get a constant bound for a parametric loop" << endl;
			abort();
		}
		return this->constant;
	}

	std::string LoopBound::getExpression(){
		if(!(this->parametric)){
			cerr << "ERROR: Trying to get a parametric loop bound for a non parametric loop" << endl;
		}
		return this->expression;
	}

	std::map<int,LoopBound> LoopBoundParser::readFromFile(const char* fileName){
		std::map<int,LoopBound> lbs;
		std::ifstream file;
		file.open(fileName);
		
		std::string line;
		std::string str;
		bool fst = true;
		while(std::getline(file,str)){
			
			// ignore first line
			if(fst){
				fst = false;
				continue;
			}

			const char* line = str.c_str();
			char* l = const_cast<char*>(line);
			// loopId
			char* element = strtok(l, CSV_DELIM);
			int loopId = atoi(element);

			// parametric
			element = strtok(NULL, CSV_DELIM);
			bool parametric = true;
			if(strcmp(element, "false") == 0){
				parametric = false;
			}

			// bound
			element = strtok(NULL, CSV_DELIM);
			if(parametric){
				std::string expression(element);
				lbs.emplace(loopId, LoopBound(loopId, expression));
			}else{
				int constant = atoi(element);
				lbs.emplace(loopId, LoopBound(loopId, constant));
			}
		}

		return lbs;
	}

	/**
	 * A simple function to tell if a block is a parameter
	 * @param b the block which could be a parameter
	 */
	bool FParamManager::isParam(Block* b){
		if(b == nullptr || b->cfg() == nullptr || !b->isSynth() || !b->isCall())
			return false;
		struct param_func* pfl = PFL;
		int i = 0;
		std::string callee(b->toSynth()->callee()->label().chars());
		// search in parameters
		while(pfl != nullptr && pfl[i].funcname != nullptr){
			std::string fname(pfl[i++].funcname);
			if(fname == callee)
				return true;
		}
		return false;
	}

	Identifier<LoopBound> LOOP_BOUND("otawa::cftree::LOOP_BOUND", LoopBound());
	Identifier<bool> IS_AFTER_ALT("otawa::cftree::IS_AFTER_ALT", false);
	Identifier<bool> IS_LAST_IN_ALT("otawa::cftree::IS_LAST_IN_ALT", false);
	Identifier<bool> IS_PIPELINE_EDGE("otawa::cftree:IS_PIPELINE_EDGE", false);
	Identifier<Edge*> EDGE_ALIAS("otawa::cftree::EDGE_ALIAS", nullptr);
	Identifier<bool> IS_AA_LOOPEXIT("otawa::cfree::IS_AA_LOOPEXIT", false);
} // namespace cftree
} // namespace otawa
