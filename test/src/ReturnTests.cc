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
