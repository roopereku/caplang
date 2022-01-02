#include "SourceFile.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseVariable(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("var"))
		return false;

	else if(inExpression)
		return true;

	current.variables.emplace_back(&tokens[i]);
	Variable& variable = current.variables.back();

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

	current.functions.emplace_back(&tokens[i]);
	Function& function = current.functions.back();

	i++;
	if(!isToken(TokenType::Parenthesis, i))
		return showExpected("parentheses after function name", i);

	DBG_LOG("Added function '%s'", function.name->getString().c_str());

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

	Token* typeName = &tokens[i];
	i++;

	if(!isToken(TokenType::CurlyBrace, i))
		return showExpected("a body for type '" + typeName->getString() + '\'', i);

	current.types.emplace_back(typeName, &current, ScopeContext::Type, i + 1, i + tokens[i].length);
	Scope& typeScope = current.types.back();

	DBG_LOG("Added type '%s'", typeScope.name->getString().c_str());

	return true;
}
