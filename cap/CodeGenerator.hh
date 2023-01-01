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
		X86Intel
	};

	CodeGenerator(Scope& scope);

	static void setOutputType(Output type);
	const std::string& getOutput() { return gen->getOutput(); }

private:
	CodeGenerator(Scope& scope, std::shared_ptr <Arch::Base> gen);

	bool run();
	bool generateLine(SyntaxTreeNode* start);
	bool generateFromNode(SyntaxTreeNode* node);

	static Output outputType;
	std::shared_ptr <Arch::Base> gen = nullptr;

	Scope& scope;
};

}

#endif
