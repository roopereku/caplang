#include <cap/BraceMatcher.hh>

namespace cap
{

bool BraceMatcher::open(Token brace)
{
	BraceType braceType = getBraceType(brace);

	// Fail if the token is not a brace.
	if(braceType == BraceType::None)
	{
		printf("Non-brace '%s' passed to BraceMatcher::open\n", brace.getTypeString());
		return false;
	}

	// Fail if the brace isn't an opening brace.
	else if(braceType != BraceType::Opening)
	{
		printf("Non-opening brace '%s' passed to BraceMatcher::open\n", brace.getTypeString());
		return false;
	}

	else if(opener.getType() != Token::Type::Invalid)
	{
		printf("??? Opener already opened\n");
		return false;
	}

	opener = brace;

	return true;
}

bool BraceMatcher::close(Token brace)
{
	BraceType braceType = getBraceType(brace);

	// Fail if the token is not a brace.
	if(braceType == BraceType::None)
	{
		printf("Non-brace '%s' passed to BraceMatcher::close\n", brace.getTypeString());
		return false;
	}

	else if(opener.getType() == Token::Type::Invalid)
	{
		printf("??? Opener not opened\n");
		return false;
	}

	// Fail if the brace isn't a closing brace.
	else if(braceType != BraceType::Closing)
	{
		printf("Non-closing brace '%s' passed to BraceMatcher::close\n", brace.getTypeString());
		return false;
	}

	else if(brace.getType() != opener.getType())
	{
		printf("Mismatching closing bracket '%c' (Opened with '%c')\n", brace[0], opener[0]);
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
