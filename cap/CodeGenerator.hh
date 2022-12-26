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

	CodeGenerator();
	void setScope(Scope& scope, bool finishPrevious = false);

	static void setOutputType(Output type);
	const std::string& getOutput() { return gen->getOutput(); }

	bool generateLine(SyntaxTreeNode& start);

private:
	bool generateFromNode(SyntaxTreeNode& node);

	static Output outputType;
	std::shared_ptr <Arch::Base> gen = nullptr;
};

}

#endif
