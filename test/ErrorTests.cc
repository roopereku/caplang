#include <gtest/gtest.h>

#include <cap/test/DynamicSource.hh>
#include <cap/Client.hh>

#include <queue>

using namespace cap::test;

class ErrorTester : public cap::Client
{
public:
	bool parse(std::wstring&& src)
	{
		DynamicSource source(std::move(src));
		return source.parse(*this);
	}

	void reportsError(std::wstring&& src, const std::wstring& error, bool inGlobalScope = false)
	{
		if(inGlobalScope)
		{
			ASSERT_FALSE(parse(std::move(src)));
		}

		else
		{
			ASSERT_FALSE(parse(L"func test()\n{\n" + std::move(src) + L"\n}"));
		}

		ASSERT_FALSE(errors.empty());
		ASSERT_STREQ(error.c_str(), errors.front().c_str());
		errors.pop();
	}

	void onSourceError(cap::SourceLocation& loc, const std::wstring& msg) override
	{
		// TODO: Store location?
		errors.emplace(msg);
	}

	std::queue <std::wstring> errors;
};

TEST(ErrorTests, ExpectedDeclaration)
{
	ErrorTester tester;

	tester.reportsError(L"a = 10", L"Only declarations are allowed here", true);

	// TODO: Add a check for types?
}

TEST(ErrorTests, UndeclaredIdentifier)
{
	ErrorTester tester;

	tester.reportsError(L"let a = c", L"Undeclared identifier 'c'");
	tester.reportsError(L"let a = 10\nlet b = a1", L"Undeclared identifier 'a1'");
}

TEST(ErrorTests, ExpectedIdentifier)
{
	ErrorTester tester;

	tester.reportsError(L"let 10 = c", L"Expected an identifier before '='");
	tester.reportsError(L"let 0xFF = 10", L"Expected an identifier before '='");
}

TEST(ErrorTests, InvalidDeclaration)
{
	ErrorTester tester;

	// TODO: Add invalid function declarations.
	// TODO: Add invalid type declarations.

	tester.reportsError(L"let a", L"Missing initialization for 'a'. Add '=' after it");
	tester.reportsError(L"let", L"Expected an expression after 'let'");
}

TEST(ErrorTests, DuplicateIdentifier)
{
	ErrorTester tester;

	tester.reportsError(L"let a = 10\nlet a = 20", L"'a' already exists");
	tester.reportsError(L"func foo()\n{\n}\nlet foo = 20", L"'foo' already exists");
	tester.reportsError(L"let foo = 20\nfunc foo()\n{\n}", L"'foo' already exists");

	// TODO: Add a check with types.
}
