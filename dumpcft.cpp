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
#include <otawa/hard/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cache/cat2/features.h>
#include <otawa/etime/features.h>
#include <otawa/hard/Processor.h>

using namespace otawa; //comme import
using namespace otawa::cftree;

#define CONDITIONS_FILE "constraints.csv"
#define LOOP_BOUNDS_FILE "loop_bounds.csv"

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

struct param_func *read_pfl(char *binary) {
	char filename[256];
	strncpy(filename, binary, sizeof(filename) - 4);
	filename[sizeof(filename) - 5] = 0;
	strcat(filename, ".pfl");
	cout << "pfl file = " << filename << endl;
	FILE *f = fopen(filename, "r");
	if (f != nullptr) {
		printf("Found PFL file, reading it\n");
		char *str;
		int param_id;
		int paramcount = 0;
		while (fscanf(f, "%ms %d", &str, &param_id) == 2)
	       		paramcount ++;
		rewind(f);

		struct param_func *res = (struct param_func*) malloc(sizeof(struct param_func)*(paramcount + 1));
		int i = 0;
		while (fscanf(f, "%ms %d", &str, &param_id) == 2) {
			if (i >= paramcount) abort(); // parano
			res[i].funcname = str;
			res[i].param_id = param_id;
			i++;
		}
		res[i].funcname = nullptr;
		printf("End of PFL file processing\n");
		fclose(f);
		PFL = res;
		return res;
	} else return NULL;
}

void strreplace(char *string, const char *find, const char *replaceWith){
    if(strstr(string, replaceWith) != NULL){
        char *temporaryString = (char*) malloc(strlen(strstr(string, find) + strlen(find)) + 1);
        strcpy(temporaryString, strstr(string, find) + strlen(find));    //Create a string with what's after the replaced part
        *strstr(string, find) = '\0';    //Take away the part to replace and the part after it in the initial string
        strcat(string, replaceWith);    //Concat the first part of the string with the part to replace with
        strcat(string, temporaryString);    //Concat the first part of the string with the part after the replaced part
        free(temporaryString);    //Free the memory to avoid memory leaks
    }
}

// =================================================
// Manage the timing measurements
// =================================================
enum { NS_PER_SECOND = 1000000000 };
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

