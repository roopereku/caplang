#include <cap/test/CapTest.hh>

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
