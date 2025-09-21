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

		ASSERT_TRUE(source.parse(*this, false));

		expectedExpr.insert(expectedExpr.begin(), base.begin(), base.end());
		NodeMatcher matcher(std::move(expectedExpr));
		matcher.traverseNode(source.getGlobal());
	}

	std::vector <ExpectedNode> base;
};

TEST(ExpressionTests, SingleIdentifier)
{
	ExpressionTester tester;

	tester.test(L"0",
	{
		Expression(),
			Integer(0)
	});

	tester.test(L"foo",
	{
		Expression(),
			Identifier(L"foo")
	});

}

TEST(ExpressionTests, BinaryOperators)
{
	ExpressionTester tester;

	tester.test(L"1+2",
	{
		Expression(),
			cap::BinaryOperator::Type::Add,
				Integer(1),
				Integer(2)
	});

	tester.test(L"1+2*3",
	{
		Expression(),
			cap::BinaryOperator::Type::Add,
				Integer(1),
				cap::BinaryOperator::Type::Multiply,
					Integer(2),
					Integer(3)
	});
}

TEST(ExpressionTests, UnaryOperators)
{
	ExpressionTester tester;

	tester.test(L"-a",
	{
		Expression(),
			cap::UnaryOperator::Type::Negate,
				Identifier(L"a")
	});

	tester.test(L"~(1)",
	{
		Expression(),
			cap::UnaryOperator::Type::BitwiseNot,
				Expression(),
					Integer(1)
	});

	tester.test(L"!(1)++",
	{
		Expression(),
			cap::UnaryOperator::Type::LogicalNot,
				cap::UnaryOperator::Type::PostIncrement,
					Expression(),
						Integer(1)
	});

	tester.test(L"-10 * 80++ - 100",
	{
		Expression(),
			cap::BinaryOperator::Type::Subtract,
				cap::BinaryOperator::Type::Multiply,
					cap::UnaryOperator::Type::Negate,
							Integer(10),
					cap::UnaryOperator::Type::PostIncrement,
							Integer(80),
				Integer(100)
	});

	tester.test(L"- ++10-- / ~ ~(0xFF--)++",
	{
		Expression(),
			cap::BinaryOperator::Type::Divide,
				cap::UnaryOperator::Type::Negate,
					cap::UnaryOperator::Type::PreIncrement,
						cap::UnaryOperator::Type::PostDecrement,
							Integer(10),
				cap::UnaryOperator::Type::BitwiseNot,
					cap::UnaryOperator::Type::BitwiseNot,
						cap::UnaryOperator::Type::PostIncrement,
							Expression(),
								cap::UnaryOperator::Type::PostDecrement,
									Integer(0xFF)
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
									Integer(1),
								cap::UnaryOperator::Type::PreDecrement,
									Integer(0xFF),
	});

	tester.test(L"a++ * 20",
	{
		Expression(),
			cap::BinaryOperator::Type::Multiply,
				cap::UnaryOperator::Type::PostIncrement,
					Identifier(L"a"),
				Integer(20)
	});

	tester.test(L"* ~foo()++",
	{
		Expression(),
			cap::UnaryOperator::Type::ParseTime,
				cap::UnaryOperator::Type::BitwiseNot,
					cap::UnaryOperator::Type::PostIncrement,
						cap::BracketOperator::Type::Call,
							Identifier(L"foo"),
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
				Identifier(L"foo"),
				Expression(),

		Expression(),
			cap::BracketOperator::Type::Call,
				Identifier(L"foo"),
				Expression(),
					Integer(1),

		Expression(),
			cap::BracketOperator::Type::Call,
				Identifier(L"foo"),
				Expression(),
					cap::BinaryOperator::Type::Add,
					Integer(1),
					Integer(2)
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
				Identifier(L"foo"),
				Expression(),

		Expression(),
			cap::BracketOperator::Type::Subscript,
				Identifier(L"foo"),
				Expression(),
					Integer(1),

		Expression(),
			cap::BracketOperator::Type::Subscript,
				Identifier(L"foo"),
				Expression(),
					cap::BinaryOperator::Type::Add,
					Integer(1),
					Integer(2)
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
								Identifier(L"foo"),
								Expression(),
							Expression(),
								cap::BinaryOperator::Type::Add,
									cap::BinaryOperator::Type::Exponent,
										Integer(6),
										Identifier(L"exp"),
									Expression(),
										cap::BinaryOperator::Type::Subtract,
											Integer(1),
											Identifier(L"a"),
						Expression(),
					Expression(),
						Integer(20),
				Expression(),
	});

	// Test generic operators.
	tester.test(L"foo <10, 20> * (a<50> - getType() <int> ().value)",
	{
		Expression(),
			cap::BinaryOperator::Type::Multiply,
				cap::BracketOperator::Type::Generic,
					Identifier(L"foo"),
					Expression(),
						cap::BinaryOperator::Type::Comma,
							Integer(10),
							Integer(20),
				Expression(),
					cap::BinaryOperator::Type::Subtract,
						cap::BracketOperator::Type::Generic,
							Identifier(L"a"),
							Expression(),
								Integer(50),
						cap::BinaryOperator::Type::Access,
							cap::BracketOperator::Type::Call,
								cap::BracketOperator::Type::Generic,
									cap::BracketOperator::Type::Call,
										Identifier(L"getType"),
										Expression(),
									Expression(),
										Identifier(L"int"),
								Expression(),
							Identifier(L"value"),
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
					Integer(5),
					Integer(10),
			Integer(20),

		Expression(),
			cap::BinaryOperator::Type::Equal,
				cap::BinaryOperator::Type::Greater,
					cap::BinaryOperator::Type::Divide,
						Identifier(L"a"),
						Identifier(L"b"),
					Integer(10),
				Identifier(L"true"),

		Expression(),
			cap::BinaryOperator::Type::Assign,
				Identifier(L"x"),
				cap::UnaryOperator::Type::PostIncrement,
					Identifier(L"a"),

		Expression(),
			cap::BinaryOperator::Type::Assign,
				Identifier(L"x"),
				cap::UnaryOperator::Type::PostDecrement,
					Identifier(L"b"),

		Expression(),
			cap::BinaryOperator::Type::Assign,
				Identifier(L"c"),
				cap::BinaryOperator::Type::Multiply,
					Identifier(L"a"),
					cap::BinaryOperator::Type::Exponent,
						Identifier(L"b"),
						Integer(10),
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
						Integer(1),
						cap::BinaryOperator::Type::Multiply,
						Integer(2),
						Expression(),
							cap::BinaryOperator::Type::Subtract,
								Integer(3),
								Integer(4),
					Expression(),
						cap::BinaryOperator::Type::Exponent,
							Integer(10),
							Expression(),
								cap::BinaryOperator::Type::Inequal,
									Integer(5),
									Integer(2),
				Expression(),
					cap::BinaryOperator::Type::BitwiseShiftRight,
						cap::BinaryOperator::Type::Modulus,
							Integer(100),
							Integer(20),
						Integer(12),
		Expression(),
			cap::BinaryOperator::Type::Add,
				cap::BinaryOperator::Type::Multiply,
					Integer(70),
					Integer(80),
				cap::BinaryOperator::Type::Divide,
					Integer(50),
					Integer(23)
	});
}

