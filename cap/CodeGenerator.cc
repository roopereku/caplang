#include "CodeGenerator.hh"
#include "arch/X86Intel.hh"
#include "arch/Test.hh"
#include "Logger.hh"
#include "Scope.hh"
#include "Debug.hh"

Cap::CodeGenerator::Output Cap::CodeGenerator::outputType;

Cap::CodeGenerator::CodeGenerator(Scope& scope) : scope(scope)
{
	//	Only functions get a name in the code :-)
	if(scope.ctx == ScopeContext::Function)
	{
	}

	switch(outputType)
	{
		case Output::Test: gen = std::make_shared <Arch::Test> (scope); break;
		case Output::X86Intel: gen = std::make_shared <Arch::X86Intel> (scope); break;
	}
}

void Cap::CodeGenerator::setOutput(Output type)
{
	outputType = type;
}

bool Cap::CodeGenerator::generateLine(SyntaxTreeNode& start)
{
	Logger::warning("generate line");
	SyntaxTreeNode* expr = &start;
	gen->prepareForLine();

	//	Skip nodes until we find an expression
	while(expr->left && expr->type != SyntaxTreeNode::Type::Expression)
		expr = expr->left.get();

	//	No expression here so do nothing
	if(!expr->left)
		return true;

	//	Generate instructions for the expression
	bool result = generateFromNode(*expr->left);

	DBG_LOG("CODE IS NOW\n%s", code.c_str());
	return result;
}

bool Cap::CodeGenerator::generateFromNode(SyntaxTreeNode& node)
{
	//	Ignore values
	if(node.type == SyntaxTreeNode::Type::Value)
		return true;

	//	Generate instructions on the left side
	if(node.left && !generateFromNode(*node.left))
		return false;

	//	Generate instructions on the right side
	if(node.right && !generateFromNode(*node.right))
		return false;

	DBG_LOG("Process '%s'", node.getTypeString());
	return gen->generateInstruction(node, code);
}
