#include "include/CFTree.h"

namespace otawa { namespace cftree {
using namespace otawa;


/*
	Un CFTree Node
	-> Leaf(b) : représente un basic bloc
	-> Alt(t1..tn) : représente alternative sur plusieurs sous-arbre
	-> Loop(h,t1,n,t2) : Loop avec h en header, répète n fois le sous-arbre t1, et execute l'arbre t2
	-> Seq(t1..tn) : Séquence de tree
*/
class CFTree {

public:
	virtual CFTreeLeaf *toLeaf() = 0; /* abstract */
	virtual CFTreeAlt *toAlt() = 0;/* abstract */
	virtual CFTreeLoop *toLoop() = 0;/* abstract */
	virtual CFTreeSeq *toSeq() = 0;/* abstract */

};

class CFTreeLeaf : public CFTree{

	BasicBlock *block;

	public:

	virtual CFTreeLeaf *toLeaf() {
		return (CFTreeLeaf*) this;
	}
	virtual CFTreeAlt *toAlt() {return NULL;}/* abstract */
	virtual CFTreeLoop *toLoop() {return NULL;}/* abstract */
	virtual CFTreeSeq *toSeq() {return NULL;}/* abstract */

	CFTreeLeaf(BasicBlock *b) {
		block = b;
	}
	BasicBlock *getBlock() { return block; }
	int getBlockId() const { return block->id(); }

};


class CFTreeAlt : public CFTree{

	std::vector<CFTree*> alts;

	public:

	virtual CFTreeAlt *toAlt() {
		return (CFTreeAlt*) this;
	}
	virtual CFTreeLeaf *toLeaf() {return NULL;}/* abstract */
	virtual CFTreeLoop *toLoop() {return NULL;}/* abstract */
	virtual CFTreeSeq *toSeq() {return NULL;}/* abstract */

	void addAlt(CFTree *s) {
		alts.push_back(s);
	}

	std::vector<CFTree *>::const_iterator childIter() {
		return alts.begin();
	}

	std::vector<CFTree *>::const_iterator childEnd() {
		return alts.end();
	}

	CFTree* getI(int ind) {
		assert(ind < alts.size());
		return alts[ind];
	}

};

class CFTreeSeq : public CFTree{

	std::vector<CFTree*> childs;

	public:

	virtual CFTreeSeq *toSeq() {
		return (CFTreeSeq*) this;
	}
	virtual CFTreeLeaf *toLeaf() {return NULL;}/* abstract */
	virtual CFTreeLoop *toLoop() {return NULL;}/* abstract */
	virtual CFTreeAlt *toAlt() {return NULL;}/* abstract */

	void addChild(CFTree *s) {
		childs.push_back(s);
	}

	std::vector<CFTree *>::const_iterator childIter() {
		return childs.begin();
	}

	std::vector<CFTree *>::const_iterator childEnd() {
		return childs.end();
	}

	CFTree* getI(int ind) {
		assert(ind < childs.size());
		return childs[ind];
	}

};

class CFTreeLoop : public CFTree{

	BasicBlock *header;
	CFTree* t1; // body
	int n;
	CFTree* t2; // exit

	public:

	virtual CFTreeLoop *toLoop() {
		return (CFTreeLoop*) this;
	}
	virtual CFTreeLeaf *toLeaf() {return NULL;}/* abstract */
	virtual CFTreeSeq *toSeq() {return NULL;}/* abstract */
	virtual CFTreeAlt *toAlt() {return NULL;}/* abstract */

	CFTreeLoop(BasicBlock *h, int bound) {
		header = h;
		n = bound;
	}

	BasicBlock *getHeader() { return header; }
	int getHeaderId() const { return header->id(); }

	void addT1(CFTree *t){
		t1 = t;
	}

	void addT2(CFTree *t){
		t2 = t;
	}

