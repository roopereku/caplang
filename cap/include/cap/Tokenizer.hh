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
	enum class Error
	{
		UnterminatedComment,
		InvalidCharacter,
		JunkAfterNumber,
		TooManyDots,
		None
	};

	Tokenizer(std::string& data);

	/// Consumes the next token.
	/// \return The consumed token or an invalid token if there's no more tokens.
	Token next();

	/// Checks whether there are remaining tokens.
	/// \return True if there no remaining tokens.
	bool empty();

	/// Resets the state of the Tokenizer.
	void reset();

	/// Gets the error if a token was invalid.
	///
	/// \return The error reason.
	Error getError();

	/// Gets the error string if a token was invalid.
	///
	/// \return The error as a string if present.
	std::string_view getErrorString();

private:
	bool nextCharacter();
	void updateCursorPosition();

	Token::Type parseIdentifier();
	Token::Type parseOperator();
	Token::Type parseNumber();
	Token::Type parseBinary();
	Token::Type parseComment();
	Token::Type parseSingleLineComment();
	Token::Type parseMultiLineComment();
	Token::Type parseHexadecimal();
	Token::Type parseIntegerOrFloat();
	Token::Type junkAfterNumber();

	std::string& data;
	size_t index;
	size_t origin;

	Token::IndexType currentRow;
	Token::IndexType currentColumn;

	bool nextIsComment = false;
	bool ignoreLastCharacter = false;
	Error error = Error::None;
};

}

#endif
