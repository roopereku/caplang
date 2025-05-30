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
		ASSERT_TRUE(source.parse(*this, false));

		expectedExpr.insert(expectedExpr.begin(), base.begin(), base.end());
		NodeMatcher matcher(std::move(expectedExpr));
		matcher.traverseNode(source.getGlobal());
	}

	std::vector <ExpectedNode> base;
};

TEST(ExpressionTests, SingleValue)
{
	ExpressionTester tester;

	tester.test(L"0",
	{
		Expression(),
			Value(L"0")
	});

	tester.test(L"foo",
	{
		Expression(),
			Value(L"foo")
	});

}

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

TEST(ExpressionTests, UnaryOperators)
{
	ExpressionTester tester;

	tester.test(L"-a",
	{
		Expression(),
			cap::UnaryOperator::Type::Negate,
				Value(L"a")
	});

	tester.test(L"~(1)",
	{
		Expression(),
			cap::UnaryOperator::Type::BitwiseNot,
				Expression(),
					Value(L"1")
	});

	tester.test(L"!(1)++",
	{
		Expression(),
			cap::UnaryOperator::Type::LogicalNot,
				cap::UnaryOperator::Type::PostIncrement,
					Expression(),
						Value(L"1")
	});

	tester.test(L"-10 * 80++ - 100",
	{
		Expression(),
			cap::BinaryOperator::Type::Subtract,
				cap::BinaryOperator::Type::Multiply,
					cap::UnaryOperator::Type::Negate,
							Value(L"10"),
					cap::UnaryOperator::Type::PostIncrement,
							Value(L"80"),
				Value(L"100")
	});

	tester.test(L"- ++10-- / ~ ~(0xFF--)++",
	{
		Expression(),
			cap::BinaryOperator::Type::Divide,
				cap::UnaryOperator::Type::Negate,
					cap::UnaryOperator::Type::PreIncrement,
						cap::UnaryOperator::Type::PostDecrement,
							Value(L"10"),
				cap::UnaryOperator::Type::BitwiseNot,
					cap::UnaryOperator::Type::BitwiseNot,
						cap::UnaryOperator::Type::PostIncrement,
							Expression(),
								cap::UnaryOperator::Type::PostDecrement,
									Value(L"0xFF")
	});

	tester.test(L"-(*(~1 << --0xFF))",
	{
		Expression(),
			cap::UnaryOperator::Type::Negate,
				Expression(),
					cap::UnaryOperator::Type::ParseTime,
						Expression(),
							cap::BinaryOperator::Type::BitwiseShiftLeft,
								cap::UnaryOperator::Type::BitwiseNot,
									Value(L"1"),
								cap::UnaryOperator::Type::PreDecrement,
									Value(L"0xFF"),
	});

	tester.test(L"a++ * 20",
	{
		Expression(),
			cap::BinaryOperator::Type::Multiply,
				cap::UnaryOperator::Type::PostIncrement,
					Value(L"a"),
				Value(L"20")
	});

	tester.test(L"* ~foo()++",
	{
		Expression(),
			cap::UnaryOperator::Type::ParseTime,
				cap::UnaryOperator::Type::BitwiseNot,
					cap::UnaryOperator::Type::PostIncrement,
						cap::BracketOperator::Type::Call,
							Value(L"foo"),
							Expression()
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

	// Test generic operators.
	tester.test(L"foo <10, 20> * (a<50> - getType() <int> ().value)",
	{
		Expression(),
			cap::BinaryOperator::Type::Multiply,
				cap::BracketOperator::Type::Generic,
					Value(L"foo"),
					Expression(),
						cap::BinaryOperator::Type::Comma,
							Value(L"10"),
							Value(L"20"),
				Expression(),
					cap::BinaryOperator::Type::Subtract,
						cap::BracketOperator::Type::Generic,
							Value(L"a"),
							Expression(),
								Value(L"50"),
						cap::BinaryOperator::Type::Access,
							cap::BracketOperator::Type::Call,
								cap::BracketOperator::Type::Generic,
									cap::BracketOperator::Type::Call,
										Value(L"getType"),
										Expression(),
									Expression(),
										Value(L"int"),
								Expression(),
							Value(L"value"),
	});
}

TEST(ExpressionTests, ExpressionEnds)
{
	ExpressionTester tester;

	// Test that expressions end at the last non-comment token.
	tester.test(LR"SRC(
		5 * 10 + /* :-) */ 20 /* Ends

		here */

		a / b > 10 == true   // Comment has no effect
		x = a++
		x = b-- // No effect from comment
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
				Value(L"x"),
				cap::UnaryOperator::Type::PostIncrement,
					Value(L"a"),

		Expression(),
			cap::BinaryOperator::Type::Assign,
				Value(L"x"),
				cap::UnaryOperator::Type::PostDecrement,
					Value(L"b"),

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
		LocalVariable(L"a"),
			cap::BinaryOperator::Type::Add,
				Value(L"1"),
				Value(L"2"),
	});
}
