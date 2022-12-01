#ifndef ARCH_X86_INTEL_HEADER
#define ARCH_X86_INTEL_HEADER

#include "Base.hh"

#include <array>

namespace Cap::Arch {

class X86Intel : public Base
{
public:
	X86Intel(Scope& scope) : Base(scope)
	{
	}

	void prepareForLine() override;
	bool generateInstruction(SyntaxTreeNode& start, std::string& code) override;

private:
	std::array <const char*, 7> registers
	{
		"eax", "ebx", "edx",
		"r12d", "r13d", "r14d", "r15d",
	};

	size_t currentRegister;
	bool registerHasValue;
	size_t stackPointer = 0;
};

}

#endif
