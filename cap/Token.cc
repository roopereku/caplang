#include "Token.hh"

const char* Cap::Token::getTypeString(TokenType t)
{
	switch(t)
	{
		case TokenType::Identifier: return "Identifier";
		case TokenType::Character: return "Character";
		case TokenType::Operator: return "Operator";
		case TokenType::String: return "String";
		case TokenType::Break: return "Break";

		case TokenType::SingleLineComment: return "SingleLineComment";
		case TokenType::MultiLineComment: return "MultiLineComment";

		case TokenType::CurlyBrace: return  "CurlyBrace";
		case TokenType::Parenthesis: return  "Parenthesis";
		case TokenType::SquareBracket: return  "SquareBracket";

		case TokenType::Hexadecimal: return "Hexadecimal";
		case TokenType::Integer: return "Integer";
		case TokenType::Binary: return "Binary";
		case TokenType::Double: return "Double";
		case TokenType::Float: return "Float";

		case TokenType::None: return "None";
	}

	return "???";
}

const char* Cap::Token::getTypeString() const
{
	return getTypeString(type);
}

std::string Cap::Token::getString() const
{
	return std::string(begin, begin + length);
}

bool Cap::Token::stringEquals(const char* str) const
{
	size_t i;

	for(i = 0; i < length && str[i] != 0; i++)
	{
		//	Quit on the first character mismatch
		if(*(begin + i) != str[i])
			return false;
	}

	//	If both the token and string are at the end, they're identical
	return str[i] == 0 && i == length;
}

bool Cap::Token::tokenEquals(Token* token) const
{
	//	The tokens can't be the same on length mismatch
	if(length != token->length) return false;

	for(size_t i = 0; i < length; i++)
	{
		//	Quit on the first character mismatch
		if(*(begin + i) != *(token->begin + i))
			return false;
	}

	return true;
}
