#ifndef ARCH_TEST_HEADER
#define ARCH_TEST_HEADER

#include "Base.hh"

namespace Cap::Arch {

class Test : public Base
{
public:
	void prepareForLine() override;
	bool generateInstruction(SyntaxTreeNode& start, std::string& code) override;

private:
	int accumulator;
};

}

#endif
