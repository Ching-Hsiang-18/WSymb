#include <stdio.h>

#include <otawa/app/Application.h>
#include <otawa/cfg/features.h>
#include <otawa/script/Script.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/DynProcessor.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/proc/DynFeature.h>
#include <otawa/cfg/Virtualizer.h>

//includes pour l'affichage du CFG
#include <otawa/display/CFGOutput.h>
#include <elm/io/OutFileStream.h>
#include <otawa/ipet/features.h>
#include <otawa/flowfact/features.h>

#include "include/CFTree.h"

using namespace otawa; //comme import
using namespace otawa::cftree;




void fix_virtualized_loopinfo(CFG *entryCFG, Block *loop = nullptr) {
	for (CFG::BlockIter iter(entryCFG->blocks()); iter(); iter++) {
		if (ENCLOSING_LOOP_HEADER(*iter) == nullptr)
			ENCLOSING_LOOP_HEADER(*iter) = loop;

		if ((*iter)->isSynth()) {
			CFG *callee = (*iter)->toSynth()->callee();
			fix_virtualized_loopinfo(callee, ENCLOSING_LOOP_HEADER(*iter));
		}
	}
}

bool is_strictly_in(Block *inner, Block *outer) {
	ASSERT(LOOP_HEADER(inner));
	ASSERT(LOOP_HEADER(outer));

	Block *current = inner;
	while (current != nullptr) {
		current = ENCLOSING_LOOP_HEADER(current);
		if (current == outer) return true;
	}
	return false;
}

int main(int argc, char **argv) {
	if ((argc < 3) || (argc > 4)) {
		fprintf(stderr, "usage: %s <ARM binary> <output header file> [<entry fct (by default main)>]\n", argv[0]);
		exit(1);
	}
	WorkSpace *ws = NULL;
	PropList conf;
	Manager manager;
	NO_SYSTEM(conf) = true;
	TASK_ENTRY(conf) = "main";
	VERBOSE(conf) = true;

	ws = manager.load(argv[1], conf);


	ws->require(otawa::VIRTUALIZED_CFG_FEATURE, conf);

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
	const char *entryname = (argc >= 4) ? argv[3] : "main";
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
	cout << "Computing WCET using IPET..." << endl;
	ws->require(ipet::WCET_FEATURE, conf);
	cout << "WCET value using IPET: " << ipet::WCET(ws) << endl;

	cout << "Exporting to AWCET...";
	formula_t f;
	memset(&f, 0, sizeof(f));
	CFTREE(entry)->exportToAWCET(&f);
	cout << "done." << endl;
	
	cout << "Computing WCET using CFTree..." << endl;
	loopinfo_t li;
	li.bnd = [](int x) {
		return 10;
	};

	int wcet = evaluate(&f, &li, nullptr, nullptr);
	cout << "WCET value using CFTree: " << wcet << endl;


	FILE *outfile = fopen(argv[2], "w");

	fprintf(outfile, "int loop_bounds(int loop_id) {\n  switch(loop_id) {\n");
	for (CFGCollection::Iter iter(*coll); iter(); iter ++) {
		cout << "managing loop bounds for "<< (*iter)->label() << endl;
		for (CFG::BlockIter iter2((*iter)->blocks()); iter2(); iter2++) {
			if (LOOP_HEADER(*iter2)) {
				int bound = MAX_ITERATION(*iter2);
				fprintf(outfile, "    case %d: return %d;\n", (*iter2)->id(), bound);
			}
		}
	}
	fprintf(outfile, "    default:    abort();\n");
	fprintf(outfile, "  }\n}\n");


	fix_virtualized_loopinfo(entry);

	fprintf(outfile, "int loop_hierarchy(int inner, int outer) {\n  switch(inner) {\n");
	for (CFGCollection::Iter iter(*coll); iter(); iter ++) {
		for (CFG::BlockIter iter2((*iter)->blocks()); iter2(); iter2++) {
			if (LOOP_HEADER(*iter2)) {
				fprintf(outfile, "    case %d: switch (outer) {\n", (*iter2)->id());
				
				for (CFGCollection::Iter iter3(*coll); iter3(); iter3 ++) {
					for (CFG::BlockIter iter4((*iter3)->blocks()); iter4(); iter4++) {
						if (LOOP_HEADER(*iter4)) {
							if (is_strictly_in(*iter2, *iter4)) {
								fprintf(outfile, "      case %d: return 1;\n", (*iter4)->id());    
							}
						}
					}
				}
				fprintf(outfile, "      default: return 0;\n    }\n");
			}
		}
	}
	fprintf(outfile, "    default:    abort();\n");
	fprintf(outfile, "  }\n}\n");

	fprintf(outfile, "formula_t f = ");
	compute_eta_count(&f);
	writeC(&f, outfile, 0);	
	fprintf(outfile, ";\n");
	fclose(outfile);
	
}
