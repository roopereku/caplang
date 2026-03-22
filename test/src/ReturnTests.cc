#include <cap/test/CapTest.hh>

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
