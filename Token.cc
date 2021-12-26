#include "Token.hh"

const char* Cap::Token::getTypeString(TokenType t)
{
	switch(t)
	{
		case TokenType::Identifier: return "Identifier";
		case TokenType::Character: return "Character";
		case TokenType::Operator: return "Operator";
		case TokenType::String: return "String";

		case TokenType::Hexadecimal: return "Hexadecimal";
		case TokenType::Integer: return "Integer";
		case TokenType::Binary: return "Binary";
		case TokenType::Double: return "Double";
		case TokenType::Float: return "Float";

		case TokenType::None: return "None";
	}
}

const char* Cap::Token::getTypeString()
{
	return getTypeString(type);
}
