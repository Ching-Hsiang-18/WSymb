#include "include/CFTree.h"

namespace otawa { 

namespace cftree {
// definition propriete cftree

//--------------------
//	CLASS CFTREELEAF
//--------------------

CFTreeLeaf* CFTreeLeaf::toLeaf() {
	return (CFTreeLeaf*) this;
}
CFTreeAlt* CFTreeLeaf::toAlt() {return NULL;}/* abstract */
CFTreeLoop* CFTreeLeaf::toLoop() {return NULL;}/* abstract */
CFTreeSeq* CFTreeLeaf::toSeq() {return NULL;}/* abstract */

CFTreeLeaf::CFTreeLeaf(Block *b) {
	block = b;
}

Block* CFTreeLeaf::getBlock() { return block; }
int CFTreeLeaf::getBlockId() const { return block->id(); }

//--------------------
//	CLASS CFTREEALT
//--------------------
CFTreeAlt::CFTreeAlt(std::vector<CFTree*> new_alts) {
	alts = new_alts;
}

CFTreeAlt* CFTreeAlt::toAlt() {
	return (CFTreeAlt*) this;
}
CFTreeLeaf* CFTreeAlt::toLeaf() {return NULL;}/* abstract */
CFTreeLoop* CFTreeAlt::toLoop() {return NULL;}/* abstract */
CFTreeSeq* CFTreeAlt::toSeq() {return NULL;}/* abstract */

void CFTreeAlt::addAlt(CFTree *s) {
	alts.push_back(s);
}

std::vector<CFTree *>::const_iterator CFTreeAlt::childIter() {
	return alts.begin();
}

std::vector<CFTree *>::const_iterator CFTreeAlt::childEnd() {
	return alts.end();
}

CFTree* CFTreeAlt::getI(size_t ind) {
	assert(ind < alts.size());
	return alts[ind];
}

//--------------------
//	CLASS CFTREESEQ
//--------------------

CFTreeSeq* CFTreeSeq::toSeq() {
	return (CFTreeSeq*) this;
}
CFTreeLeaf* CFTreeSeq::toLeaf() {return NULL;}/* abstract */
CFTreeLoop* CFTreeSeq::toLoop() {return NULL;}/* abstract */
CFTreeAlt* CFTreeSeq::toAlt() {return NULL;}/* abstract */

CFTreeSeq::CFTreeSeq(std::vector<CFTree*> new_childs) {
	childs = new_childs;
}

void CFTreeSeq::addChild(CFTree *s) {
	childs.push_back(s);
}

std::vector<CFTree *>::const_iterator CFTreeSeq::childIter() {
	return childs.begin();
}

std::vector<CFTree *>::const_iterator CFTreeSeq::childEnd() {
	return childs.end();
}

CFTree* CFTreeSeq::getI(size_t ind) {
	assert(ind < childs.size());
	return childs[ind];
}

//--------------------
//	CLASS CFTREELOOP
//--------------------

CFTreeLoop* CFTreeLoop::toLoop() {
	return (CFTreeLoop*) this;
}
CFTreeLeaf* CFTreeLoop::toLeaf() {return NULL;}/* abstract */
CFTreeSeq* CFTreeLoop::toSeq() {return NULL;}/* abstract */
CFTreeAlt* CFTreeLoop::toAlt() {return NULL;}/* abstract */

CFTreeLoop::CFTreeLoop(BasicBlock *h, int bound, CFTree* n_bd, CFTree *n_ex) {
	header = h;
	n = bound;
	bd = n_bd;
	ex = n_ex;
}

BasicBlock* CFTreeLoop::getHeader() { return header; }
int CFTreeLoop::getHeaderId() const { return header->id(); }

CFTree* CFTreeLoop::getBody(){
	return bd;
}

CFTree* CFTreeLoop::getExit(){
	return ex;
}

void CFTreeLoop::changeBound(int bound){
	n = bound;
}

//--------------------
//				CLASS DAG
//--------------------

std::vector<DAGNode*>::const_iterator DAG::iter() {
	return all.begin();
}

std::vector<DAGNode*>::const_iterator DAG::end() {
	return all.end();
}
void DAG::addNode(DAGNode *s) {
	all.push_back(s);
}

void DAG::setStart(DAGNode *s) {
	n_start = s;
}

void DAG::setVirtNext(DAGNode *s){
	n_virt_next = s;
}
void DAG::setVirtExit(DAGNode *s){
	n_virt_exit = s;
}

DAGNode* DAG::getStart(){
	return n_start;
}

DAGNode* DAG::getVirtNext(){
	return n_virt_next;
}
DAGNode* DAG::getVirtExit(){
	return n_virt_exit;
}

DAGNode* DAG::getElement(size_t i){
	return all[i];
}


void DAG::visit(std::function<void(DAGNode*)> &h, bool recursive) {
	for (std::vector<DAGNode*>::const_iterator it = all.begin(); it != all.end(); it++) {
		DAGNode *n = *it;
		h(n);
		if ((n->toHNode() != nullptr) && recursive) {
			n->toHNode()->getDag()->visit(h, recursive);
		}
	}

}

//----------------------
//		CLASS DAGNODE
//----------------------

void DAGNode::addSucc(DAGNode *s) {
	if (std::find(succ.begin(), succ.end(), s) == succ.end()) {
		succ.push_back(s);
	}
}

void DAGNode::addPred(DAGNode *s) {
	if (std::find(pred.begin(), pred.end(), s) == pred.end()) {
		pred.push_back(s);
	}
}

std::vector<DAGNode*>::const_iterator DAGNode::succIter() {
	return succ.begin();
}

std::vector<DAGNode*>::const_iterator DAGNode::predIter() {
	return pred.begin();
}

std::vector<DAGNode*>::const_iterator DAGNode::succEnd() {
	return succ.end();
}

std::vector<DAGNode*>::const_iterator DAGNode::predEnd() {
	return pred.end();
}

std::vector<DAGNode*> DAGNode::getSucc(){
	return succ;
}

std::vector<DAGNode*> DAGNode::getPred(){
	return pred;
}

//----------------------
//		CLASS DAGHNODE
//----------------------

DAGHNode* DAGHNode::toHNode() {
	return (DAGHNode*) this;
}
DAGVNode* DAGHNode::toVNode() { return NULL; }
DAGBNode* DAGHNode::toBNode() { return NULL; }

DAGHNode::DAGHNode(BasicBlock *b) {
	header = b;
	DAG_HNODE(header) = this;
}

void DAGHNode::setDAG(DAG *dag){
	sub_dag = dag;
}

BasicBlock* DAGHNode::getHeader() { return header; }
DAG* DAGHNode::getDag() {return sub_dag;}
int DAGHNode::getLoopId() const { return header->id(); }

//----------------------
//		CLASS DAGVNODE
//----------------------

DAGVNode* DAGVNode::toVNode() {
	return (DAGVNode*) this;
}
DAGHNode* DAGVNode::toHNode() { return NULL; }
DAGBNode* DAGVNode::toBNode() { return NULL; }

DAGVNode::DAGVNode(int _type) { type = _type; }
int DAGVNode::getType() const { return type;}

//----------------------
//		CLASS DAGBNODE
//----------------------

DAGBNode* DAGBNode::toBNode() {
	return (DAGBNode*) this;
}
DAGHNode* DAGBNode::toHNode() { return NULL; }
DAGVNode* DAGBNode::toVNode() { return NULL; }

DAGBNode::DAGBNode(Block *b) {
	_synth = b->isSynth();
	block = b;
	if (_synth)
		callee = b->toSynth()->callee();
	
	DAG_BNODE(block) = this;
}

Block* DAGBNode::getBlock() { 
	return block; 
}

bool DAGBNode::isSynth() {
	return _synth;
}

CFG* DAGBNode::getCallee() {
	ASSERT(isSynth());
	return callee;
}

int DAGBNode::getBlockId() const { return block->id(); }

DAG::~DAG()
{
  for (/* std::vector<DAGNode*>::iterator */ auto it = iter(); it != end(); it++) {
	  DAGNode *n = (*it);
	  delete n;
  }
}

//----------------------
//		CLASS PLUGIN
//----------------------

class Plugin : public ProcessorPlugin { // plugin qui va transformer le CFG
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

void CFTreeExtractor::configure(const PropList &props) {
        // configure CFTreeExtractor avec la PropList
	Processor::configure(props);
}

//---------------------------------------
//				PRETTYPRINTER FOR DAG
//---------------------------------------

io::Output &operator<<(io::Output &o, const DAGVNode &n) {
	if (n.getType() == VNODE_EXIT) {
		return o << "EXIT";
	} else if (n.getType() == VNODE_NEXT) {
		return o << "NEXT";
	} else ASSERT(false);
}

io::Output &operator<<(io::Output &o, const DAGBNode &n) {
	return (o << "B_" << n.getBlockId());
}
io::Output &operator<<(io::Output &o, const DAGHNode &n) {
	return (o << "LH_" << n.getLoopId());
}

io::Output &operator<<(io::Output &o, DAGNode &n) {
	if (n.toVNode())
		o << *n.toVNode();
	if (n.toBNode())
		o << *n.toBNode();
	if (n.toHNode())
		o << *n.toHNode();
	return o;
}

io::Output &operator<<(io::Output &o, DAG &n) { // POUR DEBUG
	o << "\n";
	o << "start "<< *n.getStart() << "\n";
	for (auto it = n.iter(); it != n.end(); it++) {
		DAGNode *n = (*it);
		//o << (n->getSucc()).size() << endl;
		o << *n << " -> ";
		for (auto it2 = n->succIter(); it2 != n->succEnd(); it2++) {
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

io::Output &operator<<(io::Output &o, CFTreeLeaf &n) {
	return (o << "CFTreeLeaf(B_" << n.getBlockId()) << ")";
}

io::Output &operator<<(io::Output &o, CFTreeAlt &n) {
		o << "\n";
		o << "CFTreeAlt( \n";
		for (auto it = n.childIter(); it != n.childEnd(); it++) {
			CFTree *n = (*it);
			o << "\t" << *n << "\n";
		}
		o << ") \n";
		return o;
}

io::Output &operator<<(io::Output &o, CFTreeLoop &n) {
		o << "\n";
		o << "CFTreeLoop(B_" <<   (n.getHeaderId()) << " \n";
		o << "CFTreeLoop.body : " << *(n.getBody()) << "\n";
		o << "CFTreeLoop.body : " << *(n.getExit()) << "\n";
		o << ") \n";
		return o;
}

io::Output &operator<<(io::Output &o, CFTreeSeq &n) { // POUR DEBUG
	o << "\n";
	// o << "start "<< *n.getStart() << "\n";
	o << "CFTreeSeq( \n";
	for (auto it = n.childIter(); it != n.childEnd(); it++) {
		CFTree *n = (*it);
		o << "\t" << *n << "\n";
	}
	o << ") \n";
	return o;
}

io::Output &operator<<(io::Output &o, CFTree &n) {
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
static std::string write_tree(CFTreeSeq &n, unsigned int *lab);
static std::string write_tree(CFTreeLeaf &n, unsigned int *lab);
static std::string write_tree(CFTree &n, unsigned int *lab);

static std::string write_tree(CFTreeLoop &n, unsigned int *lab){
	std::stringstream ss;
	unsigned int loop_lab = *lab;
	ss << "l" << loop_lab << " [label = \"Loop(b"<< n.getHeaderId() << ")\"]; \n";

	// body
	CFTree *ch = n.getBody();
	unsigned int old_lab = ++(*lab);
	ss << write_tree(*ch, lab);
	ss << "l"<< loop_lab << " -> { l"<< old_lab <<" }; \n";

	unsigned int exit_lab = ++(*lab);
	ss << "l" << exit_lab << " [label = \" Exit\"]; \n";
	ss << "l"<< loop_lab << " -> { l"<< exit_lab <<" }; \n";
	// exit
	ch = n.getExit();
	old_lab = ++(*lab);
	ss << write_tree(*ch, lab);
	ss << "l"<< exit_lab << " -> { l"<< old_lab <<" }; \n";
	return ss.str();
}


static std::string write_tree(CFTreeAlt &n, unsigned int *lab){
	std::stringstream ss;
	unsigned int alt_lab = *lab;
	ss << "l" << alt_lab << " [label = \" Alt\"]; \n";
	for (auto it = n.childIter(); it != n.childEnd(); it++) {
		CFTree *ch = (*it);
		unsigned int old_lab = ++(*lab);
		ss << write_tree(*ch, lab);
		ss << "l"<< alt_lab << " -> { l"<< old_lab <<" }; \n";
	}
	return ss.str();
}

static std::string write_tree(CFTreeSeq &n, unsigned int *lab){
	std::stringstream ss;
	unsigned int seq_lab = *lab;
	ss << "l" << seq_lab << " [label = \" Seq\"]; \n";
	for (auto it = n.childIter(); it != n.childEnd(); it++) {
		CFTree *ch = (*it);
		unsigned int old_lab = ++(*lab);
		ss << write_tree(*ch, lab);
		ss << "l"<< seq_lab << " -> { l"<< old_lab <<" }; \n";
	}
	return ss.str();
}

static std::string write_tree(CFTreeLeaf &n, unsigned int *lab){
	std::stringstream ss;
	ss << "l" << *lab << " [label = \"";
	if (n.getBlock() == nullptr) {
		ss << "NONE";
	} else if (n.getBlock()->isSynth()) {
		const char *fname =  n.getBlock()->toSynth()->callee()->label().toCString();
		ss << "CALL_" << fname;
	} else {
		ss << "B_" << n.getBlockId();
	}
	ss <<  "\"]; \n";
	return ss.str();
}

static std::string write_tree(CFTree &n, unsigned int *lab) {
	if (n.toLeaf())
		return write_tree(*n.toLeaf(), lab);
	if (n.toAlt())
		return write_tree(*n.toAlt(), lab);
	if (n.toLoop())
		return write_tree(*n.toLoop(), lab);
	if (n.toSeq())
		return write_tree(*n.toSeq(), lab);
	return "";
}

static io::Output& write_code(io::Output&, CFTreeLoop &n, unsigned int indent);
static io::Output& write_code(io::Output&,CFTreeAlt &n, unsigned int indent);
static io::Output& write_code(io::Output&,CFTreeSeq &n, unsigned int indent);
static io::Output& write_code(io::Output&,CFTreeLeaf &n, unsigned int indent);
static io::Output& write_code(io::Output&,CFTree &n, unsigned int indent);

static io::Output& write_code(io::Output&o,CFTreeLoop &n, unsigned int indent){
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


static io::Output& write_code(io::Output&o,CFTreeAlt &n, unsigned int indent){
	bool first = true;
	bool has_none = false;
	unsigned int count = 0;
	unsigned int i;
	for (auto it = n.childIter(); it != n.childEnd(); it++) {
		CFTree *t = (*it);
		if (t->toLeaf() && t->toLeaf()->getBlock() == nullptr)  {
			has_none = true;
			continue;
		}
		for (i = 0; i < indent; i++) 
			o << " ";
		if (first) {
			o << "if (...) {\n";
		} else {
			if (has_none || count < (n.size() - 1)) {
				o << "} else if (...) {\n";
			} else {
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

static io::Output& write_code(io::Output&o,CFTreeSeq &n, unsigned int indent){
	for (auto it = n.childIter(); it != n.childEnd(); it++) {
		CFTree *t = (*it);
		o = write_code(o, *t, indent);
	}
	return o;
}

static io::Output& write_code(io::Output&o,CFTreeLeaf &n, unsigned int indent){
	for (unsigned int i = 0; i < indent; i++) 
		o << " ";
	if (n.getBlock()->isSynth()) {
		o << n.getBlock()->toSynth()->callee()->label() << "(...);\n";
	} else {
		o << "B" << n.getBlock()->toBasic()->id() << ";\n";
	}
	return o;
}

static io::Output& write_code(io::Output&o,CFTree &n, unsigned int indent) {
	if (n.toLeaf())
		return write_code(o, *n.toLeaf(), indent);
	if (n.toAlt())
		return write_code(o, *n.toAlt(), indent);
	if (n.toLoop())
		return write_code(o, *n.toLoop(), indent);
	if (n.toSeq())
		return write_code(o, *n.toSeq(), indent);
	abort();
}

//=================================================


//-------------------------------
//	METHODS TO CONSTRUCT DAG
//-------------------------------

/*
	Returns the list of blocks contained in the loop that has b as header
*/
void getAllBlocksLoop(DAG *dag, std::vector<DAGBNode*> *blocks, std::vector<DAGHNode*> *lh_blocks, CFG *cfg, BasicBlock *l_h) {
	for (CFG::BlockIter iter(cfg->blocks()); iter(); iter++){
		Block *bb = *iter; // bloc qu'on veut ajouter
		Block *bb2 = ENCLOSING_LOOP_HEADER(bb); // pointeur
		if ((bb2 == l_h) || (bb == l_h)) { // on ajoute les blocs qui sont directement dans la boucle l_h
			if (bb->isBasic() || bb->isSynth()) {
				Block *b = bb;
				if (LOOP_HEADER(bb) && b != l_h){ // si un autre loop_header
					// cout << "Je rentre ici" << endl;

					DAGHNode *n = new DAGHNode(b->toBasic()); //headers are always basic
					dag->addNode(n);
					lh_blocks->push_back(n);
				}
				else { // sinon on va l'ajouter au DAG
					// cout << "cree dagnode pour bb: " << b << endl;
					DAGBNode *n = DAG_BNODE(b);
					if (n == NULL) { // Si il n'a jamais été ajouté
						n = new DAGBNode(b);
						dag->addNode(n);
					}
					for (Block::EdgeIter it2(b->outs()); it2(); it2++) { // on cherche les successeurs du bloc
						Edge *edge = *it2;
						if (LOOP_EXIT_EDGE(edge) == NULL){
							if (edge->sink()->isBasic() || edge->sink()->isSynth()) { // si pointe vers un autre bloc basic

								Block *bbs = edge->sink();
								// cout << "lien de b1 : " << b << "vers b2 : " << bbs << endl;
								if (LOOP_HEADER(bbs) /*  && bbs != l_h */)
									continue;

								DAGNode *ns = DAG_BNODE(bbs);
								if (ns == NULL) {
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
void setEntryDAG(CFG *cfg, DAG *dag){
	for (CFG::BlockIter iter(cfg->blocks()); iter(); iter++){
			Block *bb = *iter;
			if (bb->isEntry()){
				for (Block::EdgeIter it(bb->outs()); it(); it++){
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
void addEdgesNext(DAG *dag, DAGBNode *n, BasicBlock *l_h, DAGVNode *next) {
	Block *bb = n->getBlock();
	for (Block::EdgeIter it(bb->outs()); it(); it++) {
		Edge *edge = *it;
		Block *target = edge->target();
		if (target->isBasic() || target->isSynth()) {
			if (target == l_h) {
				n->addSucc(next);
				next->addPred(n);
				//Edge::Edge next_edge;
				// add (Block *v, Block *w, Edge *e)
			}
		} else if (target->isExit()) {
			ASSERT(l_h == NULL);
			if (l_h == NULL) {
					n->addSucc(next);
					next->addPred(n);
			}
		}
	}
}

/*
	Returns the Edges(s) that exit of l_h loop and add edge
*/
void addEdgesExit(DAG *dag, DAGBNode *n, BasicBlock *l_h, DAGVNode *exit) {
	Block *bb = n->getBlock();
	for (Block::EdgeIter it(bb->outs()); it(); it++) {
		Edge *edge = *it;
		if (LOOP_EXIT_EDGE(edge) != nullptr) { // si il y a un edge qui sort de la loop
			n->addSucc(exit);
			exit->addPred(n);
		}
		else if (l_h == NULL){
			Block *target = edge->target();
			 if (target->isExit()){
				 n->addSucc(exit);
				 exit->addPred(n);
			 }
		}
	}
}

bool contains(DAG *d, Block *b, bool recursive=false) {
	bool res = false;
	std::function<void(DAGNode*)> fn = [b, &res](DAGNode *n) {
		if (n->toBNode()) {
			Block *bb = n->toBNode()->getBlock();
			if (bb == b) 
				res = true;
		}
	};
	d->visit(fn, recursive);
	return res;
}

bool loop_contains(BasicBlock *h, Block *b) {
	if (b == h)
		return true;
	Block *current = b;
	while (ENCLOSING_LOOP_HEADER(current) != nullptr) {
		current = ENCLOSING_LOOP_HEADER(current);
		if (current == b)
			return true;
	}
	return false;
}

DAG* CFTreeExtractor::toDAG(CFG *cfg /* CFG a dag-ifier */ , BasicBlock *l_h /* boucle a dag-ifier, ou NULL si racine du CFG */ ){
	// D'abord on cree un DAG vide avec juste les noeuds virtuels EXIT et NEXT
	DAG *dag = new DAG();
	DAGVNode *next = new DAGVNode(VNODE_NEXT); // noeud NEXT en cours de construction
	DAGVNode *exit = new DAGVNode(VNODE_EXIT); // noeud EXIT 
	dag->addNode(next);
	dag->addNode(exit);
	dag->setVirtNext(next);
	dag->setVirtExit(exit);

	// blocks : tous les blocs non-headers qui sont directement dans l_h
	std::vector<DAGBNode*> blocks; //Bd moins les headers de boucle

	// lh_blocks : tous les blocs headers qui sont directement dans l_h
	std::vector<DAGHNode*> lh_blocks; //lh' tous les headers de boucle

	//std::vector<DAG*> sub_cfg;
	otawa::cftree::getAllBlocksLoop(dag, &blocks, &lh_blocks, cfg, l_h);

	for (unsigned i = 0; i < lh_blocks.size(); i++){ // construit les dags des sous boucles directs
		//sub_cfg.push_back(toDAG(cfg, lh_blocks[i]->getHeader()));
		lh_blocks[i]->setDAG(toDAG(cfg, lh_blocks[i]->getHeader()));
	}
	
	// Maintenant on a dans lh_blocks[i]->getDag() tous les sous-dags directs. 

	/* Relier les basic blocks du DAG actuel aux noeuds virtuels NEXT et EXIT */	
	for (unsigned i = 0; i < blocks.size(); i++) { 
		addEdgesNext(dag, blocks[i], l_h, next);
		addEdgesExit(dag, blocks[i], l_h, exit);
	}

	/* Relier les superblocks du DAG actuel aux noeuds virtuels NEXT et EXIT */
	for (unsigned i = 0; i < lh_blocks.size(); i++) { 
		DAG *d = lh_blocks[i]->getDag();
		std::function<void(DAGNode*)> fn = [d, exit,l_h, next](DAGNode *n) {
			if (n->toBNode()) {
				Block *bb = n->toBNode()->getBlock();
				for (Block::EdgeIter it(bb->outs()); it(); it++) {
					if (l_h == nullptr) {
						if (it->target()->isExit()) {
							n->addSucc(next);
							next->addPred(n);

							n->addSucc(exit);
							exit->addPred(n);
						}
					} else {
						if (it->target() == l_h) {
							n->addSucc(next);
							next->addPred(n);
						}
						Edge *e = *it;
						BasicBlock *edgehdr = LOOP_EXIT_EDGE(e) ? LOOP_EXIT_EDGE(e)->toBasic() : nullptr;
						if (loop_contains(edgehdr, l_h)) {
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


	for (unsigned i = 0; i < lh_blocks.size(); i++) {
		// Add edges that starts from lh_blocks[i]. Can start from any node in superblock
		std::function<void(DAGNode*)> fn = [&lh_blocks, i, &blocks, &cfg] (DAGNode *n) {
			if (n->toBNode()) {
				Block *bb = n->toBNode()->getBlock();
				for (Block::EdgeIter it(bb->outs()); it(); it++) {
					Edge *e = *it;
					Block *target = e->target();
					// add edges that go to basic blocks
					for (unsigned int j = 0; j < blocks.size(); j++) {
						if (blocks[j]->getBlock() == target) {
							lh_blocks[i]->addSucc(blocks[j]);
							blocks[j]->addPred(lh_blocks[i]);
						}
					}
					// add edges that go to super blocks. Can go only to header.
					for (unsigned int j = 0; j < lh_blocks.size(); j++) {
						if (i != j && contains(lh_blocks[j]->getDag(), target)) {
							if (target != lh_blocks[j]->getHeader()) {
								cerr << "CFG " << cfg->label() 
									<< " contains an irreducible loop, edge=" 
									<< e->source()->id() << "->" 
									<< e->target()->id() << 
									" header=" << 
									lh_blocks[j]->getHeader()->id() << "\n";
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
		for (BasicBlock::EdgeIter it(bb->ins()); it(); it++) {
			Edge *e = *it;
			Block *source= e->source();
			// Since the CFG contains no irreducible loops, if l_h!=nullptr, then source is contained in l_h
			// add edges that come from basic blocks
			for (unsigned int j = 0; j < blocks.size(); j++) {
				if (blocks[j]->getBlock() == source) {
					lh_blocks[i]->addPred(blocks[j]);
					blocks[j]->addSucc(lh_blocks[i]);
				}
			}
			// add edges that come from super blocks. Can come from any block inside the superblock
			for (unsigned int j = 0; j < lh_blocks.size(); j++) {
				if (i != j && contains(lh_blocks[j]->getDag(), source)) {
					lh_blocks[i]->addPred(lh_blocks[j]);
					lh_blocks[j]->addSucc(lh_blocks[i]);
				}
			}
		}
	}	


	if (l_h != NULL){ 
		DAGBNode *n = DAG_BNODE(l_h);
		ASSERT(n != nullptr);
		dag->setStart(n);
	} else {
		setEntryDAG(cfg, dag);
	}
	return dag;
}

/*
Vérifie qu'un noeud est dominant à un autre
*/
int DAGNodeDominate(DAGNode *b1, DAGNode *b2){

	if (b1 == b2) return 1;
	
	if (b1->toVNode()) return 0;

	Block *bb1 = b1->toBNode() ? b1->toBNode()->getBlock() : b1->toHNode()->getHeader();
	
	if (b2->toVNode()) {
		for (auto pred = b2->predIter(); pred != b2->predEnd(); pred++)  {
			if (!DAGNodeDominate(b1, *pred))
				return 0;
		}
		return 1;
	} else {
	
		Block *bb2 = b2->toBNode() ? b2->toBNode()->getBlock() : b2->toHNode()->getHeader();
		if (Dominance::dominates(bb1, bb2)){
			return 1;
		}
	}
	return 0;
}

/*
Retourne les blocks qui sont présents dans tous les chemins entre start et end
*/
void forcedPassedNodes(DAG *dag, DAGNode *start, DAGNode *end, std::vector<DAGNode*> *fpn){
	// std::vector<DAGNode*> *fpn;

	// pour tous les noeuds du dag
	for (auto it = dag->iter(); it != dag->end(); it++) {
		DAGNode *n = (*it);
		if (start != n){
			if (DAGNodeDominate(start, n) && DAGNodeDominate(n, end)) {
				fpn->push_back(n);
			}
		}
	}
}

void pouet() {
}

/*
Retourne le noeud qui est dominé par tous les autres
*/
DAGNode* getDominated(std::vector<DAGNode*> fpn){
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

CFTree* toCFT(DAG *dag, DAGNode *start, DAGNode *end, int all){
	std::vector<CFTree*> ch;
	std::vector<DAGNode*> fpn;
	
	forcedPassedNodes(dag, start, end, &fpn);

	if (start == end){
		if (start->toBNode()){
			Block *bb =  start->toBNode()->getBlock();
			return (new CFTreeLeaf(bb));
		}
	}

	while (fpn.size() > 0){
		DAGNode *c = getDominated(fpn); // l'element dominé
		std::vector<DAGNode*>::iterator it = std::find(fpn.begin(), fpn.end(), c); // retrouver l'index de l'élement dominé
		fpn.erase(it); // N <- N \ c

		if (c->toBNode()){
			Block *bb =  c->toBNode()->getBlock();
			ch.insert(ch.begin(), new CFTreeLeaf(bb));
		}
		// Si on a une loop interne, on construit le CFTree associé
		else if ( c->toHNode()){
			DAGHNode *lh = c->toHNode();
			DAG * sub_dag = lh->getDag();
			CFTree *bd = toCFT(lh->getDag(), sub_dag->getStart(), sub_dag->getVirtNext(), 1);
			CFTree *ex = toCFT(lh->getDag(), sub_dag->getStart(), sub_dag->getVirtExit(), 1);

			ASSERT(sub_dag->getStart()->toBNode());
			if ((sub_dag->getStart()) -> toBNode()){
				// bb is a loop header, therefore it is always basic
				BasicBlock *bb = ((sub_dag->getStart())->toBNode())->getBlock()->toBasic();
				ch.insert(ch.begin(), new CFTreeLoop(bb, 0, bd, ex));
			}
		}

		// Si c a plusieurs prédécesseurs, càd qu'il y a un if avant
		if (c->getPred().size() > 1){
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

			std::vector<CFTree*> br;

			// On construit le CFTreeAlt
			for (auto pred = c->predIter(); pred != c->predEnd(); pred++){
				if (*pred == ncd) {
					br.push_back(new CFTreeLeaf(nullptr));
				} else {
					br.push_back(toCFT (dag, ncd, *pred, 0));
				}
			}
			if (br.size() == 1) {
				ch.insert(ch.begin(), br[0]);
			} else {
				ch.insert(ch.begin(), new CFTreeAlt(br));
			}


		}


	}

	ASSERT(!all || (start-> getPred().size() == 0));
	if (all) {
		Block *bb =  start->toBNode()->getBlock();
		ch.insert(ch.begin(), new CFTreeLeaf(bb));
	}

	if (ch.size() == 1) {
		return ch[0];
	} else {
		CFTreeSeq* t_ch = new CFTreeSeq(ch);
		return t_ch;
	}
}


void CFTree::exportToDot(const elm::string &str) {
	std::ofstream myfile;
	myfile.open(str.toCString());
	unsigned int lab = 0;
	std::stringstream ss;
	ss << "digraph BST { \n" << write_tree(*this, &lab) << "}";
	myfile << ss.str() ;
	myfile.close();
}
void CFTree::exportToC(io::Output &out) {
	unsigned int indent = 2;
	write_code(out, *this, indent);
}


CFTree* CFTreeExtractor::processCFG(CFG *cfg) {
	bool reachable = false;
	for (CFG::BlockIter iter(cfg->blocks()); !reachable && iter(); iter++){
		if ((*iter)->isExit()) {
			for (Block::EdgeIter it((*iter)->ins()); !reachable && it(); it++)
				reachable = true;
		}
	}
	if (!reachable) {
		cout << "Not processing CFG: " << cfg->label() << " because EXIT is not reachable\n";
		return nullptr;
	}
	cout << "Processing CFG: " << cfg->label() << "\n";
	DAG *dag = toDAG(cfg, NULL);
	CFTree *tree = toCFT(dag, dag->getStart(), dag->getVirtExit(), 1);

	CFTREE(cfg) = tree;

	return tree;
}

CFTCollection::CFTCollection(std::vector<CFTree*> v){
	cfts = v;
}

void CFTreeExtractor::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	for (CFGCollection::Iter iter(*coll); iter(); iter ++) {
		CFG *currentCFG = *iter;
		processCFG(currentCFG);
	}

}

Identifier<DAGHNode*> DAG_HNODE("otawa::cftree:DAG_HNODE");
Identifier<DAGBNode*> DAG_BNODE("otawa::cftree:DAG_BNODE");
Identifier<CFTree*> CFTREE("otawa::cftree:CFTREE");

} }
