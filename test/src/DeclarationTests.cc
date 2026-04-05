#include <cap/test/CapTest.hh>

using namespace cap::test;

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

// clang-format off

CAP_TEST(PreValidation, NestedDeclarationsOnSameLine1)
{
    test.matches(L"func foo(){type bar{}}",
    {
        Function(L"foo"),
            Scope(),
                ClassType(L"bar"),
                    Scope(),
    });
}

CAP_TEST(PreValidation, NestedDeclarationsOnSameLine2)
{
    test.matches(L"func foo(){type bar{}}",
    {
        Function(L"foo"),
            Scope(),
                ClassType(L"bar"),
                    Scope(),
    });
}

CAP_TEST(PreValidation, NestedDeclarationsOnSameLine3)
{
    test.matches(L"type foo{func bar(){}}",
    {
        ClassType(L"foo"),
            Scope(),
                Function(L"bar"),
                    Scope(),
    });
}

CAP_TEST(PreValidation, NestedDeclarationsOnSameLine4)
{
    test.matches(L"func foo(){type bar{func nestedFoo(){let local1 = int64, local2 = int32}}}",
    {
        Function(L"foo"),
            Scope(),
                ClassType(L"bar"),
                    Scope(),
                        Function(L"nestedFoo"),
                            Scope(),
                                LocalVariable(L"local1"),
                                    Identifier(L"int64"),
                                LocalVariable(L"local2"),
                                    Identifier(L"int32"),
    });
}
