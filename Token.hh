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

	SingleLineComment,
	MultiLineComment,

	CurlyBrace,
	Parenthesis,
	SquareBracket,

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
	const char* getTypeString() const;
	std::string getString() const;

	char* begin;
	unsigned length;
	TokenType type;

	unsigned line;
	unsigned column;
};

}

#endif
