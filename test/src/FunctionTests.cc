#include <cap/test/CapTest.hh>

using namespace cap::test;

CAP_TEST(Error, UndeclaredTypeAsFunctionReturnType)
{
    test.reportsError(L"func foo() -> abc\n{\n}\n", L"Undeclared identifier 'abc'");
}

CAP_TEST(Error, FunctionDeclarationWithoutName)
{
    test.reportsError(L"func", L"Expected an identifier after 'func'");
}

CAP_TEST(Error, FunctionDeclarationWithoutParenthesis)
{
    test.reportsError(L"func foo", L"Expected '(' after function name");
}

CAP_TEST(Error, FunctionDeclarationWithoutBody)
{
    test.reportsError(L"func foo()", L"Expected '{'");
}

CAP_TEST(Error, IdenticalFunctionDeclarations)
{
    test.reportsError(
        LR"SRC(
		func foo(a = int64, b = string)
		{
		}

		func foo(a = int64, b = string)
		{
		}
        )SRC",
        L"Function with the same parameters already exists");
}

// TODO: Implement this with some setup functionality?
auto testCall = [](std::wstring&& str) -> std::wstring
{
    std::wstring funcs =
        LR"SRC(
        func foo(a = string, b = int64) -> int64
        {
        }

        func foo(a = int64) -> int64
        {
        }
        )SRC";

    return funcs + L"func main()\n{\n" + std::move(str) + L"\n}\n";
};

CAP_TEST(Error, NoMatchingOverload1)
{
    test.reportsError(testCall(L"foo()"), L"No matching overload found for 'foo'");
}

CAP_TEST(Error, NoMatchingOverload2)
{
    test.reportsError(testCall(L"foo(\"test\")"), L"No matching overload found for 'foo'");
}

CAP_TEST(Error, NoMatchingOverload3)
{
    test.reportsError(testCall(L"foo(10, \"test\")"), L"No matching overload found for 'foo'");
}

CAP_TEST(Error, NoMatchingOverload4)
{
    test.reportsError(testCall(L"foo(10, \"test\", 20, 50)"), L"No matching overload found for 'foo'");
}

CAP_TEST(Error, FunctionParametersMustBeInitializedWithTypes)
{
    test.reportsError(L"func foo(a = int64, b = \"test\")\n{\n}", L"Parameters must be initialized with types");
}

// clang-format off

CAP_TEST(PreValidation, FunctionWithoutParametersAndNothingNested)
{
    test.matches(L"func foo()\n{\n}",
    {
        Function(L"foo"),
            Scope()
    });
}

CAP_TEST(PreValidation, FunctionWithParametersAndNothingNested)
{
    test.matches(L"func foo(a = int64, b = uint32)\n{\n}",
    {
        Function(L"foo"),
            Parameter(L"a"),
                Identifier(L"int64"),
            Parameter(L"b"),
                Identifier(L"uint32"),
            Scope()
    });
}

CAP_TEST(PreValidation, NestedFunctionsWithoutParameters)
{
    test.matches(L"func foo()\n{\nfunc bar()\n{\n}\n}\n",
    {
        Function(L"foo"),
            Scope(),
                Function(L"bar"),
                    Scope()
    });
}

CAP_TEST(PreValidation, FunctionWithReturnType)
{
    test.matches(L"func foo() -> SomeType\n{\n}\n",
    {
        Function(L"foo"),
            Expression(),
                Identifier(L"SomeType"),
            Scope()
    });
}

std::wstring_view setupWithFunctionsAndVariables = LR"SRC(
    func foo(a = string, b = int64, c = int64)
    {
    }

    type Foo
    {
        func foo(a = string, b = int64)
        {
        }
    }

    let x = 10
    let y = 200
    let str = "some string"
)SRC";

CAP_TEST(PostValidation, FunctionIsMatchedBasedOnVariableTypes1)
{
    test.setup(setupWithFunctionsAndVariables);
    test.enclosedMatches(L"foo(str, x, y)",
    {
        Expression() > L"void",
            cap::BracketOperator::Type::Call > L"void",
                Identifier(L"foo") > L"func(string, int64, int64) -> void",
                Expression(),
                    cap::BinaryOperator::Type::Comma,
                        cap::BinaryOperator::Type::Comma,
                            Identifier(L"str") > L"string",
                            Identifier(L"x") > L"int64",
                        Identifier(L"y") > L"int64"
    });
}

CAP_TEST(PostValidation, FunctionIsMatchedBasedOnVariableTypes2)
{
    test.setup(setupWithFunctionsAndVariables);
    test.enclosedMatches(L"Foo.foo(str, x)",
    {
        Expression() > L"void",
            cap::BinaryOperator::Type::Access > L"void",
                Identifier(L"Foo") > L"Foo",
                cap::BracketOperator::Type::Call > L"void",
                    Identifier(L"foo") > L"func(string, int64) -> void",
                    Expression(),
                        cap::BinaryOperator::Type::Comma,
                            Identifier(L"str") > L"string",
                            Identifier(L"x") > L"int64",
    });
}

// TODO: Add a test where function parameters are passed onwards?
