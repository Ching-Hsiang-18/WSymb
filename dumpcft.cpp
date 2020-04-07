/* ----------------------------------------------------------------------------
   Copyright (C) 2020, Université de Lille, Lille, FRANCE

   This file is part of CFTExtractor.

   CFTExtractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation ; either version 2 of
   the License, or (at your option) any later version.

   CFTExtractor is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY ; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program ; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA
   ---------------------------------------------------------------------------- */

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

static bool is_strictly_in(Block *inner, Block *outer) {
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
		fprintf(stderr, "usage: %s <ARM binary> <formula file> [<entry fct (by default main)>]\n", argv[0]);
		exit(1);
	}
	WorkSpace *ws = NULL;
	PropList conf;
	Manager manager;
	
	NO_SYSTEM(conf) = true;
	TASK_ENTRY(conf) = "main";
	
	VERBOSE(conf) = false;

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
/*	cout << "Computing WCET using IPET..." << endl; */
	ws->require(ipet::WCET_FEATURE, conf);
/*	cout << "WCET value using IPET: " << ipet::WCET(ws) << endl; */

/*	cout << "Exporting to AWCET..."; */
	formula_t f;
	memset(&f, 0, sizeof(f));
	CFTREE(entry)->exportToAWCET(&f);
/*	cout << "done." << endl; */
	
	int max_loop_id = 0;

	fix_virtualized_loopinfo(entry);
	
	FILE *pwf_file = fopen(argv[2], "w");
	long long *loop_bounds = (long long*) malloc(sizeof(long long)*max_loop_id + 1);
	for (int i = 0; i <= max_loop_id; i++)
		loop_bounds[i] = -1;
		
		
	for (CFGCollection::Iter iter(*coll); iter(); iter ++) {
		for (CFG::BlockIter iter2((*iter)->blocks()); iter2(); iter2++) {
			if (LOOP_HEADER(*iter2)) {
				loop_bounds[(*iter2)->id()] = MAX_ITERATION((*iter2));
			}
		}
	}
	
	writePWF(&f, pwf_file, loop_bounds);
	fprintf(pwf_file, " loops: ");
	for (CFGCollection::Iter iter(*coll); iter(); iter ++) {
		for (CFG::BlockIter iter2((*iter)->blocks()); iter2(); iter2++) {
			if (LOOP_HEADER(*iter2)) {
				loop_bounds[(*iter2)->id()] = MAX_ITERATION((*iter2));
//				printf("bounding loop %d with %d\n", (*iter2)->id(), loop_bounds[(*iter2)->id()]);
				
				for (CFGCollection::Iter iter3(*coll); iter3(); iter3 ++) {
					for (CFG::BlockIter iter4((*iter3)->blocks()); iter4(); iter4++) {
						if (LOOP_HEADER(*iter4)) {
							if (is_strictly_in(*iter2, *iter4)) {
								fprintf(pwf_file, "l:%u _C l:%u; ", (*iter2)->id(), (*iter4)->id());
							}
						}
					}
				}
			}
		}
	}
	fprintf(pwf_file, " endl\n");

	fclose(pwf_file);
	free(loop_bounds);
	
}
