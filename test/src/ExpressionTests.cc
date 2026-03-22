#include <cap/Client.hh>
#include <cap/Scope.hh>
#include <cap/test/CapTest.hh>
#include <cap/test/DynamicSource.hh>
#include <cap/test/NodeMatcher.hh>

using namespace cap::test;

CAP_TEST(Error, IntegerTooLarge64Hexadecimal)
{
    test.reportsError(L"0xFFFFFFFFFFFFFFFF1", L"Integer too large to fit inside 64 bits");
}

CAP_TEST(Error, IntegerTooLarge64Binary)
{
    test.reportsError(L"0b11111111111111111111111111111111111111111111111111111111111111111",
                      L"Integer too large to fit inside 64 bits");
}

CAP_TEST(Error, IntegerTooLarge64Decimal)
{
    test.reportsError(L"18446744073709551616", L"Integer too large to fit inside 64 bits");
}

CAP_TEST(Error, ConsecutiveValuesFail1)
{
    test.reportsError(L"10 20", L"Consecutive values are not allowed");
}

CAP_TEST(Error, ConsecutiveValuesFail2)
{
    test.reportsError(L"a b", L"Consecutive values are not allowed");
}

CAP_TEST(Error, ConsecutiveValuesFail3)
{
    test.reportsError(L"10 b", L"Consecutive values are not allowed");
}

CAP_TEST(Error, ConsecutiveValuesFail4)
{
    test.reportsError(L"a 20", L"Consecutive values are not allowed");
}

CAP_TEST(Error, ConsecutiveValuesFail5)
{
    test.reportsError(L"a foo()", L"Consecutive values are not allowed");
}

CAP_TEST(Error, ConsecutiveValuesFail6)
{
    test.reportsError(L"bar(1 * 2 + 3) foo()", L"Consecutive values are not allowed");
}

CAP_TEST(Error, ConsecutiveValuesFail7)
{
    test.reportsError(L"bar(1 * 2 + 3) 20", L"Consecutive values are not allowed");
}

CAP_TEST(Error, ConsecutiveValuesFail8)
{
    test.reportsError(L"(1 * 2 + 3) foo", L"Consecutive values are not allowed");
}

// clang-format off

CAP_TEST(PreValidation, SingleInteger)
{
    test.enclosedMatches(L"0",
    {
        Expression(),
            Integer(0)
    });
}

CAP_TEST(PreValidation, SingleIdentifier)
{
    test.enclosedMatches(L"foo",
    {
        Expression(),
            Identifier(L"foo")
    });
}

CAP_TEST(PreValidation, AdditionWithIntegers)
{
    test.enclosedMatches(L"1+2",
    {
        Expression(),
            cap::BinaryOperator::Type::Add,
                Integer(1),
                Integer(2)
    });
}

CAP_TEST(PreValidation, AdditionAndMultiplicationWithIntegers)
{
    test.enclosedMatches(L"1+2*3",
    {
        Expression(),
            cap::BinaryOperator::Type::Add,
                Integer(1),
                cap::BinaryOperator::Type::Multiply,
                    Integer(2),
                    Integer(3)
    });
}

CAP_TEST(PreValidation, UnaryNegateWithIdentifier)
{
    test.enclosedMatches(L"-a",
    {
        Expression(),
            cap::UnaryOperator::Type::Negate,
                Identifier(L"a")
    });
}

CAP_TEST(PreValidation, UnaryNOTWithIdentifierInParentheses)
{
    test.enclosedMatches(L"~(1)",
    {
        Expression(),
            cap::UnaryOperator::Type::BitwiseNot,
                Expression(),
                    Integer(1)
    });
}

CAP_TEST(PreValidation, EnclosedIdentifierSurroundedWithPreAndPostUnaryOperators)
{
    test.enclosedMatches(L"!(1)++",
    {
        Expression(),
            cap::UnaryOperator::Type::LogicalNot,
                cap::UnaryOperator::Type::PostIncrement,
                    Expression(),
                        Integer(1)
    });
}

CAP_TEST(PreValidation, CombinedUnaryAndBinaryOperators1)
{
    test.enclosedMatches(L"-10 * 80++ - 100",
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
}

CAP_TEST(PreValidation, CombinedUnaryAndBinaryOperators2)
{
    test.enclosedMatches(L"- ++10-- / ~ ~(0xFF--)++",
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
}

CAP_TEST(PreValidation, UnaryOperatorsAppliedAtDifferentBracketDepths)
{
    test.enclosedMatches(L"-(*(~1 << --0xFF))",
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
}

CAP_TEST(PreValidation, PostUnaryOperatorBeforeBinaryOperator)
{
    test.enclosedMatches(L"a++ * 20",
    {
        Expression(),
            cap::BinaryOperator::Type::Multiply,
                cap::UnaryOperator::Type::PostIncrement,
                    Identifier(L"a"),
                Integer(20)
    });
}

CAP_TEST(PreValidation, UnaryOperatorPrefixedWithUnaryOperatorAndResultPostfixedWithUnaryOperator)
{
    test.enclosedMatches(L"* ~foo()++",
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

CAP_TEST(PreValidation, SimpleCallOperators)
{
    test.enclosedMatches(LR"SRC(
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
}

CAP_TEST(PreValidation, SimpleSubscripts)
{
    test.enclosedMatches(LR"SRC(
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
}

CAP_TEST(PreValidation, ChainedBracketOperators)
{
    test.enclosedMatches(L"foo()[6 ** exp + (1 - a)]()(20)[]",
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
}

CAP_TEST(PreValidation, ComplicatedExpression2)
{
    test.enclosedMatches(L"foo <10, 20> * (a<50> - getType() <int> ().value)",
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

CAP_TEST(PreValidation, CommentsNotParsedAsExpressions)
{
    test.enclosedMatches(LR"SRC(
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
}

CAP_TEST(PreValidation, ComplicatedMultilineExpression)
{
    test.enclosedMatches(LR"SRC(
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

