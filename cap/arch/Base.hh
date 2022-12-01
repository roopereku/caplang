#ifndef ARCH_BASE_HEADER
#define ARCH_BASE_HEADER

#include "../SyntaxTreeNode.hh"

namespace Cap {
class Scope;

namespace Arch {

class Base
{
public:
	Base(Scope& scope) : scope(scope)
	{
	}

	virtual void prepareForLine()=0;
	virtual bool generateInstruction(SyntaxTreeNode& node, std::string& code)=0;

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

	Scope& scope;
};

}}

#endif