#ifndef CAP_TOKEN_HH
#define CAP_TOKEN_HH

#include <cap/ParserContext.hh>

#include <cstddef>
#include <string>
#include <stack>

namespace cap
{

class Source;
class Client;

class Token
{
public:
	enum class Type
	{
		OpeningBracket,
		ClosingBracket,
		Identifier,
		Integer,
		Float,
		Hexadecimal,
		Binary,
		Octal,
		String,
		Operator,
		Invalid,
	};

	class ParserContext;

	Token(size_t index, size_t length);

	size_t getIndex() const;
	size_t getLength() const;
	Type getType() const;

	static const char* getTypeString(Type type);
	const char* getTypeString() const;

	bool isValid() const;
	bool isReservedIdentifier(cap::ParserContext& ctx) const;
	bool isOpeningBracket(cap::ParserContext& ctx, wchar_t ch) const;
	bool isClosingBracket(cap::ParserContext& ctx, wchar_t ch) const;

	bool canBeValue() const;

	bool isBeforeNewline(cap::ParserContext& ctx);

	static Token parseFirst(ParserContext& ctx);
	static Token parseNext(ParserContext& ctx, Token token);

private:
	struct ParseResult
	{
		ParseResult(Token::Type result) : tokenType(result) {}
		ParseResult(std::wstring&& error)
			: tokenType(Type::Invalid), error(std::move(error)) {}

		Token::Type tokenType;
		std::wstring error;
	};

	static Token parse(ParserContext& ctx, Token token);
	static void skipWhitespace(ParserContext& ctx, size_t& i);
	ParseResult parseBracket(ParserContext& ctx, size_t& i);
	ParseResult parseIdentifier(ParserContext& ctx, size_t& i);
	ParseResult parseOperator(ParserContext& ctx, size_t& i);

	ParseResult parseNumeric(ParserContext& ctx, size_t& i);
	ParseResult parseDecimal(ParserContext& ctx, size_t& i);
	ParseResult parseHexadecimal(ParserContext& ctx, size_t& i);
	ParseResult parseBinary(ParserContext& ctx, size_t& i);
	ParseResult parseOctal(ParserContext& ctx, size_t& i);

	bool setTypeIfMoved(ParserContext& ctx, size_t& i, ParseResult(Token::*callback)(ParserContext&, size_t&));

	size_t index;
	size_t length;

	Type type = Type::Invalid;
};

class Token::ParserContext : public cap::ParserContext
{
public:
	ParserContext(Client& client, Source& source)
		: cap::ParserContext(client, source)
	{
	}

	std::stack <Token> openedBrackets;
};

}

#endif
