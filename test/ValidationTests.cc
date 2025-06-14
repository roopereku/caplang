#include <gtest/gtest.h>

#include <cap/test/DynamicSource.hh>
#include <cap/test/NodeMatcher.hh>
#include <cap/Function.hh>
#include <cap/Client.hh>

using namespace cap::test;

class ValidationTester : public cap::Client
{
public:
	void setup(std::wstring&& src)
	{
		setupSrc = std::move(src);
	}

	void test(std::wstring&& src, std::vector <ExpectedNode>&& expectedExpr)
	{
		cap::test::DynamicSource source;
		source += setupSrc;
		source += L"\nfunc test()\n{\n";
		source += std::move(src);
		source += L"\n}\n";

		ASSERT_TRUE(source.parse(*this, true));

		expectedExpr.insert(expectedExpr.begin(), Scope());
		NodeMatcher matcher(std::move(expectedExpr));

		std::shared_ptr <cap::Scope> root;
		for(auto decl : source.getGlobal()->declarations)
		{
			if(decl->getName() == L"test")
			{
				ASSERT_TRUE(decl->getType() == cap::Declaration::Type::Function);
				auto func = std::static_pointer_cast <cap::Function> (decl);
				root = func->getBody();
			}
		}

		matcher.traverseNode(root);
	}

private:
	std::wstring setupSrc;
};

TEST(ValidationTests, AccessOperatorResultType)
{
	ValidationTester tester;

	tester.setup(LR"SRC(
		type Foo
		{
			type Baz
			{
				let a = Bar.a * 10
			}

			type Bar
			{
				let a = 10

				func nested(x = int64, y = string)
				{
					return "test"
				}
			}
		}
	)SRC");

	tester.test(L"let a = Foo.Bar.a",
	{
		LocalVariable(L"a") > L"int64",
		cap::BinaryOperator::Type::Access > L"int64",
			cap::BinaryOperator::Type::Access > L"Foo.Bar",
				Identifier(L"Foo") > L"Foo",
				Identifier(L"Bar") > L"Foo.Bar",
			Identifier(L"a", L"Foo.Bar.a") > L"int64"
	});

	tester.test(L"let b = Foo.Bar.nested",
	{
		LocalVariable(L"b") > L"func(int64, int64) -> string",
		cap::BinaryOperator::Type::Access > L"func(int64, string) -> string",
			cap::BinaryOperator::Type::Access > L"Foo.Bar",
				Identifier(L"Foo") > L"Foo",
				Identifier(L"Bar") > L"Foo.Bar",
			Identifier(L"nested", L"Foo.Bar.nested") > L"func(int64, string) -> string"
	});

	tester.test(L"let b = Foo.Bar.nested(1, \"test\")",
	{
		LocalVariable(L"b") > L"string",
			cap::BinaryOperator::Type::Access > L"string",
				cap::BinaryOperator::Type::Access > L"Foo.Bar",
					Identifier(L"Foo") > L"Foo",
					Identifier(L"Bar") > L"Foo.Bar",
			cap::BracketOperator::Type::Call > L"string",
				Identifier(L"nested", L"Foo.Bar.nested") > L"func(int64, string) -> string",
				Expression(),
					cap::BinaryOperator::Type::Comma,
						Integer(1) > L"int64",
						String(L"test") > L"string"
	});
}

TEST(ValidationTests, InferredReturnType)
{
	ValidationTester tester;

	tester.setup(LR"SRC(
		func returnInt()
		{
			return 10
		}

		func returnString()
		{
			return "test"
		}

		func returnVoid()
		{
			return
		}

		func returnDefault()
		{
		}
	)SRC");

	tester.test(L"let a = returnInt()",
	{
		LocalVariable(L"a"),
			cap::BracketOperator::Type::Call > L"int64",
				Identifier(L"returnInt") > L"func() -> int64",
				Expression()
	});

	tester.test(L"let b = returnString()",
	{
		LocalVariable(L"b"),
			cap::BracketOperator::Type::Call > L"string",
				Identifier(L"returnString") > L"func() -> string",
				Expression()
	});

	tester.test(L"returnVoid()",
	{
		Expression(),
			cap::BracketOperator::Type::Call > L"void",
				Identifier(L"returnVoid") > L"func() -> void",
				Expression()
	});

	tester.test(L"returnDefault()",
	{
		Expression(),
			cap::BracketOperator::Type::Call > L"void",
				Identifier(L"returnDefault") > L"func() -> void",
				Expression()
	});
}

TEST(ValidationTests, VariablesAsParameters)
{
	ValidationTester tester;

	tester.setup(LR"SRC(
		func foo(a = string, b = int64, c = int64)
		{
		}

		type Foo
		{
			func foo(a = string, b = int64)
			{
			}
		}

		let x = 10
		let y = 200
		let str = "some string"

	)SRC");

	tester.test(L"foo(str, x, y)",
	{
		Expression() > L"void",
			cap::BracketOperator::Type::Call > L"void",
				Identifier(L"foo") > L"func(string, int64, int64) -> void",
				Expression(),
					cap::BinaryOperator::Type::Comma,
						cap::BinaryOperator::Type::Comma,
							Identifier(L"str") > L"string",
							Identifier(L"x") > L"int64",
						Identifier(L"y") > L"int64"
	});

	tester.test(L"Foo.foo(str, x)",
	{
		Expression() > L"void",
			cap::BinaryOperator::Type::Access > L"void",
				Identifier(L"Foo") > L"Foo",
				cap::BracketOperator::Type::Call > L"void",
					Identifier(L"foo") > L"func(string, int64) -> void",
					Expression(),
						cap::BinaryOperator::Type::Comma,
							Identifier(L"str") > L"string",
							Identifier(L"x") > L"int64",
	});

	// TODO: Add a test where function parameters are passed onwards?
}
