#ifndef TOKEN_HEADER
#define TOKEN_HEADER

namespace Cap {

enum class TokenType
{
	Identifier,
	Character,
	Operator,
	String,

	Hexadecimal,
	Integer,
	Binary,
	Double,
	Float,
};

struct Token
{
	char* begin;
	unsigned length;
	TokenType type;

	unsigned line;
	unsigned column;
};

}

#endif
