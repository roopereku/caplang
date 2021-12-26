#ifndef SOURCE_TOKENIZER_HEADER
#define SOURCE_TOKENIZER_HEADER

#include "Token.hh"

#include <vector>
#include <string>

namespace Cap {

class TokenizedSource
{
public:
	TokenizedSource(const char* path);

private:
	void addToken(TokenType type, size_t begin, size_t end);
	void tokenize();

	bool parseIdentifier(size_t& i);
	bool parseOperator(size_t& i);
	bool parseString(size_t& i);

	bool parseHexadecimal(size_t& i);
	bool parseNumeric(size_t& i);
	bool parseDecimal(size_t& i);
	bool parseBinary(size_t& i);

	std::string data;
	std::vector <Token> tokens;
};

}

#endif
