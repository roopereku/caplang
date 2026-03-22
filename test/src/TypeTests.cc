#include <cap/test/CapTest.hh>

CAP_TEST(Error, TypeWithoutName)
{
    test.reportsError(L"type", L"Expected an identifier after 'type'");
}

CAP_TEST(Error, TypeWithoutBody)
{
    test.reportsError(L"type foo", L"Expected '{'");
}

CAP_TEST(Error, GenericTypeWithoutGenericName)
{
    test.reportsError(L"type foo <> {\n}\n", L"Expected a generic");
}
