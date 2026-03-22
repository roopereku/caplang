#include <cap/test/CapTest.hh>

using namespace cap::test;

CAP_TEST(Error, Name)
{
    test.reportsError(L"(@foo)", L"Expression must not end in an attribute here");
}

CAP_TEST(Error, StandaloneSubExpressionEndsInAttribute)
{
    test.reportsError(L"(@foo @bar)", L"Expression must not end in an attribute here");
}

CAP_TEST(Error, SubExpressionEndsInAttribute)
{
    test.reportsError(L"a + (@foo) - b", L"Expression must not end in an attribute here");
}

CAP_TEST(Error, CallWithoutArgumentsEndsInAttribute)
{
    test.reportsError(L"a (@foo)", L"Expression must not end in an attribute here");
}

CAP_TEST(Error, FunctionEmptyParametersEndInAttribute)
{
    test.reportsError(L"func something(@foo) {\n}\n", L"Expression must not end in an attribute here");
}

CAP_TEST(Error, AttributeFunctionEmptyParametersEndInAttribute)
{
    test.reportsError(L"@attr\nfunc something(@foo) {\n}\n", L"Expression must not end in an attribute here");
}

CAP_TEST(Error, NonAttributeLocalUsedAsAttributeForLocal)
{
    test.reportsError(
        LR"SRC(
		let foo = 10
		let @foo bar = 20
	)SRC",
        L"Declarations used as attributes must be declared as attributes");
}

CAP_TEST(Error, NonAttributeFunctionUsedAsAttributeForLocal)
{
    test.reportsError(
        LR"SRC(
		func foo()
		{
		}

		let @foo bar = 20
	)SRC",
        L"Declarations used as attributes must be declared as attributes");
}

CAP_TEST(Error, NonAttributeTypeUsedAsAttributeForLocal)
{
    test.reportsError(
        LR"SRC(
		type foo
		{
		}

		let @foo bar = 20
	)SRC",
        L"Declarations used as attributes must be declared as attributes");
}

CAP_TEST(Error, NonAttributeLocalUsedAsAttributeForFunction)
{
    test.reportsError(
        LR"SRC(
		let foo = 10

		@foo
		func bar()
		{
		}
	)SRC",
        L"Declarations used as attributes must be declared as attributes");
}

CAP_TEST(Error, AttributeAndNonAttributeLocalsUsedAsAttributeForLocal)
{
    test.reportsError(
        LR"SRC(
		let @attribute valid
		let invalid = 10

		let @valid @invalid foo = 10
	)SRC",
        L"Declarations used as attributes must be declared as attributes");
}

CAP_TEST(Error, AttributeAppliedToLetTODO)
{
    test.reportsError(L"@attribute\nlet a = 10", L"TODO: Figure out if attributes in Variable::Root are allowed");
}

// TODO: Make an error test for the following attribute cases:
// - @@foo
// - @(@foo)
// - @foo()
// - @foo[]

// TODO: Make an error test for when a scope ends with an attribute

// clang-format off

CAP_TEST(PreValidation, AttributeDisconnected1)
{
    test.matches(L"@debug something",
    {
        Expression(),
            Identifier(L"something")
    });
}

CAP_TEST(PreValidation, AttributeDisconnected2)
{
    test.matches(L"@debug @otherAttr(1 + 2) something",
    {
        Expression(),
            Identifier(L"something")
    });
}

CAP_TEST(PreValidation, AttributeDisconnected3)
{
    test.matches(L"@*dynamicAttr something\n@debug print(\"test\")",
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

// TODO: Enable this once the expression is no longer interpreted as @foo(...) + ...
//test.test(L"@foo (@bar x) + @baz y", {
//	Expression(),
//		cap::BinaryOperator::Type::Add,
//		Expression(),
//			Identifier(L"x"),
//		Identifier(L"y")
//});


// TODO: Once e.g. "@foo() ()" can be differentiated from "@foo()()" this test can be
// replaced with a real test.
CAP_TEST(PreValidation, SeparatedParenthesesAttachedToAttributeTODO)
{
    test.matches(L"@foo () baz",
    {
        Expression(),
            Identifier(L"baz")
    });
}

// TODO: Make a similar error test testing that something like "@foo()()" isn't allowed.
// Calling an attribute doesn't make much sense.

// TODO: @attr (1 + 2)
// TODO: @attr [1 + 2]
// TODO: @foo(1 + 2) (@bar)

std::wstring_view setupWithAttributes = L"let @attribute foo, @attribute bar";

CAP_TEST(PostValidation, SingleAttributeAppliedToVariable)
{
    test.setup(setupWithAttributes);
    test.enclosedMatches(L"let @foo a = 10",
    {
        LocalVariable(L"a") > L"int64",
            AttributeUsage(),
                Identifier(L"foo"),
            Integer(10) > L"int64"
    });
}

CAP_TEST(PostValidation, MultipleAttributesAppliedToVariable)
{
    test.setup(setupWithAttributes);
    test.enclosedMatches(L"let @foo @bar a = 10",
    {
        LocalVariable(L"a") > L"int64",
            AttributeUsage(),
                Identifier(L"foo"),
            AttributeUsage(),
                Identifier(L"bar"),
            Integer(10) > L"int64"
    });
}

CAP_TEST(PostValidation, AttributesAppliedToParameters)
{
    test.setup(setupWithAttributes);
    test.enclosedMatches(LR"SRC(
        func a(@foo param1 = int64, @bar param2 = string)
        {
        }
    )SRC",
    {
        Function(L"a"),
            Parameter(L"param1") > L"int64",
                AttributeUsage(),
                    Identifier(L"foo"),
                Identifier(L"int64") > L"int64",
            Parameter(L"param2") > L"string",
                AttributeUsage(),
                    Identifier(L"bar"),
                Identifier(L"string") > L"string",
            Expression() > L"void",
            Scope()
    });
}

CAP_TEST(PostValidation, AttributeAppliedToFunction)
{
    test.setup(setupWithAttributes);
    test.enclosedMatches(LR"SRC(
        @foo
        func a(param1 = int64)
        {
        }
    )SRC",
    {
        Function(L"a"),
            AttributeUsage(),
                Identifier(L"foo"),
            Parameter(L"param1") > L"int64",
                Identifier(L"int64") > L"int64",
            Expression() > L"void",
            Scope()
    });
}
