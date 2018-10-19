#ifndef CFTREE_H
#define CFTREE_H 1
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/otawa.h>

namespace otawa { namespace cftree {

class CFTreeExtractor : public Processor {
public:
	static p::declare reg;
	explicit CFTreeExtractor(p::declare &r = reg);

protected:
	void processWorkSpace(WorkSpace * /*ws*/) override;
	void configure(const PropList &props) override;
private:
	void processCFG(CFG *cfg);
};

extern p::feature EXTRACTED_CFTREE_FEATURE;

} }

#endif
