#include <cap/test/CapTest.hh>

CAP_TEST(Error, LocalVariablesWithSameName)
{
    test.reportsError(L"let a = 10\nlet a = 20", L"'a' already exists");
}

CAP_TEST(Error, FunctionAndLocalVariableWithSameName)
{
    test.reportsError(L"func foo()\n{\n}\nlet foo = 20", L"'foo' already exists");
}

CAP_TEST(Error, LocalVariableAndFunctionWithSameName)
{
    test.reportsError(L"let foo = 20\nfunc foo()\n{\n}", L"'foo' already exists");
}

CAP_TEST(Error, TypeAndLocalVariableWithSameName)
{
    test.reportsError(L"type foo\n{\n}\nlet foo = 20", L"'foo' already exists");
}

CAP_TEST(Error, LocalVariableAndTypeWithSameName)
{
    test.reportsError(L"let foo = 20\ntype foo\n{\n}", L"'foo' already exists");
}

CAP_TEST(Error, LocalVariableWithBuiltinNameInt8)
{
    test.reportsError(L"let int8 = 10\n", L"'int8' already exists");
}

CAP_TEST(Error, LocalVariableWithBuiltinNameInt32)
{
    test.reportsError(L"let int32 = 10\n", L"'int32' already exists");
}

CAP_TEST(Error, LocalVariableWithBuiltinNameUint16)
{
    test.reportsError(L"let uint16 = 10\n", L"'uint16' already exists");
}

CAP_TEST(Error, LocalVariableWithBuiltinNameUint64)
{
    test.reportsError(L"let uint64 = 10\n", L"'uint64' already exists");
}

CAP_TEST(Error, ParameterWithSameNameAsFunction)
{
    test.reportsError(L"func foo(foo = int64)\n{\n}", L"'foo' already exists");
}

CAP_TEST(Error, LocalVariableInsideFunctionBodyWithSameName)
{
    test.reportsError(L"func foo(a = int64)\n{\nlet a = 10\n}", L"'a' already exists");
}

CAP_TEST(Error, LocalVariableAndParameterWithSameName)
{
    test.reportsError(L"let a = 10\nfunc foo(a = int64)\n{\n}", L"'a' already exists");
}

CAP_TEST(Error, LocalVariableAndInnerLocalVariableWithSameName)
{
    test.reportsError(L"let a = 10\nfunc foo()\n{\nlet a = 10\n}", L"'a' already exists");
}
