#include <gtest/gtest.h>

#include <cap/test/DynamicSource.hh>
#include <cap/ParserContext.hh>
#include <cap/Client.hh>

#include <random>
#include <array>

using cap::Token;

void addRandomWhitespace(cap::test::DynamicSource& source, bool preferNewlines)
{
	constexpr std::array <wchar_t, 3> whitespace
	{
		' ', '\t', '\n'
	};

	std::random_device device;
	std::mt19937 gen(device());

	std::uniform_int_distribution <size_t> indexDist(0, whitespace.size() - 1);
	std::uniform_int_distribution <size_t> lengthDist(0, 3);

	std::wstring ws(' ', lengthDist(gen));
	for(size_t i = 0; i < ws.length(); i++)
	{
		ws[i] = preferNewlines ? '\n' : whitespace[indexDist(gen)];
	}

	source += std::move(ws);
}

void checkLocation(const cap::SourceLocation& loc, Token::Type type,
		std::wstring_view value, size_t index, size_t length)
{
	ASSERT_EQ(
		std::string_view(loc.getToken().getTypeString()),
		std::string_view(Token::getTypeString(type))
	);

	if(value.length() > 0)
	{
		ASSERT_EQ(loc.getToken().getIndex(), index);
		ASSERT_EQ(loc.getToken().getLength(), length);

		auto result = loc.getString();
		ASSERT_EQ(result, value);
		ASSERT_TRUE(loc.getSource().match(loc.getToken(), value));
	}
}

class TokenTester : public cap::Client
{
public:
	TokenTester() : ctx(*this, source), current(Token::parseFirst(ctx))
	{
	}

	void onSourceError(cap::SourceLocation&, const std::wstring&) override
	{
		//printf("[%u:%u] Error in '%ls': %ls\n", at.getRow(), at.getColumn(), at.getString().c_str(), msg.c_str());
		errors++;
	}

	void expect(Token::Type type, std::wstring_view str, bool shouldError)
	{
		unsigned prevErrors = errors;

		addRandomWhitespace(source, makeWhitespaceNewline);
		size_t beginning = source.getLength();
		source += std::wstring(str);
		addRandomWhitespace(source, makeWhitespaceNewline);

		current = Token::parseNext(ctx, current);

		if(!shouldError)
		{
			cap::SourceLocation location(source, current);
			checkLocation(location, type, str, beginning, str.length());
		}

		if(shouldError)
		{
			ASSERT_EQ(errors, prevErrors + 1);
		}

		else
		{
			ASSERT_EQ(errors, prevErrors);
		}
	}

	unsigned errors = 0;
	cap::test::DynamicSource source;

	Token::ParserContext ctx;
	Token current;

	bool makeWhitespaceNewline = false;
};

TEST(TokenTests, TestIdentifier)
{
	TokenTester test;

	test.expect(Token::Type::Identifier, L"__123", false);
	test.expect(Token::Type::Identifier, L"__abc", false);
	test.expect(Token::Type::Identifier, L"abc", false);
	test.expect(Token::Type::Identifier, L"abc_cba", false);
}

TEST(TokenTests, TestInteger)
{
	TokenTester test;

	test.expect(Token::Type::Integer, L"123", false);
	test.expect(Token::Type::Integer, L"9999999", false);

	test.expect(Token::Type::Integer, L"0", false);
	test.expect(Token::Type::Invalid, L"4567a", true);

	// TODO: Test a scenario when the invalid character is in the middle.
}

TEST(TokenTests, TestFloat)
{
	TokenTester test;

	test.expect(Token::Type::Float, L"123.0", false);
	test.expect(Token::Type::Float, L"9999.99999", false);
	test.expect(Token::Type::Invalid, L"1.0a", true);

	// TODO: Test a scenario when the invalid character is in the middle.
}

TEST(TokenTests, TestHexadecimal)
{
	TokenTester test;

	test.expect(Token::Type::Hexadecimal, L"0xabcdef012345679", false);
	test.expect(Token::Type::Hexadecimal, L"0xABCDEF012345679", false);
	test.expect(Token::Type::Invalid, L"0xefg", true);

	// TODO: Test a scenario when the invalid character is in the middle.
	//test.expect(Token::Type::Invalid, L"0xegf", true);
}

