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
	//if(current.node->parent->type != SyntaxTreeNode::Type::Variable)
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

	//	The function name has to be an identifier that's not a keyword
	if(isToken(TokenType::Identifier, i))
	{
		if(isKeyword(tokens[i]))
		{
			Logger::error(tokens[i], "Expected a name for a function");
			return errorOut();
		}

		name = &tokens[i];
		i++;
	}

	if(!isToken(TokenType::Parenthesis, i))
	{
		Logger::error(tokens[i], "Expected parenthesis for function parameters");
		return errorOut();
	}

	Function& function = current.addFunction(name);

	i++;
	if(!parseLine(i, *function.scope, true))
		return errorOut();

	//	Now that the parameters are parsed, switch to the body
	function.scope->node = function.scope->root.right.get();
	i++;

	if(!parseBody(i, *function.scope))
		return errorOut();

	DBG_LOG("LISTING FUNCTION");
	function.scope->root.list();

	i--;
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
		Logger::error(tokens[i], "Expected a name for a type");
		return errorOut();
	}

	Token* name = &tokens[i];
	Type& type = current.addType(name);

	i++;
	if(!parseBody(i, *type.scope))
		return errorOut();

	DBG_LOG("---- Listing type '%s' ----", name->getString().c_str());
	type.scope->root.list();

	return true;
}
