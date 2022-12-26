#ifndef ARCH_BASE_HEADER
#define ARCH_BASE_HEADER

#include "../SyntaxTreeNode.hh"

#include <string>

namespace Cap {
class Scope;

namespace Arch {

class Base
{
public:
	virtual void prepareForLine()=0;
	virtual void finishScope()=0;

	virtual bool generateInstruction(SyntaxTreeNode& node)=0;

	void setScope(Scope& scope);
	const std::string& getOutput() { return code; }

protected:
	enum class InstructionType
	{
		Arithmetic,
		Assignment,
		Comparison,
		Unary
	};

	const char* instructionTypeString(InstructionType t);
	InstructionType getType(SyntaxTreeNode::Type t);
	std::string getValue(SyntaxTreeNode& node);

	Scope* scope = nullptr;
	std::string code;
};

}}

#endif
