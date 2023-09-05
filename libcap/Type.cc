#include <cap/Type.hh>
#include <cap/BraceMatcher.hh>

#include <stack>

namespace cap
{

bool Type::parse(Tokenizer& tokens, BraceMatcher& braces)
{
	printf("Parsing type '%s'\n", name.getString().c_str());

	// Make sure that the next token is an opening curly brace.
	Token signatureOpener = tokens.next();
	if(signatureOpener.getType() != Token::Type::CurlyBrace || signatureOpener[0] != '{')
	{
		printf("Expected '{' after a type name\n");
		return false;
	}

	// Open the type body.
	braces.open(std::move(signatureOpener));

	// Parse the type body and stop on failure.
	printf("Parsing body of '%s'\n", name.getString().c_str());
	if(!Scope::parse(tokens, braces))
		return false;

	return true;
}

}
