#include "include/CFTree.h"

namespace otawa { namespace cftree {
// using namespace otawa;

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

CFTreeLeaf::CFTreeLeaf(BasicBlock *b) {
	block = b;
}

BasicBlock* CFTreeLeaf::getBlock() { return block; }
int CFTreeLeaf::getBlockId() const { return block->id(); }

//--------------------
//	CLASS CFTREEALT
//--------------------

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

CFTree* CFTreeAlt::getI(int ind) {
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

CFTree* CFTreeSeq::getI(int ind) {
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

CFTreeLoop::CFTreeLoop(BasicBlock *h, int bound, CFTree* n_bd, std::vector<CFTree *> n_ex) {
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

CFTree* CFTreeLoop::getiExit(int ind){
	return ex[ind];
}

void CFTreeLoop::addT1(CFTree *t){
	bd = t;
}

void CFTreeLoop::addT2(CFTree *t){
	ex.push_back(t);
}

void CFTreeLoop::changeBound(int bound){
	n = bound;
}

std::vector<CFTree *>::const_iterator CFTreeLoop::iter_e() {
	return ex.begin();
}


std::vector<CFTree *>::const_iterator CFTreeLoop::end_e() {
	return ex.end();
}

/*
TODO : add semantics in CFT
*/
// template <class T>
// class CFTSemantic {
// 	virtual T evalAlt(std::vector<T*> *list) = 0;
// 	virtual T evalSeq(std::vector<T*> *list) = 0;
// 	virtual T evalLoop(T *body, T *exit, BasicBlock *header, int n) = 0;
// 	virtual T evalLeaf(BasicBlock *bb) = 0;
// };
//
// template <class T>
// class EvaluateCFT {
//   public:
// 	static T evaluate(CFTree *cft, CFTSemantic<T> *sem) {
// 	}
// };
//
//
// class NodeCount: public CFTSemantic<int> {
// 	virtual int evalAlt(std::vector<int*> *list) {
// 		int sum = 0;
// 		for (int i = 0; i < list->size(); i++) {
// 			sum += *(list->at(i));
// 		}
// 	}
// 	virtual int evalSeq(std::vector<int*> *list) {
// 		int sum = 0;
// 		for (int i = 0; i < list->size(); i++) {
// 			sum += *(list->at(i));
// 		}
// 	}
// 	virtual int evalLoop(int *body, int *exit, BasicBlock *header, int n) {
// 		return *body + *exit;
// 	}
// 	virtual int evalLeaf(BasicBlock *bb) {
// 		return 1;
// 	}
// };
//
// int countNodes(CFTree *cft) {
// 	NodeCount nc;
// 	return EvaluateCFT<int>::evaluate(cft, &nc);
// }

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

void DAG::setStar(DAGNode *s) {
	n_start = s;
}

void DAG::addEnd(DAGNode *s) {
	n_ends.push_back(s);
}

void DAG::setNext(DAGNode *s){
	n_next = s;
}

DAGNode* DAG::getStart(){
	return n_start;
}

DAGNode* DAG::getNext(){
	return n_next;
}

DAGNode* DAG::getElement(unsigned i){
	return all[i];
}

std::vector<DAGNode*>::const_iterator DAG::iter_e(){
	return n_ends.begin();
}

std::vector<DAGNode*>::const_iterator DAG::end_e(){
	return n_ends.end();
}

DAGNode* DAG::getiEnd(int i){
	assert(i < n_ends.size());
	return n_ends[i];
}

bool DAG::find_node(DAGNode *elem){ // cherche si un bloc est présent dans le dag
	std::vector<DAGNode*>::const_iterator it;
	it = find (iter(), end(), elem);
	return (it != end());
}

//----------------------
//		CLASS DAGNODE
//----------------------

void DAGNode::addSucc(DAGNode *s) {
	succ.push_back(s);
}

void DAGNode::addPred(DAGNode *s) {
	pred.push_back(s);
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

DAGBNode::DAGBNode(BasicBlock *b) {
	block = b;
	DAG_BNODE(block) = this;
}

BasicBlock* DAGBNode::getBlock() { return block; }
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
	}
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
	// o << "start "<< *n.getStart() << "\n";
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
		o << "CFTreeLoop.exit : " << "\n";
		for (auto it = n.iter_e(); it != n.end_e(); it++) {
			CFTree *ex = (*it);
			o << *ex << "\n";
		}
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
//				DOT FOR CFG
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
	for (auto it = n.iter_e(); it != n.end_e(); it++) {
		ch = *it;
		old_lab = ++(*lab);
		ss << write_tree(*ch, lab);
		ss << "l"<< exit_lab << " -> { l"<< old_lab <<" }; \n";
	}
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
	ss << "l" << *lab << " [label = \"B_"<< n.getBlockId() << "\"]; \n";
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

//=================================================


//-------------------------------
//	METHODS TO CONSTRUCT DAG
//-------------------------------

/*
	Returns the list of blocks contained in the loop that has b as header
*/
void getAllBlocksLoop(DAG *dag, std::vector<DAGBNode*> *blocks, std::vector<DAGHNode*> *lh_blocks, CFG *cfg, BasicBlock *l_h) {
	for (CFG::BlockIter iter(cfg->blocks()); iter; iter++){
		Block *bb = *iter; // bloc qu'on veut ajouter
		Block *bb2 = ENCLOSING_LOOP_HEADER(bb); // pointeur
		if ((bb2 == l_h) || (bb == l_h)) { // on ajoute les blocs qui sont directement dans la boucle l_h
			if (bb->isBasic()) {
				BasicBlock *b = bb->toBasic();
				if (LOOP_HEADER(bb) && b != l_h){ // si un autre loop_header
					// cout << "Je rentre ici" << endl;

					DAGHNode *n = new DAGHNode(b);
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
					for (BasicBlock::EdgeIter it2(b->outs()); it2; it2++) { // on cherche les successeurs du bloc
						Edge *edge = *it2;
						if (LOOP_EXIT_EDGE(edge) == NULL){
							if (edge->sink()->isBasic()) { // si pointe vers un autre bloc basic

								BasicBlock *bbs = edge->sink()->toBasic();
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
	for (CFG::BlockIter iter(cfg->blocks()); iter; iter++){
			Block *bb = *iter;
			if (bb->isEntry()){
				for (Block::EdgeIter it(bb->outs()); it; it++){
					Edge *edge = *it;
					Block *start = edge->target();
					BasicBlock *b_start = *start->toBasic();
					DAGBNode *n = DAG_BNODE(b_start);
					dag->setStar(n);
			}
		}
	}
}

/*
*/
void setExitDAG(CFG *cfg, DAG *dag){
	for (CFG::BlockIter iter(cfg->blocks()); iter; iter++){
			Block *exit = *iter;
			if (exit->isExit()){
				for (CFG::BlockIter iter(cfg->blocks()); iter; iter++){
					Block *bb = *iter;
					for (Block::EdgeIter it(bb->outs()); it; it++){
						Edge *edge = *it;
						Block *exit2 = edge->target();
						if (exit == exit2){
							BasicBlock *b_end = *bb->toBasic();
							DAGBNode *n = DAG_BNODE(b_end);
							dag->addEnd(n);
							return;
						}
				}
			}
		}
	}
}

/*
	Returns the Edge(s) that iterated on the l_h block and add edge
*/
void addEdgesNext(DAG *dag, DAGBNode *n, BasicBlock *l_h, DAGVNode *next) {
	BasicBlock *bb = n->getBlock();
	for (BasicBlock::EdgeIter it(bb->outs()); it; it++) {
		Edge *edge = *it;
		Block *target = edge->target();
		if (target->isBasic()) {
			BasicBlock *b2 = *target->toBasic();
			if (b2 == l_h) {
				n->addSucc(next);
				next->addPred(n);
				dag->setNext(n);
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
	BasicBlock *bb = n->getBlock();
	for (BasicBlock::EdgeIter it(bb->outs()); it; it++) {
		Edge *edge = *it;
		if (LOOP_EXIT_EDGE(edge) != nullptr) { // si il y a un edge qui sort de la loop
			n->addSucc(exit);
			exit->addPred(n);
			dag->addEnd(n);
		}
		else if (l_h == NULL){
			Block *target = edge->target();
			 if (target->isExit()){
				 n->addSucc(exit);
				 exit->addPred(n);
				 dag->addEnd(n);
			 }
		}
	}
}

/*
Noeud entrant des LH
*/
void addLHEdgesEntry(DAG *dag, DAGNode *n, DAGHNode *l_hp) {
	BasicBlock *bb = n->toBNode() ? n->toBNode()->getBlock() : n->toHNode()->getHeader();
	BasicBlock *bb_l_hp = l_hp->getHeader();

	// Pour tous les edges du bloc
	for (BasicBlock::EdgeIter it(bb->outs()); it; it++) {
		Edge *edge = *it;
		Block *target = edge->target();
		if (target->isBasic()){
			BasicBlock *b2 = *target->toBasic();
			if (b2 == bb_l_hp) {
				n->addSucc(l_hp);
				l_hp->addPred(n);
			}
		}
	}
}

/*
Noeud sortant des LH
*/
void addLHEdgesExit(DAG *dag, DAGNode *n, DAGHNode *l_hp) {
	BasicBlock *bb = n->toBNode() ? n->toBNode()->getBlock() : n->toHNode()->getHeader();
	//BasicBlock *bb_l_hp = l_hp->getHeader();
	DAG *sub_dag = l_hp->getDag(); // sous_dag du noeud header
	for (BasicBlock::EdgeIter it(bb->ins()); it; it++) { // On regarde tous les edges de ce noeud
		Edge *edge = *it;
		Block *target = edge->source();
		if (target->isBasic()){
			BasicBlock *b2 = *target->toBasic();
			//if (b2 == bb_l_hp) {
			if (sub_dag->find_node(DAG_BNODE(b2))) {
				n->addPred(l_hp);
				l_hp->addSucc(n);
			}
		}
	}
}

DAG* CFTreeExtractor::toDAG(CFG *cfg, BasicBlock *l_h){
	int i;
	int j;
	DAG *dag = new DAG();

	std::vector<DAGBNode*> blocks; //Bd moins les headers de boucle
	std::vector<DAGHNode*> lh_blocks; //lh' tous les headers de boucle
	//std::vector<DAG*> sub_cfg;


	DAGVNode *next = new DAGVNode(VNODE_NEXT);
	DAGVNode *exit = new DAGVNode(VNODE_EXIT);

	dag->addNode(next);
	dag->addNode(exit);

	otawa::cftree::getAllBlocksLoop(dag, &blocks, &lh_blocks, cfg, l_h);

	for (i = 0; i < lh_blocks.size(); i++){ // construit les dags des sous boucles directs
		//sub_cfg.push_back(toDAG(cfg, lh_blocks[i]->getHeader()));
		lh_blocks[i]->setDAG(toDAG(cfg, lh_blocks[i]->getHeader()));
	}

	for (i = 0; i < blocks.size(); i++){ // ajoute les noeuds Next pour chaque bloc
		addEdgesNext(dag, blocks[i], l_h, next);
		addEdgesExit(dag, blocks[i], l_h, exit);
	}

	for (i = 0; i < lh_blocks.size(); i++){
		for (j = 0; j < blocks.size(); j++) {
			addLHEdgesEntry(dag, blocks[j], lh_blocks[i]);
			addLHEdgesExit(dag, blocks[j], lh_blocks[i]);
		}


		for (j = 0; j < lh_blocks.size(); j++){
			if (i != j) {
				addLHEdgesEntry(dag, lh_blocks[j], lh_blocks[i]);
				addLHEdgesExit(dag, lh_blocks[j], lh_blocks[i]);
			}
		}
	}

	if (l_h != NULL){ // noeud d'entrée du dag (donc l_h ou null)
		DAGBNode *n = DAG_BNODE(l_h);
		if (n == NULL){
			cout << "probleme" << endl;
		}
		dag->setStar(n);
	} else {
		setEntryDAG(cfg, dag);
		setExitDAG(cfg, dag);
	}

	return dag;
}

/*
Vérifie qu'un noeud est dominant à un autre
*/
int DAGNodeDominate(DAGNode *b1, DAGNode *b2){

	BasicBlock *bb1 = b1->toBNode() ? b1->toBNode()->getBlock() : b1->toHNode()->getHeader();
	BasicBlock *bb2 = b2->toBNode() ? b2->toBNode()->getBlock() : b2->toHNode()->getHeader();
	if (Dominance::dominates(bb1, bb2)){
		return 1;
	}
	return 0;
}

int isNotVisited(DAGNode *x, std::vector<DAGNode*> *path){
    int size = path->size();
    for (int i = 0; i < size; i++)
        // if (path[i] == x) // comparer avec les blocs
        //     return 0;
    return 42;
}

/*
Retourne les blocks qui sont présents dans tous les chemins entre start et end
*/
void forcedPassedNodes(DAG *dag, DAGNode *start, DAGNode *end, std::vector<DAGNode*> *fpn){
	// std::vector<DAGNode*> *fpn;

	// pour tous les noeuds du dag
	for (auto it = dag->iter(); it != dag->end(); it++) {
		DAGNode *n = (*it);
		if (n->toVNode()){
			continue;
		}
		else {
			if (start != n){
				if ( (DAGNodeDominate(start, n)) && DAGNodeDominate(n, end))
					fpn->push_back(n);
			}
		}
	}
}

/*
Retourne le noeud qui est dominé par tous les autres
*/
DAGNode* getDominated(std::vector<DAGNode*> fpn){
	auto first = fpn.begin();
	unsigned int ind = 0;
	while (fpn.size() > 1){ // On va supprimer tous les blocs qui dominent un autre bloc
		unsigned i = 0;
		while (i < fpn.size()){
			if (*first != fpn[i]){
				if (DAGNodeDominate(fpn[i], *first)){ // Si fpn[i] domine first
					fpn.erase(fpn.begin()+i); // On peut le supprimer
				}
				else if (DAGNodeDominate(*first, fpn[i])){ // Si first domine fpn
					fpn.erase(first); // on supprime first
					first = fpn.begin(); // nouveau first
					i = 0; // on recommence
				}
			}
			i++;
		}
		first++;
	}
	return *first;
}

CFTree* toCFT(DAG *dag, DAGNode *start, DAGNode *end, int all){
	std::vector<CFTree*> ch;
	std::vector<DAGNode*> fpn;


	forcedPassedNodes(dag, start, end, &fpn);

	if (start == end){
		if (start->toBNode()){
			BasicBlock *bb =  start->toBNode()->getBlock();
			return (new CFTreeLeaf(bb));
		}
	}

	while (fpn.size() > 0){
		DAGNode *c = getDominated(fpn); // l'element dominé
		std::vector<DAGNode*>::iterator it = std::find(fpn.begin(), fpn.end(), c); // retrouver l'index de l'élement dominé
		fpn.erase(it); // N <- N \ c
		// Si c a plusieurs prédécesseurs, càd qu'il y a un if avant
		if (c->getPred().size() > 1){
			DAGNode *ncd;
			if (fpn.size() > 0)
				ncd = getDominated(fpn);
			else
				ncd = start;

			// On ajoute à la suite le bloc dominé
			if (c->toBNode()){
				BasicBlock *bb =  c->toBNode()->getBlock();
				ch.insert(ch.begin(), new CFTreeLeaf(bb));
			}

			CFTreeAlt *br = new CFTreeAlt();

			// On construit le CFTreeAlt
			for (auto pred = c->predIter(); pred != c->predEnd(); pred++){

				br->addAlt(toCFT (dag, ncd, *pred, 0));
			}
			ch.insert(ch.begin(), br);


		}


		// Si c n'a qu'un seul prédécesseur
		else {
			if (c->toBNode()){
				BasicBlock *bb =  c->toBNode()->getBlock();
				ch.insert(ch.begin(),new CFTreeLeaf(bb));
			}

			// Si on a une loop interne, on construit le CFTree associé
			else if ( c->toHNode()){
				DAGHNode *lh = c->toHNode();
				DAG * sub_dag = lh->getDag();
				CFTree *bd = toCFT(lh->getDag(), sub_dag->getStart(), sub_dag->getNext(), 1);
				std::vector <CFTree *> ex;

				for (auto it = sub_dag->iter_e(); it != sub_dag->end_e(); it++) {
					CFTree *ex_suc = toCFT(lh->getDag(), sub_dag->getStart(), *it, 1);
					ex.push_back(ex_suc);
				}

				if ((sub_dag->getStart()) -> toBNode()){
					BasicBlock *bb = ((sub_dag->getStart())->toBNode())->getBlock();
					ch.insert(ch.begin(), new CFTreeLoop(bb, 0, bd, ex));
				}
			}
		}
	}

	ASSERT(!all || (start-> getPred().size() == 0));
	if (all) {
		BasicBlock *bb =  start->toBNode()->getBlock();
		ch.insert(ch.begin(), new CFTreeLeaf(bb));
	}

	CFTreeSeq* t_ch = new CFTreeSeq(ch);
	return t_ch;
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

CFTree* CFTreeExtractor::processCFG(CFG *cfg) {
	DAG *dag = toDAG(cfg, NULL);
	CFTree *tree = toCFT(dag, dag->getStart(), dag->getiEnd(0), 1);

	CFTREE(cfg) = tree;

	return tree;
}

CFTCollection::CFTCollection(std::vector<CFTree*> v){
	cfts = v;
}

void CFTreeExtractor::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	for (CFGCollection::Iter iter(*coll); iter; iter ++) {
		CFG *currentCFG = *iter;
		processCFG(currentCFG);
	}

}

Identifier<DAGHNode*> DAG_HNODE("otawa::cftree:DAG_HNODE");
Identifier<DAGBNode*> DAG_BNODE("otawa::cftree:DAG_BNODE");
Identifier<CFTree*> CFTREE("otawa::cftree:CFTREE");

} }
