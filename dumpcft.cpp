#include <stdio.h>

#include <otawa/app/Application.h>
#include <otawa/cfg/features.h>
#include <otawa/script/Script.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/DynProcessor.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/proc/DynFeature.h>

//includes pour l'affichage du CFG
#include <otawa/display/CFGOutput.h>
#include <elm/io/OutFileStream.h>
#include <otawa/ipet/features.h>

#include "include/CFTree.h"

using namespace otawa; //comme import
using namespace otawa::cftree;
using namespace otawa::pwcet;


int main(int argc, char **argv) {
	if ((argc < 2) || (argc > 3)) {
		fprintf(stderr, "usage: %s <ARM binary> [<entry fct (by default main)>]\n", argv[0]);
		exit(1);
	}
	WorkSpace *ws = NULL;
	PropList conf;
	Manager manager;
	NO_SYSTEM(conf) = true;
	TASK_ENTRY(conf) = "main";
	VERBOSE(conf) = true;

	ws = manager.load(argv[1], conf);


	ws->require(DynFeature("otawa::cftree::EXTRACTED_CFT_FEATURE"), conf);
 
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	StringBuffer buf;
	buf << argv[1] << "-decomp.c";
	elm::io::OutFileStream s(buf.toString());
	elm::io::Output out(s);
	
	for (CFGCollection::Iter iter(*coll); iter(); iter ++) {
		CFG *currentCFG = *iter;
		StringBuffer buf;
		buf << argv[1] << "-" << currentCFG->label() << "-cftree.dot";
		if (CFTREE(currentCFG)) {
			CFTREE(currentCFG)->exportToDot(buf.toString());
			out << currentCFG->label() << "() {\n";
			CFTREE(currentCFG)->exportToC(out);
			out << "}\n\n";
		}
	}
	
	// produce awcet
	CFG *entry = nullptr;
	const char *entryname = (argc >= 3) ? argv[2] : "main";
	elm::String entryname_s(entryname);
	for (CFGCollection::Iter iter(*coll); iter(); iter ++) {
		if (entryname_s == (*iter)->name()) {
			entry = (*iter);
			break;
		}
	}
	if (entry == nullptr) {
		cerr << "entry point " << entryname_s << " not found" << endl;
		exit(1);
	}
	cout << "Exporting to AWCET...";
	formula_t f;
	CFTREE(entry)->exportToAWCET(&f);
	cout << "done." << endl;
	
	
	
}
