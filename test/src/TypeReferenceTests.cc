#include <cap/test/CapTest.hh>

using namespace cap::test;

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

// clang-format off

CAP_TEST(PreValidation, TypeReferenceInitializesLocalVariable)
{
    test.matches(L"let T = type int64",
    {
        LocalVariable(L"T"),
            TypeReference(),
                Identifier(L"int64")
    });
}

CAP_TEST(PreValidation, TypeReferenceAcceptsValue1)
{
    test.matches(L"let a = type int64 + 2",
    {
        LocalVariable(L"a"),
            cap::BinaryOperator::Type::Add,
                TypeReference(),
                    Identifier(L"int64"),
                Integer(2)
    });
}

CAP_TEST(PreValidation, TypeReferenceAcceptsValue2)
{
    test.matches(L"let a = type -int64",
    {
        LocalVariable(L"a"),
            TypeReference(),
                cap::UnaryOperator::Type::Negate,
                    Identifier(L"int64")
    });
}

CAP_TEST(PreValidation, TypeReferenceAcceptsValue3)
{
    test.matches(L"let a = type (int64 + int32)",
    {
        LocalVariable(L"a"),
            TypeReference(),
                Expression(),
                    cap::BinaryOperator::Type::Add,
                        Identifier(L"int64"),
                        Identifier(L"int32")
    });
}

CAP_TEST(PreValidation, TypeReferenceAcceptsValue4)
{
    test.matches(L"let a = type *(int64)",
    {
        LocalVariable(L"a"),
            TypeReference(),
                cap::UnaryOperator::Type::ParseTime,
                    Expression(),
                        Identifier(L"int64")
    });
}

CAP_TEST(PreValidation, TypeReferenceAcceptsValue5)
{
    test.matches(L"let a = type *getTypeDynamically()",
    {
        LocalVariable(L"a"),
            TypeReference(),
                cap::UnaryOperator::Type::ParseTime,
                    cap::BracketOperator::Type::Call,
                        Identifier(L"getTypeDynamically"),
                        Expression()
    });
}

CAP_TEST(PreValidation, TypeReferenceAcceptsValue6)
{
    test.matches(L"let a = type typeLookup[0]",
    {
        LocalVariable(L"a"),
            TypeReference(),
                cap::BracketOperator::Type::Subscript,
                    Identifier(L"typeLookup"),
                    Expression(),
                        Integer(0)
    });
}
