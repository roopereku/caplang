#include <cap/BraceMatcher.hh>

#include <cap/event/GenericMessage.hh>

namespace cap
{

bool BraceMatcher::open(Token brace, EventEmitter& events)
{
	BraceType braceType = getBraceType(brace);

	// Fail if the token is not a brace.
	if(braceType == BraceType::None)
	{
		events.emit(GenericMessage(brace, "??? Non-brace passed to BraceMatcher::open", Message::Type::Error));
		return false;
	}

	// Fail if the brace isn't an opening brace.
	else if(braceType != BraceType::Opening)
	{
		events.emit(GenericMessage(brace, "??? Non-opening brace passed to BraceMatcher::open", Message::Type::Error));
		return false;
	}

	else if(opener.getType() != Token::Type::Invalid)
	{
		events.emit(GenericMessage(brace, "??? Opener already initialized", Message::Type::Error));
		return false;
	}

	opener = brace;

	return true;
}

bool BraceMatcher::close(Token brace, EventEmitter& events)
{
	BraceType braceType = getBraceType(brace);

	// Fail if the token is not a brace.
	if(braceType == BraceType::None)
	{
		events.emit(GenericMessage(brace, "??? Non-brace passed to BraceMatcher::close", Message::Type::Error));
		return false;
	}

	else if(opener.getType() == Token::Type::Invalid)
	{
		events.emit(GenericMessage(brace, "??? Opener is uninitialized", Message::Type::Error));
		return false;
	}

	// Fail if the brace isn't a closing brace.
	else if(braceType != BraceType::Closing)
	{
		events.emit(GenericMessage(brace, "??? Non-closing brace passed to BraceMatcher::close", Message::Type::Error));
		return false;
	}

	else if(brace.getType() != opener.getType())
	{
		events.emit(GenericMessage(brace, "??? Mismatching closing brace. Opened with" + opener.getString(), Message::Type::Error));
		return false;
	}

	opener = Token::createInvalid();

	return true;

}

BraceType BraceMatcher::getBraceType(Token& token)
{
	switch(token.getType())
	{
		case Token::Type::Parenthesis:
			return token[0] == '(' ? BraceType::Opening : BraceType::Closing;

		case Token::Type::CurlyBrace:
			return token[0] == '{' ? BraceType::Opening : BraceType::Closing;

		case Token::Type::SquareBracket:
			return token[0] == '[' ? BraceType::Opening : BraceType::Closing;

		default:
			return BraceType::None;
	}
}

}
