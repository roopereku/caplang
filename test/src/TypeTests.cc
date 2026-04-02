#include <cap/test/CapTest.hh>

using namespace cap::test;

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
    // TODO: Add the same test but without a newline before the opening brace.
    test.reportsError(L"type foo <>\n{\n}\n", L"Expected a generic");
}

// clang-format off

CAP_TEST(PreValidation, TypeWithoutNested)
{
    test.matches(L"type foo\n{\n}",
    {
        ClassType(L"foo"),
            Scope()
    });
}

CAP_TEST(PreValidation, TypeWithNestedType)
{
    test.matches(L"type foo\n{\ntype bar\n{\n}\n}",
    {
        ClassType(L"foo"),
            Scope(),
                ClassType(L"bar"),
                    Scope()
    });
}

CAP_TEST(PreValidation, TypeWithPreTypedGenerics)
{
    test.matches(L"type foo <T1 = int64, T2 = uint32>\n{\n}",
    {
        ClassType(L"foo"),
            Generic(L"T1"),
                Identifier(L"int64"),
            Generic(L"T2"),
                Identifier(L"uint32"),
            Scope()
    });
}

// TODO: Add base class tests.