int main(int argc, char **argv) {

	// start measurements
	struct timespec start, finish, delta;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);

	try{

	if ((argc < 3) || (argc > 4)) {
		fprintf(stderr, "usage: %s <ARM binary> <formula file> [<entry fct (by default main)>]\n", argv[0]);
		exit(1);
	}
	struct param_func *pfl = read_pfl(argv[1]);

	WorkSpace *ws = NULL;
	PropList conf;
	Manager manager;
	
	NO_SYSTEM(conf) = true;
	//TASK_ENTRY(conf) = "main";
	TASK_ENTRY(conf) = (argc >= 4) ? argv[3] : "main";

	// add the processor model
#ifdef PIPELINE
	char proc[4096];
	strcpy(proc, argv[0]);
	strcat(proc, "/hw/processor.xml");
	strreplace(proc, "dumpcft", "");
	PROCESSOR_PATH(conf) = proc; // relative path to the processor file
	//PROCESSOR(conf) = hard::Processor::load(proc);
#endif
#ifdef ICACHE
	// add a default cache (will not be used into the app, but the plugin needs it in order to manage LBlocks, or a required features will make the program crash)	
	char result[4096];
	strcpy(result, argv[0]);
	strcat(result, "/hw/cache.xml");
	strreplace(result, "dumpcft", "");
	CACHE_CONFIG_PATH(conf) = result; // relative path to the cache file
#endif
	//VERBOSE(conf) = true;

	ws = manager.load(argv[1], conf);

	ws->require(otawa::VIRTUALIZED_CFG_FEATURE, conf);

	const CFGCollection *coll = INVOLVED_CFGS(ws);

	// set the entry of the task
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

	// push conditions on the CFG to make it easier to know if we generate ALT or CONDITIONAL_ALT
	ConditionParser parser;
	std::map<otawa::address_t, std::vector<std::map<bool, otawa::cftree::BranchCondition *> *> *> conditionsMap = parser.readFromFile(CONDITIONS_FILE);
	
	// adon to support loop headers
	LoopBoundParser lbp;
	std::map<int,LoopBound> lbs = lbp.readFromFile(LOOP_BOUNDS_FILE);

	// needed before running CFT
	ws->require(LOOP_HEADERS_FEATURE, conf);
	parser.pushConditionsOnCFGs(entry, &conditionsMap, &lbs);
	
#ifdef ICACHE
	ws->require(COLLECTED_LBLOCKS_FEATURE, conf);  // cache implement : retrieve L-Blocks
	ws->require(ICACHE_CATEGORY2_FEATURE, conf);   // cache implement : retrieve Block cache type
	ws->require(ICACHE_CONSTRAINT2_FEATURE, conf); // cache implement : retrieve miss penalty
#endif
	ws->require(DynFeature("otawa::cftree::EXTRACTED_CFT_FEATURE"), conf);

#ifdef ICACHE
	// set the penalty of the cache
	int icacheMissPenalty = hard::CACHE_CONFIGURATION_FEATURE.get(ws)->instCache()->missPenalty();

	// DISPLAY CACHE INFOS
	// cout << endl << "CACHE INFORMATIONS:" << endl;
	// cout << "\tSize: " << hard::CACHE_CONFIGURATION_FEATURE.get(ws)->instCache()->cacheSize() << endl;
	// cout << "\tWays: " << hard::CACHE_CONFIGURATION_FEATURE.get(ws)->instCache()->wayCount() << endl << endl;

	//cout << "Miss penalty of the current cache : " << icacheMissPenalty << endl;
	BasicCacheBlock::setPenalty(icacheMissPenalty);
#endif

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
/*	cout << "Computing WCET using IPET..." << endl; */
#ifdef PIPELINE
	etime::RECORD_TIME(conf) = true;
	ws->require(etime::EDGE_TIME_FEATURE, conf);
#endif
	// This requires to solve ILP, and thus it is not optimal, that's why we use another require
	//ws->require(ipet::WCET_FEATURE, conf);
	//cout << "WCET value using IPET: " << ipet::WCET(ws) << endl;
	
	// Instead of WCET feature, we use:
	// LOOP_HEADERS_FEATURE so as to get the LOOP_HEADER boolean value
	// FLOW_FACTS_FEATURE in order to get the loop bounds
	// BB_TIME_FEATURE to get the WCET of basic blocks
	ws->require(ipet::FLOW_FACTS_FEATURE, conf);
	ws->require(ipet::BB_TIME_FEATURE, conf);
/*	cout << "Exporting to AWCET..."; */
	formula_t f;
	memset(&f, 0, sizeof(f));
	/* infeasible paths implement */
	CFTREE(entry)->exportToAWCET(&f, pfl);
/*	cout << "done." << endl; */

#ifdef IP
	if(allConstraints.size() > 0){
		cout << "Some constraints could not be attached to a node: " ;
		printPseudoPaths(allConstraints);
		throw InvalidConstraintException("Error : could not attach all constraints to tree nodes");
	}
#endif
	
	int max_loop_id = 0;

	fix_virtualized_loopinfo(entry);
	
	FILE *pwf_file = fopen(argv[2], "w");
	
	for (CFGCollection::Iter iter(*coll); iter(); iter ++)
		for (CFG::BlockIter iter2((*iter)->blocks()); iter2(); iter2++)
			if (LOOP_HEADER(*iter2))
				if ((*iter2)->id() > max_loop_id)
					max_loop_id = (*iter2)->id();
			

	long long *loop_bounds = (long long*) malloc(sizeof(long long)*(max_loop_id + 1));
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

	// avoid double free of pointers
	finder.empty();

	} catch (otawa::Exception &ex){
		cout << "An error occured : " << ex.message() << endl;
		cerr << "An error occured : " << ex.message() << endl;
	}

	// stop measurements
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &finish);
	// compute analysis time
	sub_timespec(start, finish, &delta);
	printf("dumpcft time: %d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);
	
}
