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
}

const char* Cap::Token::getTypeString() const
{
	return getTypeString(type);
}

std::string Cap::Token::getString() const
{
	return std::string(begin, begin + length);
}