	void changeBound(int bound){
		n = bound;
	}

};


template <class T>
class CFTSemantic {
	virtual T evalAlt(std::vector<T*> *list) = 0;
	virtual T evalSeq(std::vector<T*> *list) = 0;
	virtual T evalLoop(T *body, T *exit, BasicBlock *header, int n) = 0;
	virtual T evalLeaf(BasicBlock *bb) = 0;
};

template <class T>
class EvaluateCFT {
  public:
	static T evaluate(CFTree *cft, CFTSemantic<T> *sem) {
	}
};


class NodeCount: public CFTSemantic<int> {
	virtual int evalAlt(std::vector<int*> *list) {
		int sum = 0;
		for (int i = 0; i < list->size(); i++) {
			sum += *(list->at(i));
		}
	}
	virtual int evalSeq(std::vector<int*> *list) {
		int sum = 0;
		for (int i = 0; i < list->size(); i++) {
			sum += *(list->at(i));
		}
	}
	virtual int evalLoop(int *body, int *exit, BasicBlock *header, int n) {
		return *body + *exit;
	}
	virtual int evalLeaf(BasicBlock *bb) {
		return 1;
	}
};

int countNodes(CFTree *cft) {
	NodeCount nc;
	return EvaluateCFT<int>::evaluate(cft, &nc);
}


class DAG {
	std::vector<DAGNode*> all; // ensemble des noeuds du dag
	DAGNode *n_start; // le point d'entrée du dag
	std::vector<DAGNode*> n_ends; // les sorties du dag

  public:
	std::vector<DAGNode*>::const_iterator iter() {
		return all.begin();
	}

	std::vector<DAGNode*>::const_iterator end() {
		return all.end();
	}
	void addNode(DAGNode *s) {
		all.push_back(s);
	}

	void setStar(DAGNode *s) {
		n_start = s;
	}

	void addEnd(DAGNode *s) {
		n_ends.push_back(s);
	}

	DAGNode *getStart(){
		return n_start;
	}

	DAGNode *getElement(unsigned i){
		return all[i];
	}

	std::vector<DAGNode*>::const_iterator iter_e(){
		return n_ends.begin();
	}

	std::vector<DAGNode*>::const_iterator end_e(){
		return n_ends.end();
	}

	DAGNode *getiEnd(int i){
		assert(i < n_ends.size());
		return n_ends[i];
	}

	bool find_node(DAGNode *elem){ // cherche si un bloc est présent dans le dag
		std::vector<DAGNode*>::const_iterator it;
		it = find (iter(), end(), elem);
		return (it != end());
	}

  ~DAG();
};


class DAGNode {

	std::vector<DAGNode*> pred; // les noeuds précédents du sommet
	std::vector<DAGNode*> succ; // les noeuds suivants du sommet

	DAGNode* dom_start;
	DAGNode* dom_end;
	std::vector<DAGNode*> dominators; // les noeuds qui dominent ces noeuds (change selon le start/ end donné en argument au moment de la construction)

  public:
	virtual DAGVNode *toVNode() = 0; /* abstract */
	virtual DAGHNode *toHNode() = 0;/* abstract */
	virtual DAGBNode *toBNode() = 0;/* abstract */

	void addSucc(DAGNode *s) {
		succ.push_back(s);
	}

	void addPred(DAGNode *s) {
		pred.push_back(s);
	}

	void addDomin(DAGNode *s) {
		dominators.push_back(s);
	}

	std::vector<DAGNode*>::const_iterator succIter() {
		return succ.begin();
	}

	std::vector<DAGNode*>::const_iterator predIter() {
		return pred.begin();
	}

	std::vector<DAGNode*>::const_iterator succEnd() {
		return succ.end();
	}

	std::vector<DAGNode*>::const_iterator predEnd() {
		return pred.end();
	}

	std::vector<DAGNode*>::const_iterator dominIter() {
		return dominators.begin();
	}

	std::vector<DAGNode*>::const_iterator dominEnd() {
		return dominators.end();
	}

	std::vector<DAGNode*> getSucc(){
		return succ;
	}

	std::vector<DAGNode*> getPred(){
		return pred;
	}
};
	// DAGNode *end;


class DAGVNode : public DAGNode {
#define VNODE_EXIT 0
#define VNODE_NEXT 1
	int type;

  public:
	virtual DAGVNode *toVNode() {
		return (DAGVNode*) this;
	}
	virtual DAGHNode *toHNode() { return NULL; }
	virtual DAGBNode *toBNode() { return NULL; }

