#include "include/CFTree.h"
#include <otawa/proc/ProcessorPlugin.h>

namespace otawa { namespace cftree {
using namespace otawa;


class CFTree {
};


class Plugin : public ProcessorPlugin {
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
        Processor::configure(props);
}

void CFTreeExtractor::processCFG(CFG *cfg) {
	cout << "Je traite le CFG de la fonction: " << cfg->name() << endl;
	for (CFG::BlockIter iter(cfg->blocks()); iter; iter++) {
		Block *bb = *iter;
//		cout << "  Je traite le bloc de base: " << bb << endl;
		if (LOOP_HEADER(bb)) {
			cout << "Le block " << bb << "est un loop header" << endl;
		}
		Block *bb2 = ENCLOSING_LOOP_HEADER(bb);
		if (bb2 == nullptr) {
/*
			if (LOOP_HEADER(bb)) {
				cout << "Ce loop header n'est pas contenu dans une boucle externe." << endl;
			} else cout << "Ce block n'est pas dans une boucle." << endl;
*/
		} else {
			cout << "Le block " << bb  << "est contenu dans la boucle: " << bb2 << endl;
		}
		if (bb->isBasic()) {
			BasicBlock *b = bb->toBasic();
			for (BasicBlock::EdgeIter iter3(b->outs()); iter3; iter3++) {
				Edge *edge = *iter3;
				if (LOOP_EXIT_EDGE(edge) != nullptr) {
					Block *bb3 = LOOP_EXIT_EDGE(edge);
					cout << "L'edge " << edge << " sort de la boucle " << bb3 << endl;
				}
			}
		}
		
	}
}

void CFTreeExtractor::processWorkSpace(WorkSpace *ws) {
	cout << "Debut du plugin." << endl;
	const CFGCollection *coll = INVOLVED_CFGS(ws);

	for (CFGCollection::Iter iter(*coll); iter; iter ++) {
		CFG *currentCFG = *iter;
		processCFG(currentCFG);

	}
}

} }