TEST(TokenTests, TestBinary)
{
	TokenTester test;

	test.expect(Token::Type::Binary, L"0b1010", false);
	test.expect(Token::Type::Binary, L"0b0101", false);
	test.expect(Token::Type::Invalid, L"0b01012", true);

	// TODO: Test a scenario when the invalid character is in the middle.
	//test.expect(Token::Type::Invalid, L"0b01201", true);
}

TEST(TokenTests, TestBrackets)
{
	TokenTester test;

	// Test that nested brackets can be opened and closed successfully.
	test.expect(Token::Type::OpeningBracket, L"(", false);
	test.expect(Token::Type::OpeningBracket, L"{", false);
	test.expect(Token::Type::OpeningBracket, L"[", false);
	test.expect(Token::Type::ClosingBracket, L"]", false);
	test.expect(Token::Type::ClosingBracket, L"}", false);
	test.expect(Token::Type::ClosingBracket, L")", false);

	// Test that unopened brackets report an error.
	test.expect(Token::Type::Invalid, L")", true);
	test.expect(Token::Type::Invalid, L"]", true);
	test.expect(Token::Type::Invalid, L"}", true);

	// Test that mismatching brackets report an error.
	test.expect(Token::Type::OpeningBracket, L"(", false);
	test.expect(Token::Type::Invalid, L"]", true);
	test.expect(Token::Type::Invalid, L"}", true);

	// Make sure that the unclosed "(" reports an error.
	// NOTE: By appending an empty string the "end" of the
	// source file is found and it is made sure that all
	// brackets are closed.
	test.expect(Token::Type::Invalid, L"", true);

	// Finally make sure that the opened bracket can be closed
	// and that there are no unterminated brackets.
	test.expect(Token::Type::ClosingBracket, L")", false);
	test.expect(Token::Type::Invalid, L"", false);
}

TEST(TokenTests, TestGenerics)
{
	TokenTester test;

	test.source += L"<a>";

	test.current = Token::parseFirst(test.ctx);
	ASSERT_STREQ(test.current.getTypeString(), "OpeningBracket");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "Identifier");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "ClosingBracket");

	test.source += L"<a, b, c>";

	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "OpeningBracket");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "Identifier");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "Operator");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "Identifier");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "Operator");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "Identifier");
	test.current = Token::parseNext(test.ctx, test.current);
	ASSERT_STREQ(test.current.getTypeString(), "ClosingBracket");
}

TEST(TokenTests, TestComments)
{
	TokenTester test;
	test.makeWhitespaceNewline = true;

	test.expect(Token::Type::Comment, L"// foo", false);
	test.expect(Token::Type::Comment, L"//", false);
	test.expect(Token::Type::Identifier, L"foo", false);
	test.expect(Token::Type::Comment, L"/* foo */", false);
	test.expect(Token::Type::Invalid, L"/* ", true);
}

TEST(TokenTests, TestString)
{
	TokenTester test;

	test.expect(Token::Type::String, L"\"foo\"", false);
	test.expect(Token::Type::String, L"\"\"", false);
	test.expect(Token::Type::String, L"\" \\\" \"", false);
	test.expect(Token::Type::Invalid, L"\"", true);
	test.expect(Token::Type::Invalid, L"\"foo\n\"", true);
	test.expect(Token::Type::Invalid, L"\" \\\\\" \"", true);
}

TEST(TokenTests, TestLineChangeDetection)
{
	TokenTester test;
	test.source += L"a b\nc\n\n\nd e // comment\n\n f  /* not last */ g \n\n\n\n x /";

	test.current = Token::parseFirst(test.ctx); // a
	ASSERT_FALSE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // b
	ASSERT_TRUE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // c
	ASSERT_TRUE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // d
	ASSERT_FALSE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // e
	ASSERT_TRUE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // "comment"
	test.current = Token::parseNext(test.ctx, test.current); // f
	ASSERT_FALSE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // "not last"
	test.current = Token::parseNext(test.ctx, test.current); // g
	ASSERT_TRUE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // x
	ASSERT_FALSE(test.current.isLastOfLine(test.ctx));
	test.current = Token::parseNext(test.ctx, test.current); // /
	ASSERT_TRUE(test.current.isLastOfLine(test.ctx));
}

