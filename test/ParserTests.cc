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
			ParameterRoot(),
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
}
