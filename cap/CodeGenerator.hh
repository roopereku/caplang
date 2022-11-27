#ifndef CODE_GENERATOR_HEADER
#define CODE_GENERATOR_HEADER

#include "SyntaxTreeNode.hh"

#include <string>

namespace Cap
{

class Scope;
class CodeGenerator
{
public:
	enum class Output
	{
		Test
	};

	CodeGenerator(Scope& scope);

	static void setOutput(Output type);
	bool generateLine(SyntaxTreeNode& start);

private:
	bool generateFromNode(SyntaxTreeNode& node);
	bool generateInstructionTest(SyntaxTreeNode& node);

	enum class InstructionType
	{
		Arithmetic,
		Assignment,
		Comparison,
		Unary
	};

	const char* instructionTypeString(InstructionType t);
	InstructionType getType(SyntaxTreeNode::Type t);

	static Output outputType;

	//	FIXME remove because this is specific to the test language
	int accumulator;
	int sp;

	std::string code;
	Scope& scope;
};

}

#endif
