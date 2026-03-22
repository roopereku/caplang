#include <cap/test/CapTest.hh>

CAP_TEST(Error, LocalAssignedLiteralTypeName)
{
    test.reportsError(L"let a = int64", L"Type names must be preceded by 'type'");
}

CAP_TEST(Error, LocalAssignedValueWithTypePrefix)
{
    test.reportsError(L"let a = type 10", L"Value given to 'type' must be a type name");
}

CAP_TEST(Error, LocalAssignedValidTypeReferenceInParentheses)
{
    // TODO: This might become valid at some point.
    test.reportsError(L"let x = (type string)", L"Type names must be preceded by 'type'");
}
