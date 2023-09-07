#ifndef CAP_TOKENIZER_HH
#define CAP_TOKENIZER_HH

#include <cap/Token.hh>

#include <string_view>
#include <string>

namespace cap
{

class Tokenizer
{
public:
	Tokenizer(std::string_view filePath);

	/// Consumes the next token.
	///
	/// \returns The consumed token or an invalid token if there's no more tokens.
	Token next();

	/// Checks whether there are remaining tokens.
	///
	/// \returns True if there no remaining tokens.
	bool empty();

private:
	bool nextCharacter();
	void updateCursorPosition();

	Token::Type parseIdentifier();
	Token::Type parseOperator();
	Token::Type parseNumber();
	Token::Type junkAfterNumber();

	std::string data;
	size_t index = 0;

	Token::IndexType currentRow = 1;
	Token::IndexType currentColumn = 0;
};

}

#endif
