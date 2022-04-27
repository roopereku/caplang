#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseVariable(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("var"))
		return false;

	else if(inExpression)
		return true;

	//	Don't make nested Variable nodes
	if(current.node->parent->type != SyntaxTreeNode::Type::Variable)
	{
		//	At this point the node has type "None". Change it to variable
		current.node->type = SyntaxTreeNode::Type::Variable;
		current.node->value = &tokens[i];

		//	Initialize a node for the expression that declares variables
		current.node->left = std::make_shared <SyntaxTreeNode> (current.node);
		current.node = current.node->left.get();
	}

	i++;
	return true;
}

bool Cap::SourceFile::parseFunction(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("func"))
		return false;

	i++;
	Token* name = nullptr;

	if(!inExpression)
	{
		if(!isToken(TokenType::Identifier, i) || isKeyword(tokens[i]))
		{
			Logger::error(tokens.getPath(), tokens[i], "Expected a name for a function");
			return errorOut();
		}

		name = &tokens[i];

		i++;
	}

	Function& function = current.addFunction(name);

	if(!isToken(TokenType::Parenthesis, i))
	{
		Logger::error(tokens.getPath(), tokens[i], "Expected parentheses after function name '%s'", name->getString().c_str());
		return errorOut();
	}

	//	Initially we want to to have the scope cover the parentheses to parse the parameters
	function.scope = std::make_shared <Scope> (&current, ScopeContext::Function, i, i + tokens[i].length);
	function.scope->node = function.scope->root.left.get();
	i++;

	//	Parse the parameters
	if(!parseExpression(i, *function.scope, true))
		return true;

	//	TODO support function declarations
	//	Does the function have a body?
	i++;
	if(!isToken(TokenType::CurlyBrace, i) || *tokens[i].begin == '}')
	{
		Logger::error(tokens.getPath(), tokens[i], "Expected a body for function '%s'", name->getString().c_str());
		return errorOut();
	}

	//	Tell the scope to cover the body
	function.scope->begin = i + 1;
	function.scope->end = i + tokens[i].length;
	i = function.scope->end;

	//	Parse the function
	function.scope->node = function.scope->root.right.get();
	parseScope(*function.scope);

	DBG_LOG("---- Listing function '%s' ----", !name ? "anonymous" : name->getString().c_str());

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
	if(!isToken(TokenType::Identifier, i) || isKeyword(tokens[i]))
	{
		Logger::error(tokens.getPath(), tokens[i], "Expected a name for a type");
		return errorOut();
	}

	Token* name = &tokens[i];

	i++;
	if(!isToken(TokenType::CurlyBrace, i) || *tokens[i].begin == '}')
	{
		Logger::error(tokens.getPath(), tokens[i], "Expected a body for type '%s'", name->getString().c_str());
		return errorOut();
	}

	Type& type = current.addType(name, i + 1, i + 1 + tokens[i].length);

	i = type.scope->end - 1;
	parseScope(*type.scope);

	DBG_LOG("---- Listing type '%s' ----", name->getString().c_str());
	type.scope->root.list();

	return true;
}
