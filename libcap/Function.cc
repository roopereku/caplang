#include <cap/Function.hh>
#include <cap/BraceMatcher.hh>

#include <cap/event/GenericMessage.hh>

#include <stack>

namespace cap
{

bool Function::parse(ParserState& state)
{
	printf("Parsing function '%s'\n", name.getString().c_str());

	// Make sure that the first token is an opening parenthesis.
	Token signatureOpener = state.tokens.next();
	if(signatureOpener.getType() != Token::Type::Parenthesis || signatureOpener[0] != '(')
	{
		state.events.emit(GenericMessage(signatureOpener, "Expected '(' after function name", Message::Type::Error));
		return false;
	}

	// Open the function signature.
	state.braces.open(std::move(signatureOpener), state.events);

	// Parse the parameters and stop on failure.
	printf("Parsing parameters of '%s'\n", name.getString().c_str());
	if(!parameters.parse(state))
		return false;

	// Make sure that the next token is an opening curly brace.
	signatureOpener = state.tokens.next();
	if(signatureOpener.getType() != Token::Type::CurlyBrace || signatureOpener[0] != '{')
	{
		state.events.emit(GenericMessage(signatureOpener, "Expected '{' after function signature", Message::Type::Error));
		return false;
	}

	// Open the function body.
	state.braces.open(std::move(signatureOpener), state.events);

	// Parse the function body and stop on failure.
	printf("Parsing body of '%s'\n", name.getString().c_str());
	if(!Scope::parse(state))
		return false;

	return true;
}

}
