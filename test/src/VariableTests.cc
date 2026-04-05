#include <cap/test/CapTest.hh>

using namespace cap::test;

CAP_TEST(Error, VariableAssignmentInGlobalScope)
{
    test.reportsError(L"a = 10", L"Only declarations are allowed here", true);
}

CAP_TEST(Error, VariableAssignmentInType)
{
    test.reportsError(L"type T\n{\na = 10\n}", L"Only declarations are allowed here", true);
}

CAP_TEST(Error, LocalVariableInitializedWithUndeclaredIdentifier)
{
    test.reportsError(L"let a = c", L"Undeclared identifier 'c'");
}

CAP_TEST(Error, LatterOfTwoVariablesInSameLetInitializedWithUndeclaredIdentifier)
{
    test.reportsError(L"let a = 10\nlet b = a1", L"Undeclared identifier 'a1'");
}

CAP_TEST(Error, IntegerUsedAsLocalVariableName)
{
    test.reportsError(L"let 10 = c", L"Expected an identifier");
}

CAP_TEST(Error, HexadecimalUsedAsLocalVariableName)
{
    test.reportsError(L"let 0xFF = 10", L"Expected an identifier");
}

CAP_TEST(Error, UnaryParseTimeWithoutCallUsedAsLocalVariableName)
{
    test.reportsError(L"let *a = 10", L"Expected an identifier");
}

CAP_TEST(Error, NonAttributeLocalVariableWithoutInitialization)
{
    test.reportsError(L"let a", L"Expected '='");
}

CAP_TEST(Error, LocalVariableWithoutName)
{
    test.reportsError(L"let", L"Expected a variable");
}

CAP_TEST(Error, LocalVariableNameOnDifferentLine)
{
    // TODO: Should this same logic apply to functions and types where
    // the name has to be on the same line as the declarator.
    test.reportsError(L"let\na = 10", L"Expected a variable");
}

// clang-format off

CAP_TEST(PreValidation, Name)
{
    test.enclosedMatches(L"let a = 1 + 2",
    {
        LocalVariable(L"a"),
            cap::BinaryOperator::Type::Add,
                Integer(1),
                Integer(2)
    });
}

CAP_TEST(PreValidation, VariableCanBeDefinedWithTrailingScopeEnd)
{
    test.matches(L"func foo()\n{\nlet a = 10}",
    {
        Function(L"foo"),
            Scope(),
                LocalVariable(L"a"),
                    Integer(10)
    });
}

CAP_TEST(PostValidation, ParameterTypeResultsInCorrectType1)
{
    test.enclosedMatches(L"func foo(a = int64)\n{\n}",
    {
        Function(L"foo") > L"func(int64) -> void",
            Parameter(L"a") > L"int64",
                Identifier(L"int64") > L"int64",
            Expression() > L"void",
            Scope()
    });
}

CAP_TEST(PostValidation, ParameterTypeResultsInCorrectType2)
{
    test.enclosedMatches(L"func foo(a = int64, b = string)\n{\n}",
    {
        Function(L"foo") > L"func(int64, string) -> void",
            Parameter(L"a") > L"int64",
                Identifier(L"int64") > L"int64",
            Parameter(L"b") > L"string",
                Identifier(L"string") > L"string",
            Expression() > L"void",
            Scope()
    });
}

// TODO: Tests for generic parameter types with constraints and interfaces.
// TODO: Tests for type references to generic parameter types with constraints and interfaces.
