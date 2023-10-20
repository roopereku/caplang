#include <cap/BraceMatcher.hh>

#include <cap/event/ErrorMessage.hh>

namespace cap
{

bool BraceMatcher::open(Token brace, EventEmitter& events)
{
	BraceType braceType = getBraceType(brace);

	// Fail if the token is not a brace.
	if(braceType == BraceType::None)
	{
		events.emit(ErrorMessage(brace, "??? Non-brace passed to BraceMatcher::open"));
		return false;
	}

	// Fail if the brace isn't an opening brace.
	else if(braceType != BraceType::Opening)
	{
		events.emit(ErrorMessage(brace, "??? Non-opening brace passed to BraceMatcher::open"));
		return false;
	}

	else if(opener.getType() != Token::Type::Invalid)
	{
		events.emit(ErrorMessage(brace, "??? Opener already initialized"));
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
		events.emit(ErrorMessage(brace, "??? Non-brace passed to BraceMatcher::close"));
		return false;
	}

	else if(opener.getType() == Token::Type::Invalid)
	{
		events.emit(ErrorMessage(brace, "??? Opener is uninitialized"));
		return false;
	}

	// Fail if the brace isn't a closing brace.
	else if(braceType != BraceType::Closing)
	{
		events.emit(ErrorMessage(brace, "??? Non-closing brace passed to BraceMatcher::close"));
		return false;
	}

	else if(brace.getType() != opener.getType())
	{
		events.emit(ErrorMessage(brace, "??? Mismatching closing brace. Opened with" + opener.getString()));
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
