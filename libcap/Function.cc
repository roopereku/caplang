#include <cap/Function.hh>
#include <cap/BraceMatcher.hh>

#include <stack>

namespace cap
{

bool Function::parse(Tokenizer& tokens, BraceMatcher& braces)
{
	printf("Parsing function '%s'\n", name.getString().c_str());

	// Make sure that the first token is an opening parenthesis.
	Token signatureOpener = tokens.next();
	if(signatureOpener.getType() != Token::Type::Parenthesis || signatureOpener[0] != '(')
	{
		printf("Expected '(' after function name\n");
		return false;
	}

	// Open the function signature.
	braces.open(std::move(signatureOpener));

	// Parse the parameters and stop on failure.
	printf("Parsing parameters of '%s'\n", name.getString().c_str());
	if(!parameters.parse(tokens, braces))
		return false;

	// Make sure that the next token is an opening curly brace.
	signatureOpener = tokens.next();
	if(signatureOpener.getType() != Token::Type::CurlyBrace || signatureOpener[0] != '{')
	{
		printf("Expected '{' after function signature\n");
		return false;
	}

	// Open the function body.
	braces.open(std::move(signatureOpener));

	// Parse the function body and stop on failure.
	printf("Parsing body of '%s'\n", name.getString().c_str());
	if(!Scope::parse(tokens, braces))
		return false;

	return true;
}

}
