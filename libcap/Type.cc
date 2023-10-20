#include <cap/Type.hh>
#include <cap/BraceMatcher.hh>
#include <cap/PrimitiveType.hh>

#include <cap/event/ErrorMessage.hh>

#include <cap/node/TypeDeclaration.hh>

namespace cap
{

bool Type::parse(ParserState& state)
{
	printf("Parsing type '%s'\n", name.getString().c_str());

	// Make sure that the next token is an opening curly brace.
	Token signatureOpener = state.tokens.next();
	if(signatureOpener.getType() != Token::Type::CurlyBrace || signatureOpener[0] != '{')
	{
		state.events.emit(ErrorMessage(signatureOpener, "Expected '{' after a type name"));
		return false;
	}

	// Open the type body.
	state.braces.open(std::move(signatureOpener), state.events);

	// Parse the type body and stop on failure.
	printf("Parsing body of '%s'\n", name.getString().c_str());
	if(!Scope::parse(state))
		return false;

	return true;
}

Type& Type::getPrimitive(Token::Type tokenType)
{
	switch(tokenType)
	{
		case Token::Type::Integer:
		{
			return *Scope::getSharedScope().getMember("int64")->as <TypeDeclaration> ()->type;
		}

		default:
			return getInvalid();
	}
}

Type& Type::getInvalid()
{
	return *Scope::getSharedScope().getMember("invalidType")->as <TypeDeclaration> ()->type;
}

bool Type::hasOperator(TwoSidedOperator::Type type)
{
	return false;
}

bool Type::hasOperator(OneSidedOperator::Type type)
{
	return false;
}

}
