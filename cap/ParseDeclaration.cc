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
		if(isDuplicateDeclaration(name, current))
			return true;

		i++;
	}

	if(!isToken(TokenType::Parenthesis, i))
		return showExpected("parentheses after function", i);

	//	TODO parse the parameters
	//	Skip the parentheses
	i += tokens[i].length + 1;

	if(!isToken(TokenType::CurlyBrace, i) || *tokens[i].begin == '}')
		return showExpected(name ? ("a body for function '" + name->getString() + '\'') : "a body for anonymous function", i);

	DBG_LOG("Spans across %u", tokens[i].length);

	Function& function = current.addFunction(name, i + 1, i + tokens[i].length);

	i = function.scope->end;
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
