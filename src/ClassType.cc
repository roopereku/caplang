#include <cap/ClassType.hh>
#include <cap/Client.hh>
#include <cap/ParserContext.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

ClassType::ClassType() :
    Declaration(Declaration::Type::Class, m_generics),
    TypeDefinition(TypeDefinition::Type::Class)
{
}

std::weak_ptr<Node> ClassType::handleToken(ParserContext& ctx, Token& token)
{
    if (m_name.empty())
    {
        if (token.getType() != Token::Type::Identifier)
        {
            SourceLocation location(ctx.m_source, token);
            ctx.m_client.sourceError(location, "Expected an identifier after 'type'");
            return {};
        }

        setToken(token);
        m_name = ctx.m_source.getString(token);

        assert(getParentScope());
        getParentScope()->declarations.add(std::static_pointer_cast<ClassType>(shared_from_this()));

        return weak_from_this();
    }

    // Parse a generic.
    else if (token.isOpeningBracket(ctx, '<'))
    {
        // TODO: Check for this again in cases like "type T <a> <b>"?

        m_generic = std::make_shared<Variable::Root>(Variable::Type::Generic);
        adopt(m_generic);
        return m_generic;
    }

    // Parse base types.
    else if (ctx.m_source[token.getIndex()] == ':')
    {
        // TODO: Check for this again in cases like "type T : a : b"?
        // Mostly applies if there is a way to end the first expression.

        m_baseTypes = std::make_shared<Expression::Root>();
        adopt(m_baseTypes);
        return m_baseTypes;
    }

    else if (!m_body)
    {
        m_body = Scope::startParsing(ctx, token, true);
        if (m_body)
        {
            adopt(m_body);
        }

        return m_body;
    }

    assert(false);
    return {};
}

std::weak_ptr<Node> ClassType::invokedNodeExited(ParserContext& ctx, Token&)
{
    if (ctx.m_exitedFrom == m_generic)
    {
        ctx.m_declarationLocation = nullptr;
        return weak_from_this();
    }

    else if (ctx.m_exitedFrom == m_baseTypes)
    {
        return weak_from_this();
    }

    else if (ctx.m_exitedFrom == m_body)
    {
        return getParent();
    }

    assert(false);
    return {};
}

std::shared_ptr<Expression::Root> ClassType::getBaseTypeRoot()
{
    return m_baseTypes;
}

std::shared_ptr<Variable::Root> ClassType::getGenericRoot()
{
    return m_generic;
}

std::shared_ptr<Scope> ClassType::getBody()
{
    return m_body;
}

bool ClassType::validate(Validator& validator)
{
    if (!m_referredType.has_value())
    {
        if (!Declaration::validate(validator))
        {
            return false;
        }

        if (getBaseTypeRoot() && !validator.traverseExpression(getBaseTypeRoot()))
        {
            return false;
        }

        m_referredType.emplace(*this);
        m_referredType.value().m_isTypeName = true;
    }

    return true;
}

std::wstring ClassType::toString(bool detailed) const
{
    // TODO: Include generics.
    std::wstring result = detailed ? getLocation() : getName();
    return result;
}

const char* ClassType::getTypeString() const
{
    return "Class Type";
}

} // namespace cap
