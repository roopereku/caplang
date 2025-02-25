#include <gtest/gtest.h>

#include <cap/test/NodeMatcher.hh>
#include <cap/test/DynamicSource.hh>
#include <cap/Client.hh>

using namespace cap::test;

class ParserTester : public cap::Client
{
public:
	ParserTester()
	{
	}

	void test(std::wstring&& str, std::vector <ExpectedNode>&& expected)
	{
		cap::test::DynamicSource source;
		source += std::move(str);

		ASSERT_TRUE(source.parse(*this, false));

		expected.insert(expected.begin(), Scope());
		NodeMatcher matcher(std::move(expected));
		matcher.traverseNode(source.getGlobal());
	}
};

TEST(ParserTests, FunctionDeclaration)
{
	ParserTester tester;

	// Function with no parameters.
	tester.test(L"func foo()\n{\n}",
	{
		Function(L"foo"),
			Scope()
	});

	// Function with parameters.
	tester.test(L"func foo(a = int64, b = uint32)\n{\n}",
	{
		Function(L"foo"),
			cap::Declaration::Root::Type::Parameter,
				cap::BinaryOperator::Type::Comma,
					cap::BinaryOperator::Type::Assign,
						Value(L"a"),
						Value(L"int64"),
					cap::BinaryOperator::Type::Assign,
						Value(L"b"),
						Value(L"uint32"),
			Scope(),
	});

	// Nested functions.
	tester.test(L"func foo()\n{\nfunc bar()\n{\n}\n}\n",
	{
		Function(L"foo"),
			Scope(),
				Function(L"bar"),
					Scope()
	});	

	// Function with an explicit return type
	tester.test(L"func foo() -> SomeType\n{\n}\n",
	{
		Function(L"foo"),
			Value(L"SomeType"),
			Scope()
	});
}

TEST(ParserTests, ClassTypeDeclaration)
{
	ParserTester tester;

	// Test class type without base classes.
	tester.test(L"type foo\n{\n}",
	{
		ClassType(L"foo"),
			Scope()
	});	

	// Test nested class type without base classes.
	tester.test(L"type foo\n{\ntype bar\n{\n}\n}",
	{
		ClassType(L"foo"),
			Scope(),
				ClassType(L"bar"),
					Scope()
	});	

	// Test a generics with initializations.
	tester.test(L"type foo <T1 = int64, T2 = uint32>\n{\n}",
	{
		ClassType(L"foo"),
			cap::Declaration::Root::Type::Generic,
				cap::BinaryOperator::Type::Comma,
					cap::BinaryOperator::Type::Assign,
						Value(L"T1"),
						Value(L"int64"),
					cap::BinaryOperator::Type::Assign,
						Value(L"T2"),
						Value(L"uint32"),
			Scope()
	});
}

TEST(ParserTests, Modifiers)
{
	ParserTester tester;

	tester.test(L"let T = type int64",
	{
		Expression(),
			cap::Declaration::Root::Type::Local,
				cap::BinaryOperator::Type::Assign,
					Value(L"T"),
					cap::ModifierRoot::Type::Alias,
						Value(L"int64"),

	});
}