TEST(ExpressionTests, Declarations)
{
	ExpressionTester tester;

	tester.test(L"let a = 1 + 2",
	{
		LocalVariable(L"a"),
			cap::BinaryOperator::Type::Add,
				Integer(1),
				Integer(2)
	});
}

TEST(ExpressionTests, TypeReferenceAcceptsSingleValue)
{
	ExpressionTester tester;

	tester.test(L"let a = type int64 + 2",
	{
		LocalVariable(L"a"),
			cap::BinaryOperator::Type::Add,
				TypeReference(),
					Identifier(L"int64"),
				Integer(2)
	});

	tester.test(L"let a = type -int64",
	{
		LocalVariable(L"a"),
			TypeReference(),
				cap::UnaryOperator::Type::Negate,
					Identifier(L"int64")
	});

	tester.test(L"let a = type (int64 + int32)",
	{
		LocalVariable(L"a"),
			TypeReference(),
				Expression(),
					cap::BinaryOperator::Type::Add,
						Identifier(L"int64"),
						Identifier(L"int32")
	});

	tester.test(L"let a = type *(int64)",
	{
		LocalVariable(L"a"),
			TypeReference(),
				cap::UnaryOperator::Type::ParseTime,
					Expression(),
						Identifier(L"int64")
	});

	tester.test(L"let a = type *getTypeDynamically()",
	{
		LocalVariable(L"a"),
			TypeReference(),
				cap::UnaryOperator::Type::ParseTime,
					cap::BracketOperator::Type::Call,
						Identifier(L"getTypeDynamically"),
						Expression()
	});

	tester.test(L"let a = type typeLookup[0]",
	{
		LocalVariable(L"a"),
			TypeReference(),
				cap::BracketOperator::Type::Subscript,
					Identifier(L"typeLookup"),
					Expression(),
						Integer(0)
	});
}

TEST(ExpressionTests, ExpressionAttributesDisconnected)
{
	ExpressionTester tester;

	tester.test(L"@debug something",
	{
		Expression(),
			Identifier(L"something")
	});

	tester.test(L"@debug @otherAttr(1 + 2) something",
	{
		Expression(),
			Identifier(L"something")
	});

	tester.test(L"@*dynamicAttr something\n@debug print(\"test\")",
	{
		Expression(),
			Identifier(L"something"),

		Expression(),
			cap::BracketOperator::Type::Call,	
				Identifier(L"print"),
				Expression(),
					String(L"test")
	});
}

TEST(ExpressionTests, AttributesApplicableToBracketValues)
{
	// TODO: @attr (1 + 2)
	// TODO: @attr [1 + 2]
}
