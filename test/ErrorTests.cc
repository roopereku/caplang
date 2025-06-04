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
		SCOPED_TRACE(src.c_str());

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

	void onSourceError(cap::SourceLocation&, const std::wstring& msg) override
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
	tester.reportsError(L"type T\n{\na = 10\n}", L"Only declarations are allowed here", true);
}

TEST(ErrorTests, UndeclaredIdentifier)
{
	ErrorTester tester;

	tester.reportsError(L"let a = c", L"Undeclared identifier 'c'");
	tester.reportsError(L"let a = 10\nlet b = a1", L"Undeclared identifier 'a1'");
	tester.reportsError(L"func foo() -> abc\n{\n}\n", L"Undeclared identifier 'abc'");
}

TEST(ErrorTests, ExpectedIdentifier)
{
	ErrorTester tester;

	tester.reportsError(L"let 10 = c", L"Expected an identifier");
	tester.reportsError(L"let 0xFF = 10", L"Expected an identifier");
	tester.reportsError(L"let *a = 10", L"Expected an identifier");
}

TEST(ErrorTests, InvalidDeclaration)
{
	ErrorTester tester;

	// TODO: Have tests with and without encapsulating function "test".

	tester.reportsError(L"func", L"Expected an identifier after 'func'");
	tester.reportsError(L"func foo", L"Expected '(' after function name");
	tester.reportsError(L"func foo()", L"Expected '{'");

	tester.reportsError(L"type", L"Expected an identifier after 'type'");
	tester.reportsError(L"type foo", L"Expected '{'");
	tester.reportsError(L"type foo <> {\n}\n", L"Expected a generic");

	tester.reportsError(L"let a", L"Expected '='");
	tester.reportsError(L"let", L"Expected a variable");

	// TODO: Should this same logic apply to functions and types where
	// the name has to be on the same line as the declarator.
	tester.reportsError(L"let\na = 10", L"Expected a variable");
}

TEST(ErrorTests, DuplicateIdentifier)
{
	ErrorTester tester;

	tester.reportsError(L"let a = 10\nlet a = 20", L"'a' already exists");

	tester.reportsError(L"func foo()\n{\n}\nlet foo = 20", L"'foo' already exists");
	tester.reportsError(L"let foo = 20\nfunc foo()\n{\n}", L"'foo' already exists");

	tester.reportsError(L"type foo\n{\n}\nlet foo = 20", L"'foo' already exists");
	tester.reportsError(L"let foo = 20\ntype foo\n{\n}", L"'foo' already exists");

	tester.reportsError(L"let int8 = 10\n", L"'int8' already exists");
	tester.reportsError(L"let int32 = 10\n", L"'int32' already exists");
	tester.reportsError(L"let uint16 = 10\n", L"'uint16' already exists");
	tester.reportsError(L"let uint64 = 10\n", L"'uint64' already exists");
}

TEST(ErrorTests, FunctionParameters)
{
	ErrorTester tester;

	// Test that duplicate function names with duplicate parameters are forbidden.
	tester.reportsError(LR"SRC(
		func foo(a = int64, b = string)
		{
		}

		func foo(a = int64, b = string)
		{
		}
	)SRC", L"Function with the same parameters already exists");

	auto testCall = [](std::wstring&& str) -> std::wstring
	{
		std::wstring funcs = LR"SRC(
			func foo(a = string, b = int64) -> int64
			{
			}

			func foo(a = int64) -> int64
			{
			}
		)SRC";

		return funcs + L"func main()\n{\n" + std::move(str) + L"\n}\n";
	};

	// Test that empty parameters fail.
	tester.reportsError(testCall(L"foo()"), L"No matching overload found for 'foo'");

	// Test that mismatching parameters fail.
	tester.reportsError(testCall(L"foo(\"test\")"), L"No matching overload found for 'foo'");
	tester.reportsError(testCall(L"foo(10, \"test\")"), L"No matching overload found for 'foo'");
	//tester.reportsError(testCall(L"foo(10, \"test\", 20, 50)"), L"No matching overload found for 'foo'");
}

TEST(ErrorTests, UnexpectedReturn)
{
	ErrorTester tester;

	tester.reportsError(LR"SRC(
		return 0
	)SRC", L"Cannot return here", true);

	tester.reportsError(LR"SRC(
		type Foo
		{
			return 0
		}
	)SRC", L"Cannot return here", true);
}

TEST(ErrorTests, MismatchingReturnType)
{
	ErrorTester tester;

	tester.reportsError(LR"SRC(
		func foo() -> int64
		{
			return "someString"
		}
	)SRC", L"Incompatible return type", true);

	tester.reportsError(LR"SRC(
		func foo()
		{
			return "someString"
			return 10
		}
	)SRC", L"Incompatible return type", true);

	tester.reportsError(LR"SRC(
		func foo()
		{
			return
			return 10
		}
	)SRC", L"Incompatible return type", true);

	tester.reportsError(LR"SRC(
		func foo() -> void
		{
			return 10
		}
	)SRC", L"Incompatible return type", true);

}
