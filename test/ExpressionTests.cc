#include <gtest/gtest.h>

#include <cap/test/NodeMatcher.hh>
#include <cap/test/DynamicSource.hh>
#include <cap/Client.hh>

using namespace cap::test;

class ExpressionTester : public cap::Client
{
public:
	ExpressionTester()
		: base({ Scope(), Function(L"test"), Scope() })
	{
	}

	void test(std::wstring&& expr, std::vector <ExpectedNode>&& expectedExpr)
	{
		std::wstring context = L"Testing expression: " + expr;
		SCOPED_TRACE(context.c_str());

		cap::test::DynamicSource source;
		source += L"func test()\n{\n";
		source += std::move(expr);
		source += L"\n}\n";

		// TODO: When validation exists, disable it
		// as a lot of the tests only make sure that the
		// parsing phase works.
		ASSERT_TRUE(source.parse(*this));

		expectedExpr.insert(expectedExpr.begin(), base.begin(), base.end());
		NodeMatcher matcher(std::move(expectedExpr));
		matcher.traverseNode(source.getGlobal());
	}

	std::vector <ExpectedNode> base;
};

TEST(ExpressionTests, BinaryOperators)
{
	ExpressionTester tester;

	tester.test(L"1+2",
	{
		Expression(),
			cap::BinaryOperator::Type::Add,
				Value(L"1"),
				Value(L"2")
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

TEST(ExpressionTests, BracketOperators)
{
	ExpressionTester tester;

	// Test call operators.
	tester.test(LR"SRC(
		foo()
		foo(1)
		foo(1 + 2)
	)SRC",
	{
		Expression(),
			cap::BracketOperator::Type::Call,
				Value(L"foo"),
				Expression(),

		Expression(),
			cap::BracketOperator::Type::Call,
				Value(L"foo"),
				Expression(),
					Value(L"1"),

		Expression(),
			cap::BracketOperator::Type::Call,
				Value(L"foo"),
				Expression(),
					cap::BinaryOperator::Type::Add,
					Value(L"1"),
					Value(L"2")
	});

	// Test subscript operators.
	tester.test(LR"SRC(
		foo[]
		foo[1]
		foo[1 + 2]
	)SRC",
	{
		Expression(),
			cap::BracketOperator::Type::Subscript,
				Value(L"foo"),
				Expression(),

		Expression(),
			cap::BracketOperator::Type::Subscript,
				Value(L"foo"),
				Expression(),
					Value(L"1"),

		Expression(),
			cap::BracketOperator::Type::Subscript,
				Value(L"foo"),
				Expression(),
					cap::BinaryOperator::Type::Add,
					Value(L"1"),
					Value(L"2")
	});

	// Test chained bracket operators.
	tester.test(L"foo()[6 ** exp + (1 - a)]()(20)[]",
	{
		Expression(),
			cap::BracketOperator::Type::Subscript,
				cap::BracketOperator::Type::Call,
					cap::BracketOperator::Type::Call,
						cap::BracketOperator::Type::Subscript,
							cap::BracketOperator::Type::Call,
								Value(L"foo"),
								Expression(),
							Expression(),
								cap::BinaryOperator::Type::Add,
									cap::BinaryOperator::Type::Exponent,
										Value(L"6"),
										Value(L"exp"),
									Expression(),
										cap::BinaryOperator::Type::Subtract,
											Value(L"1"),
											Value(L"a"),
						Expression(),
					Expression(),
						Value(L"20"),
				Expression(),
	});
}

TEST(ExpressionTests, ExpressionEnds)
{
	ExpressionTester tester;

	// Test that expressions end at a newline.
	tester.test(LR"SRC(
		5 * 10 + 20
		a / b > 10 == true
		c = a * b**10
	)SRC",
	{
		Expression(),
			cap::BinaryOperator::Type::Add,
				cap::BinaryOperator::Type::Multiply,
					Value(L"5"),
					Value(L"10"),
			Value(L"20"),

		Expression(),
			cap::BinaryOperator::Type::Equal,
				cap::BinaryOperator::Type::Greater,
					cap::BinaryOperator::Type::Divide,
						Value(L"a"),
						Value(L"b"),
					Value(L"10"),
				Value(L"true"),

		Expression(),
			cap::BinaryOperator::Type::Assign,
				Value(L"c"),
				cap::BinaryOperator::Type::Multiply,
					Value(L"a"),
					cap::BinaryOperator::Type::Exponent,
						Value(L"b"),
						Value(L"10"),
	});

	// Test that subexpressions can span over multiple lines.
	tester.test(LR"SRC(
		1 + 2 * (3 - 4) << (
			10
			** (5
			!= 2
			)
				) ^ (
			
					100 %
					20
			>>
		12
		)
		70 * 80 + 50 / 23
	)SRC",
	{
		Expression(),
			cap::BinaryOperator::Type::BitwiseXor,
				cap::BinaryOperator::Type::BitwiseShiftLeft,
					cap::BinaryOperator::Type::Add,
						Value(L"1"),
						cap::BinaryOperator::Type::Multiply,
						Value(L"2"),
						Expression(),
							cap::BinaryOperator::Type::Subtract,
								Value(L"3"),
								Value(L"4"),
					Expression(),
						cap::BinaryOperator::Type::Exponent,
							Value(L"10"),
							Expression(),
								cap::BinaryOperator::Type::Inequal,
									Value(L"5"),
									Value(L"2"),
				Expression(),
					cap::BinaryOperator::Type::BitwiseShiftRight,
						cap::BinaryOperator::Type::Modulus,
							Value(L"100"),
							Value(L"20"),
						Value(L"12"),
		Expression(),
			cap::BinaryOperator::Type::Add,
				cap::BinaryOperator::Type::Multiply,
					Value(L"70"),
					Value(L"80"),
				cap::BinaryOperator::Type::Divide,
					Value(L"50"),
					Value(L"23")
	});
}

TEST(ExpressionTests, Declarations)
{
	ExpressionTester tester;

	tester.test(L"let a = 1 + 2",
	{
		Expression(),
			DeclarationRoot(),
				cap::BinaryOperator::Type::Assign,
					Value(L"a"),
					cap::BinaryOperator::Type::Add,
						Value(L"1"),
						Value(L"2"),
	});
}
