#include <cap/Function.hh>
#include <cap/BraceMatcher.hh>
#include <cap/PrimitiveType.hh>

#include <cap/event/GenericMessage.hh>

#include <stack>

namespace cap
{

bool Function::parse(ParserState& state)
{
	printf("Parsing function '%s'\n", name.getString().c_str());

	// Make sure that the first token is an opening parenthesis.
	Token opener = state.tokens.next();
	if(opener.getType() != Token::Type::Parenthesis || opener[0] != '(')
	{
		state.events.emit(GenericMessage(opener, "Expected '(' after function name\n", Message::Type::Error));
		return false;
	}

	// Open the function signature.
	ParserState paramState(state.tokens, state.events, getRoot());
	paramState.braces.open(Token(opener), paramState.events);

	if(!createVariable(Token(opener), paramState, true))
	{
		printf("createVariable failed\n");
		return false;
	}

	// Parse the parameters and stop on failure.
	printf("Parsing parameters of '%s'\n", name.getString().c_str());
	if(!parameters.parse(paramState))
		return false;

	state.node = getRoot()->findLastNode();

	// Make sure that the next token is an opening curly brace.
	opener = state.tokens.next();
	if(opener.getType() != Token::Type::CurlyBrace || opener[0] != '{')
	{
		state.events.emit(GenericMessage(opener, "Expected '{' after function signature\n", Message::Type::Error));
		return false;
	}

	// Open the function body.
	state.braces.open(std::move(opener), state.events);

	// Parse the function body and stop on failure.
	printf("Parsing body of '%s'\n", name.getString().c_str());
	if(!Scope::parse(state))
		return false;

	return true;
}

Type& Function::getReturnType()
{
	return returnType ? *returnType : Type::getInvalid();
}

}
