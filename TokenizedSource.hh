#ifndef SOURCE_TOKENIZER_HEADER
#define SOURCE_TOKENIZER_HEADER

#include "Token.hh"

#include <vector>
#include <string>

namespace Cap {

class TokenizedSource
{
public:
	TokenizedSource(const std::string& path);
	bool matchBraces();

	inline size_t count() { return tokens.size(); }
	inline const char* getPath() { return path.c_str(); }
	inline Token& operator[](size_t index) { return tokens[index]; }
	inline size_t getIndex(size_t tokenIndex) { return tokens[tokenIndex].begin - &data[0]; }

private:
	void addToken(TokenType type, size_t begin, size_t end);
	bool matchBrace(size_t i, char match);
	void tokenize();

	bool parseIdentifier(size_t& i);
	bool parseOperator(size_t& i);
	bool parseBracket(size_t& i);
	bool parseString(size_t& i);
	bool parseBreak(size_t& i);

	bool parseSingleLineComment(size_t& i);
	bool parseMultiLineComment(size_t& i);
	bool parseComment(size_t& i);

	bool parseHexadecimal(size_t& i);
	bool parseNumeric(size_t& i);
	bool parseDecimal(size_t& i);
	bool parseBinary(size_t& i);

	std::string path;
	std::string data;

	std::vector <Token> tokens;
};

}

#endif
