#include <gtest/gtest.h>

#include <cap/test/NodeMatcher.hh>
#include <cap/test/DynamicSource.hh>
#include <cap/Client.hh>

using namespace cap::test;

class ExpressionTester : public cap::Client
{
public:
	ExpressionTester()
		: base({ Scope(), Function(L"test"), })
	{
	}

	void test(std::wstring&& expr, std::vector <ExpectedNode>&& expectedExpr)
	{
		cap::test::DynamicSource source;
		source += L"func test()\n{\n";
		source += std::move(expr);
		source += L"\n}\n";

		ASSERT_TRUE(source.parse(*this));

		expectedExpr.insert(expectedExpr.begin(), base.begin(), base.end());
		NodeMatcher matcher(std::move(expectedExpr));
		matcher.traverseNode(source.getGlobal());
	}

	std::vector <ExpectedNode> base;
};

TEST(ExpressionTests, SimpleBinaryOperators)
{
	ExpressionTester tester;

	tester.test(L"1+2",
	{
		Expression(),
			cap::BinaryOperator::Type::Add,
				Value(L"1"), Value(L"2")
	});

	tester.test(L"1+2*3",
	{
		Expression(),
			cap::BinaryOperator::Type::Add,
				Value(L"1"),
				cap::BinaryOperator::Type::Multiply,
					Value(L"2"),
					Value(L"3")
	});
}

TEST(ExpressionTests, MultipleExpressions)
{
	ExpressionTester tester;

	tester.test(L"5 * 10 + 20\na / b > 10",
	{
		Expression(),
			cap::BinaryOperator::Type::Add,
				cap::BinaryOperator::Type::Multiply,
					Value(L"5"),
					Value(L"10"),
			Value(L"20"),

		Expression(),
			cap::BinaryOperator::Type::Greater,
				cap::BinaryOperator::Type::Divide,
					Value(L"a"),
					Value(L"b"),
				Value(L"10"),
	});
}
