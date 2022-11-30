#ifndef CODE_GENERATOR_HEADER
#define CODE_GENERATOR_HEADER

#include "SyntaxTreeNode.hh"
#include "arch/Base.hh"

#include <memory>
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

	static Output outputType;
	std::shared_ptr <Arch::Base> gen = nullptr;

	std::string code;
	Scope& scope;
};

}

#endif
