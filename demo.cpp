#include <stdio.h>

#include <otawa/app/Application.h>
#include <otawa/cfg/features.h>
#include <otawa/script/Script.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/DynProcessor.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/proc/DynFeature.h>

//includes pour l'affichage du CFG
#include <otawa/display/CFGDrawer.h>
#include <otawa/display/CFGOutput.h>

#include "include/CFTree.h"

using namespace otawa; //comme import
using namespace otawa::cftree;


int main(void) {
	WorkSpace *ws = NULL;
	PropList conf;
	Manager manager;
	NO_SYSTEM(conf) = true;
	TASK_ENTRY(conf) = "_start";
	VERBOSE(conf) = true;

	ws = manager.load("./binaire", conf);
	ws->require(DynFeature("otawa::cftree::EXTRACTED_CFT_FEATURE"), conf);
 	display::CFGOutput output; // CFGOutput est un processor
 	output.process(ws, conf);

	const CFGCollection *coll = INVOLVED_CFGS(ws);
	for (CFGCollection::Iter iter(*coll); iter; iter ++) {
		CFG *currentCFG = *iter;
		StringBuffer buf;
		buf << currentCFG->label() << "-cftree.dot";
		CFTREE(currentCFG)->exportToDot(buf.toString());
	}


}

//
//
// int main(void) {
// 	/* otawa:: */ WorkSpace *ws = NULL;
// 	PropList conf;
// 	Manager manager;
//
// 	// set certaines options dans PropList
// 	NO_SYSTEM(conf) = true;
// 	TASK_ENTRY(conf) = "_start";
// 	VERBOSE(conf) = true;
//
//
// 	ws = manager.load("./binaire", conf);
// 	// Processor: quelquechose qui va utiliser/modifier le workspace
//
// 	display::CFGOutput output; // CFGOutput est un processor
// 	output.process(ws, conf);
//
// 	ws->require(COLLECTED_CFG_FEATURE, conf);
// 	ws->require(DOMINANCE_FEATURE, conf);
//
// 	const CFGCollection *coll = INVOLVED_CFGS(ws);
//
// 	for (CFGCollection::Iter iter(*coll); iter /* iter.hasNext() */ ; iter++ /* iter.next() */ ) {
// 		CFG *currentCFG = *iter /* iter.get() */ ;
// 		cout << "Je traite le CFG de la fonction: " << currentCFG->name() << endl;
//
// 		for (CFG::BlockIter iter2(currentCFG->blocks()); iter2; iter2++) {
// 			Block *bb = *iter2;
// 			cout << "  Je traite le bloc de base: " << bb << endl;
//
// 			if (bb->isBasic()) {
// 				BasicBlock *b = bb->toBasic();
// 				for (BasicBlock::EdgeIter iter3(b->outs()); iter3; iter3++) {
// 					Edge *edge = *iter3;
//
// 					Block *source = edge->source(); //source
// 					Block *sink = edge->sink(); // destination
// //					cout << "    Il y a un edge du block " << bb->id() << " au bloc " << sink->id() << endl;
//
// 					if (Dominance::dominates(sink, source)) {
// 						cout << "On a trouve une boucle, dont le header est: " << sink->id() << endl;
// 					}
//
// 				}
//
// 			}
//
// 		}
// 	}
//
//
// 	ASSERT(coll != NULL);
//
// }
