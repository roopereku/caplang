#include "SourceFile.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseVariable(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("var"))
		return false;

	else if(inExpression)
		return true;

	i++;
	if(!isToken(TokenType::Identifier, i))
		return showExpected("a name for variable", i);

	Variable& variable = current.addVariable(&tokens[i]);

	//	Initialize a node for the variable name and the following expression
	current.node->left = std::make_shared <SyntaxTreeNode> (current.node, &tokens[i], SyntaxTreeNode::Type::Value);
	current.node->right = std::make_shared <SyntaxTreeNode> (current.node);
	current.node = current.node->right.get();

	DBG_LOG("Added variable '%s'", variable.name->getString().c_str());
	return true;
}

bool Cap::SourceFile::parseFunction(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("func"))
		return false;

	else if(inExpression)
		return true;

	i++;
	if(!isToken(TokenType::Identifier, i))
		return showExpected("a name for function", i);

	Token* name = &tokens[i];

	i++;
	//	TODO parse the parameters
	if(!isToken(TokenType::Parenthesis, i))
		return showExpected("parentheses after function name", i);

	//	Skip the parentheses
	i += tokens[i].length + 2;
	if(!isToken(TokenType::CurlyBrace, i))
		return showExpected("a body for function '" + name->getString() + '\'', i);

	DBG_LOG("Spans across %lu", tokens[i].length);

	DBG_LOG("Added function '%s'", name->getString().c_str());
	Function& function = current.addFunction(name, i + 1, i + 1 + tokens[i].length);

	i = function.scope->end;
	parseScope(*function.scope);

	DBG_LOG("---- Listing function '%s' ----", name->getString().c_str());
	function.scope->root.list();
	return true;
}

bool Cap::SourceFile::parseType(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("type"))
		return false;

	else if(inExpression)
		return true;

	i++;
	if(!isToken(TokenType::Identifier, i))
		return showExpected("a name for type", i);

	Token* name = &tokens[i];
	i++;

	if(!isToken(TokenType::CurlyBrace, i))
		return showExpected("a body for type '" + name->getString() + '\'', i);

	Type& type = current.addType(name, i + 1, i + 1 + tokens[i].length);
	DBG_LOG("Added type '%s'", name->getString().c_str());

	i++;
	return true;
}
