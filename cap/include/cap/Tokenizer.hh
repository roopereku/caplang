#ifndef CAP_TOKENIZER_HH
#define CAP_TOKENIZER_HH

#include <cap/Token.hh>

#include <string>

namespace cap
{

class Tokenizer
{
public:
	Tokenizer(std::string& data);

	/// Consumes the next token.
	/// \return The consumed token or an invalid token if there's no more tokens.
	Token next();

	/// Checks whether there are remaining tokens.
	/// \return True if there no remaining tokens.
	bool empty();

	/// Resets the state of the Tokenizer.
	void reset();

private:
	bool nextCharacter();
	void updateCursorPosition();

	Token::Type parseIdentifier();
	Token::Type parseOperator();
	Token::Type parseNumber();
	Token::Type junkAfterNumber();

	std::string& data;
	size_t index;

	Token::IndexType currentRow;
	Token::IndexType currentColumn;
};

}

#endif