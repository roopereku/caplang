#include <cap/test/CapTest.hh>

using namespace cap::test;

CAP_TEST(Error, ReturnInGlobalScope)
{
    test.reportsError(
        LR"SRC(
    return 0
    )SRC",
        L"Cannot return here", true);
}

CAP_TEST(Error, ReturnInTypeBody)
{
    test.reportsError(
        LR"SRC(
    type Foo
    {
        return 0
    }
    )SRC",
        L"Cannot return here", true);
}

CAP_TEST(Error, ReturnStringInFunctionReturningInt64)
{
    test.reportsError(
        LR"SRC(
    func foo() -> int64
    {
        return "someString"
    }
    )SRC",
        L"Incompatible return type", true);
}

CAP_TEST(Error, MultipleMismatchingReturnTypes1)
{
    test.reportsError(
        LR"SRC(
    func foo()
    {
        return "someString"
        return 10
    }
    )SRC",
        L"Incompatible return type", true);
}

CAP_TEST(Error, MultipleMismatchingReturnTypes2)
{
    test.reportsError(
        LR"SRC(
    func foo()
    {
        return
        return 10
    }
    )SRC",
        L"Incompatible return type", true);
}

CAP_TEST(Error, ReturnIntInFunctionReturningVoid)
{
    test.reportsError(
        LR"SRC(
    func foo() -> void
    {
        return 10
    }
    )SRC",
        L"Incompatible return type", true);
}

// clang-format off

CAP_TEST(PreValidation, ReturnInteger)
{
    test.matches(L"func x()\n{\nreturn 0\n}\n",
    {
        Function(L"x"),
            Scope(),
                Return(),
                    Expression(),
                        Integer(0)
    });
}

CAP_TEST(PreValidation, ReturnIntegerResultOfExpression)
{
    test.matches(L"func x()\n{\nreturn\n1 + 2\n}\n",
    {
        Function(L"x"),
            Scope(),
                Return(),
                    Expression(),
                Expression(),
                    cap::BinaryOperator::Type::Add,
                        Integer(1),
                        Integer(2)
    });
}

std::wstring_view setupWithDifferentReturnTypes = LR"SRC(
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
)SRC";

CAP_TEST(PostValidation, FunctionReturningIntResultsInInt)
{
    test.setup(setupWithDifferentReturnTypes);
    test.enclosedMatches(L"let a = returnInt()",
    {
        LocalVariable(L"a"),
            cap::BracketOperator::Type::Call > L"int64",
                Identifier(L"returnInt") > L"func() -> int64",
                Expression()
    });
}

CAP_TEST(PostValidation, FunctionReturningStringResultsInString)
{
    test.setup(setupWithDifferentReturnTypes);
    test.enclosedMatches(L"let b = returnString()",
    {
        LocalVariable(L"b"),
            cap::BracketOperator::Type::Call > L"string",
                Identifier(L"returnString") > L"func() -> string",
                Expression()
    });
}

CAP_TEST(PostValidation, FunctionExplicitlyReturningVoidResultsInVoid)
{
    test.setup(setupWithDifferentReturnTypes);
    test.enclosedMatches(L"returnVoid()",
    {
        Expression(),
            cap::BracketOperator::Type::Call > L"void",
                Identifier(L"returnVoid") > L"func() -> void",
                Expression()
    });
}

CAP_TEST(PostValidation, FunctionImplicitlyReturningVoidResultsInVoid)
{
    test.setup(setupWithDifferentReturnTypes);
    test.enclosedMatches(L"returnDefault()",
    {
        Expression(),
            cap::BracketOperator::Type::Call > L"void",
                Identifier(L"returnDefault") > L"func() -> void",
                Expression()
    });
}