	DAGVNode(int _type) { type = _type; }
	int getType() const { return type;}

};

class DAGHNode : public DAGNode {
	BasicBlock *header;
	DAG* sub_dag;
  public:
	virtual DAGHNode *toHNode() {
		return (DAGHNode*) this;
	}
	virtual DAGVNode *toVNode() { return NULL; }
	virtual DAGBNode *toBNode() { return NULL; }
	DAGHNode(BasicBlock *b) {
		header = b;
		DAG_HNODE(header) = this;
	}
	void setDAG(DAG *dag){
		sub_dag = dag;
	}
	BasicBlock *getHeader() { return header; }
	DAG *getDag() {return sub_dag;}
	int getLoopId() const { return header->id(); }
};

class DAGBNode : public DAGNode {
	BasicBlock *block;
  public:
	virtual DAGBNode *toBNode() {
		return (DAGBNode*) this;
	}
	virtual DAGHNode *toHNode() { return NULL; }
	virtual DAGVNode *toVNode() { return NULL; }
	DAGBNode(BasicBlock *b) {
		block = b;
		DAG_BNODE(block) = this;
	}
	BasicBlock *getBlock() { return block; }
	int getBlockId() const { return block->id(); }
};

DAG::~DAG()
{
  for (/* std::vector<DAGNode*>::iterator */ auto it = iter(); it != end(); it++) {
	  DAGNode *n = (*it);
	  delete n;
  }
}


/*
 * DAGNode *v = new DAGVNode();

  DAGVnode *v2 = v->toVNode();
 */






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
			cout << "add exit block " << *n << endl;
		}
		else if (l_h == NULL){
			Block *target = edge->target();
			 if (target->isExit()){
				 n->addSucc(exit);
				 exit->addPred(n);
				 dag->addEnd(n);
				 cout << "add exit block " << *n << endl;
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
	cout << "Ajout des noeuds de sortie" << endl;
	cout << "On compare l'header " << *l_hp << " aveAGHNodec " << *n << endl;
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
				cout << "OUI" << endl;
				n->addPred(l_hp);
				l_hp->addSucc(n);
			}
		}
	}
}

