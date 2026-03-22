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

CAP_TEST(PostValidation, TypeReferenceInitialization)
{
    test.enclosedMatches(L"let a = type int64",
    {
        LocalVariable(L"a") > L"type int64",
            TypeReference() > L"type int64",
                Identifier(L"int64") > L"int64"
    });
}

std::wstring_view setupWithAliases = LR"SRC(
    type Outer1
    {
        type Inner1
        {
            type Inner2
            {
                let a = "value"
            }
        }
    }

    let Alias1 = type Outer1
    let Alias2 = type Alias1.Inner1.Inner2
)SRC";

CAP_TEST(PostValidation, TypeReferenceInitializationOfNestedType)
{
    test.setup(setupWithAliases);
    test.enclosedMatches(L"let accessedType = type Alias1.Inner1",
    {
        LocalVariable(L"accessedType") > L"type Outer1.Inner1",
            TypeReference() > L"type Outer1.Inner1",
                cap::BinaryOperator::Type::Access > L"Outer1.Inner1",
                    Identifier(L"Alias1") > L"type Outer1",
                    Identifier(L"Inner1") > L"Outer1.Inner1"
    });
}

CAP_TEST(PostValidation, MemberVariableAccessibleThroughTypeReference)
{
    test.setup(setupWithAliases);
    test.enclosedMatches(L"let value = Alias2.a",
    {
        LocalVariable(L"value") > L"string",
            cap::BinaryOperator::Type::Access > L"string",
                Identifier(L"Alias2") > L"type Outer1.Inner1.Inner2",
                Identifier(L"a") > L"string"
    });
}

