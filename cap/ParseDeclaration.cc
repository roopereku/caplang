#include "SourceFile.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseVariable(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("var"))
		return false;

	else if(inExpression)
		return true;

	//	At this point the node is an expression. Change it to variable
	current.node->type = SyntaxTreeNode::Type::Variable;
	current.node->value = &tokens[i];

	i++;
	if(!isToken(TokenType::Identifier, i) || isKeyword(tokens[i]))
		return showExpected("a name for variable", i);

	//	Prevent duplicates
	if(isDuplicateDeclaration(&tokens[i], current))
		return true;

	current.addVariable(&tokens[i]);

	//	Initialize a node for the variable name and the following expression
	current.node->left = std::make_shared <SyntaxTreeNode> (current.node, &tokens[i], SyntaxTreeNode::Type::Value);
	current.node->right = std::make_shared <SyntaxTreeNode> (current.node);
	current.node = current.node->right.get();

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
			return showExpected("a name for function", i);

		name = &tokens[i];

		//	TODO allow overloads
		//	Prevent duplicates
		//	FIXME do the duplicate checking in validation
		//if(isDuplicateDeclaration(name, current))
		//	return true;

		i++;
	}

	Function& function = current.addFunction(name);

	if(!isToken(TokenType::Parenthesis, i))
		return showExpected("parentheses after function", i);

	//	Initially we want to to have the scope cover the parentheses to parse the parameters
	function.scope = std::make_shared <Scope> (&current, ScopeContext::Function, i, i + tokens[i].length);
	i++;

	//	Parse the parameters
	if(!parseExpression(i, *function.scope, true))
		return true;

	//	TODO support function declarations
	//	Does the function have a body?
	i++;
	if(!isToken(TokenType::CurlyBrace, i) || *tokens[i].begin == '}')
		return showExpected(name ? ("a body for function '" + name->getString() + '\'') : "a body for anonymous function", i);

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
		return showExpected("a name for type", i);

	Token* name = &tokens[i];

	//	Prevent duplicates
	if(isDuplicateDeclaration(name, current))
		return true;

	i++;
	if(!isToken(TokenType::CurlyBrace, i) || *tokens[i].begin == '}')
		return showExpected("a body for type '" + name->getString() + '\'', i);

	Type& type = current.addType(name, i + 1, i + 1 + tokens[i].length);

	i = type.scope->end;
	parseScope(*type.scope);

	DBG_LOG("---- Listing type '%s' ----", name->getString().c_str());
	type.scope->root.list();

	return true;
}