DAG *toDAG(CFG *cfg, BasicBlock *l_h){
	int i;
	int j;
	DAG *dag = new DAG();

	cout << "Je DAGifie le cfg " << cfg->name();
	if (l_h == NULL)  {
		cout << " hors de toute boucle" << endl;
	} else {
		cout << " a partir de la boucle avec header: " << l_h->id() << endl;
	}

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
	}

	for (i = 0; i < blocks.size(); i++){ // ajoute les noeuds Exit pour chaque bloc
		addEdgesExit(dag, blocks[i], l_h, exit);
	}

	for (i = 0; i < lh_blocks.size(); i++){
		for (j = 0; j < blocks.size(); j++)
			addLHEdgesEntry(dag, blocks[j], lh_blocks[i]);

		for (j = 0; j < lh_blocks.size(); j++){
			if (i != j)
				addLHEdgesEntry(dag, lh_blocks[j], lh_blocks[i]);
		}

		for (j = 0; j < blocks.size(); j++)
			addLHEdgesExit(dag, blocks[j], lh_blocks[i]);

		for (j = 0; j < lh_blocks.size(); j++){
			if (i != j)
				addLHEdgesExit(dag, lh_blocks[j], lh_blocks[i]);
		}
	}

	if (l_h != NULL){ // noeud d'entrée du dag (donc l_h ou null)
		cout << "J'ajoute start " << endl;
		DAGBNode *n = DAG_BNODE(l_h);
		if (n == NULL){
			cout << "probleme" << endl;
		}
		dag->setStar(n);
		// dag->addNode(n);
	} else {

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
		return 42;
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
/!\ à vérifier en changeant les starts et ends
*/
void forcedPassedNodes(DAG *dag, DAGNode *start, DAGNode *end, std::vector<DAGNode*> *fpn){
	cout << "Je rentre dans la fonction pour avoir les noeuds de passages forcés" << endl;
	// std::vector<DAGNode*> *fpn;

	// pour tous les noeuds du dag
	for (auto it = dag->iter(); it != dag->end(); it++) {
		DAGNode *n = (*it);
		if (n->toVNode()){

		}
		else {
			cout << "Je vais regarder pour le noeud " << *n << endl;
			if ( (DAGNodeDominate(start, n)) && DAGNodeDominate(n, end))
				fpn->push_back(n);
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
			if (DAGNodeDominate(fpn[i], *first)){ // Si fpn[i] domine first
				fpn.erase(fpn.begin()+i); // On peut le supprimer
			}
			else if (DAGNodeDominate(*first, fpn[i])){ // Si first domine fpn
				fpn.erase(first); // on supprime first
				first = fpn.begin(); // nouveau first
				i = 0; // on recommence
			}
			i++;
		}
		first++;
	}
	return *first;
}

// DAGNode* getImmDominator(std::vector<DAGNode*> fpn, DAGNode *c){
// 	auto imm_dominator = fpn[0];
// 	while (fpn.size() > 1){
// 		unsigned i = 0;
// 		while (i < fpn.size()){
// 			if (DAGNodeDominate(fpn[i], *imm_dominator)){ // Si fpn[i] domine first
// 				fpn.erase(fpn.begin()+i); // On peut le supprimer
// 			}
// 			else if (DAGNodeDominate(*imm_dominator, fpn[i])){ // Si first domine fpn
// 				fpn.erase(imm_dominator); // on supprime first
// 				imm_dominator = fpn.begin(); // nouveau first
// 				i = 0; // on recommence
// 			}
// 			i++;
// 		}
// 		imm_dominator++;
// 	}
// 	return *imm_dominator;
// }

CFTree* makeCFT(DAG *dag, DAGNode *start, DAGNode *end){
	std::vector<CFTree*> ch;
	std::vector<DAGNode*> fpn;

	cout << "Noeud de départ " << *start << endl;
	cout << "Noeud de fin " << *end << endl;
	forcedPassedNodes(dag, start, end, &fpn);
	cout << "Nombre de noeuds de passage forcé " << fpn.size() << endl;

	// DEBUG VÉRIFIER LES NOEUDS DE PASSAGE FORCÉ
	for (auto it = fpn.begin(); it != fpn.end(); it++){
		DAGNode *n = (*it);
		cout << "Noeud de passage forcé " << *n << endl;
	}
	while (fpn.size() > 0){
		DAGNode *c = getDominated(fpn); // l'element dominé
		std::vector<DAGNode*>::iterator it = std::find(fpn.begin(), fpn.end(), c); // retrouver l'index de l'élement dominé
		cout << "Noeud dominé " << *c << endl; // c tel que n >> c
		cout << "Taille de fpn " << fpn.size() << endl;
		fpn.erase(it); // N <- N \ c
		if (c->getPred().size() > 1){
			DAGNode *ncd = getDominated(fpn);
			CFTreeAlt *br;
			for (auto pred = c->predIter(); pred != c->predEnd(); pred++){
				br->addAlt(makeCFT (dag, ncd, *pred));
			}
			ch.push_back(br);
		}
		ch.push_back(new CFTreeLeaf(c->getBlock()));
	}
	return ch;
}

void CFTreeExtractor::processCFG(CFG *cfg) {
	cout << "Je traite le CFG de la fonction: " << cfg->name() << endl;

	cout << "Traiter la partie hors de toute boucle: " << endl;
	DAG *dag = toDAG(cfg, NULL);
	cout << "DAG: " << *dag;
	makeCFT(dag, dag->getElement(2), dag->getElement(4));

	for (auto it = dag->iter(); it != dag->end(); it++) {
		DAGNode *n = (*it);
		if (n->toHNode()){
			DAGHNode *hn = n->toHNode();
			DAG *sub_dag = hn->getDag();
			// cout << "DAG: " << *sub_dag;
			// makeCFT(sub_dag, sub_dag->getStart(), sub_dag->getiEnd(0));
		}

	}
	//makeCFT(dag, dag->getStart(), dag->getiEnd(0));
	cout << "-------------------------------------------------------" << endl;

	// for (CFG::BlockIter iter(cfg->blocks()); iter; iter++) {
	// 	Block *bb = *iter;
	// 	if (LOOP_HEADER(bb)) {
	// 		// cout << "Je traite la boucle: " << bb->id() << endl;
	// 		DAG *dag = toDAG(cfg, bb->toBasic());
	// 		cout << "DAG: " << *dag;
	// 		makeCFT(dag, dag->getStart(), dag->getiEnd(0));
	// 		cout << "-------------------------------------------------------" << endl;
	// 		// cout << "dsq " << *(dag->getiEnd(0)) << endl;
	//
	// 	}
	// }

}

void CFTreeExtractor::processWorkSpace(WorkSpace *ws) {
	cout << "Debut du plugin." << endl;
	const CFGCollection *coll = INVOLVED_CFGS(ws); // l'ensemble des CFG du programme
	// ws->require(DOMINANCE_FEATURE, conf);

	for (CFGCollection::Iter iter(*coll); iter; iter ++) {
		CFG *currentCFG = *iter;
		processCFG(currentCFG);

	}
}


} }
