#include <cap/test/CapTest.hh>

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
