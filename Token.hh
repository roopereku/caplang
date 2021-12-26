#ifndef TOKEN_HEADER
#define TOKEN_HEADER

#include <string>

namespace Cap {

enum class TokenType
{
	Identifier,
	Character,
	Operator,
	String,
	Break,

	Hexadecimal,
	Integer,
	Binary,
	Double,
	Float,

	None,
};

struct Token
{
	static const char* getTypeString(TokenType t);
	const char* getTypeString();
	std::string getString();

	char* begin;
	unsigned length;
	TokenType type;

	unsigned line;
	unsigned column;
};

}

#endif
