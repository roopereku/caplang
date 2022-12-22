#ifndef ARCH_X86_INTEL_HEADER
#define ARCH_X86_INTEL_HEADER

#include "Base.hh"
#include "../Variable.hh"

#include <unordered_map>
#include <array>

namespace Cap::Arch {

class X86Intel : public Base
{
public:
	void prepareForLine() override;
	bool generateInstruction(SyntaxTreeNode& start, std::string& code) override;

private:
	std::array <const char*, 7> registers
	{
		"eax", "ebx", "edx",
		"r12d", "r13d", "r14d", "r15d",
	};

	std::array <bool, 7> registerHasValue;
	std::unordered_map <Variable*, size_t> stackLocations;
	std::unordered_map <SyntaxTreeNode*, size_t> registerUsedAt;

	size_t currentRegister;
	size_t previousRegister;

	SyntaxTreeNode* previousNode;
	size_t stackPointer = 0;
};

}

#endif
