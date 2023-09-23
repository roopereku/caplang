#ifndef CAP_BRACE_MATCHER_HH
#define CAP_BRACE_MATCHER_HH

#include <cap/Token.hh>

#include <stack>

namespace cap
{

enum class BraceType
{
	Opening,
	Closing,
	None
};

class BraceMatcher
{
public:
	bool open(Token brace);
	bool close(Token brace);

	/// Checks the type of the given brace.
	///
	/// \returns BraceTye::None if the token isn't a brace, otherwise the appropriate BraceType.
	static BraceType getBraceType(Token& token);

	size_t isOpened()
	{
		return opener.getType() != Token::Type::Invalid;
	}

	const Token& getOpener()
	{
		return opener;
	}

private:
	Token opener = Token::createInvalid();
};

}

#endif
