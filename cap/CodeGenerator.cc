#include "CodeGenerator.hh"
#include "arch/X86Intel.hh"
#include "Logger.hh"
#include "Scope.hh"
#include "Debug.hh"

Cap::CodeGenerator::Output Cap::CodeGenerator::outputType;

Cap::CodeGenerator::CodeGenerator(Scope& scope) : scope(scope)
{
	switch(outputType)
	{
		case Output::X86Intel: gen = std::make_shared <Arch::X86Intel> (); break;
	}

	run();
}

Cap::CodeGenerator::CodeGenerator(Scope& scope, std::shared_ptr <Arch::Base> gen) : scope(scope), gen(gen)
{
	run();
}

bool Cap::CodeGenerator::run()
{
	Logger::warning("Starting code generation");

	if(scope.getFunctionCount() > 0)
	{
		for(size_t i = 0; i < scope.getFunctionCount(); i++)
		{
			Logger::warning("Generating function '%s'", scope.findFunction(i)->name->getString().c_str());
			CodeGenerator cg(*scope.findFunction(i)->scope);
		}
	}

	SyntaxTreeNode* n = &scope.root;
	gen->setScope(scope);

	/*	Traverse through the AST "Line-by-line". Lines begin on one node
	 *	and continue using the left-side node. The right side node is the
	 *	next line, which means that we can generate code using the left node
	 *	and skip over to the right-side node */
	while(n->right)
	{
		//	The left node might not exist. This means that the node is a "block"
		if(n->left)
		{
			if(!generateLine(n))
				return false;
		}

		//	The left node isn't present so treat the node as a block
		else
		{
			if(n->type != SyntaxTreeNode::Type::Block)
			{
				Logger::error("???: Node isn't a block");
				return false;
			}

			//	The length of the token contains the block index. Let's generate code for that scope
			Logger::warning("No left node because current is '%s'", n->getTypeString());
			CodeGenerator cg(*scope.findBlock(n->value->length), gen);
		}

		n = n->right.get();
	}

	Logger::warning("Generation done");
	DBG_LOG("Output\n%s", gen->getOutput().c_str());
	return true;
}

void Cap::CodeGenerator::setOutputType(Output type)
{
	outputType = type;
}

bool Cap::CodeGenerator::generateLine(SyntaxTreeNode* start)
{
	//Logger::warning("generate line");
	SyntaxTreeNode* expr = start;
	gen->prepareForLine();

	//	Skip nodes until we find an expression
	while(expr->left && expr->type != SyntaxTreeNode::Type::Expression)
		expr = expr->left.get();

	//	No expression here so do nothing
	if(!expr->left)
		return true;

	//	Generate instructions for the expression
	bool result = generateFromNode(expr->left.get());

	return result;
}

bool Cap::CodeGenerator::generateFromNode(SyntaxTreeNode* node)
{
	//	Ignore values and nodes nops
	if(node->type == SyntaxTreeNode::Type::Value || node->type == SyntaxTreeNode::Type::None)
		return true;

	//	Generate instructions on the left side
	if(node->left && !generateFromNode(node->left.get()))
		return false;

	//	Generate instructions on the right side
	if(node->right && !generateFromNode(node->right.get()))
		return false;

	return gen->generateInstruction(*node);
}
