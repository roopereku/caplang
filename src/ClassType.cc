#include <cap/BracketOperator.hh>
#include <cap/ClassType.hh>
#include <cap/Client.hh>
#include <cap/Identifier.hh>
#include <cap/ParserContext.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

ClassType::ClassType() :
    Declaration(Declaration::Type::Class, generics),
    TypeDefinition(TypeDefinition::Type::Class)
{
}

std::weak_ptr<Node> ClassType::handleToken(ParserContext& ctx, Token& token)
{
    if (!m_description)
    {
        m_description = std::make_shared<Expression::Root>();
        adopt(m_description);

        return m_description->handleToken(ctx, token);
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
    if (ctx.m_exitedFrom == m_description)
    {
        std::shared_ptr<Expression> checked = m_description->getFirst();
        if (!checked)
        {
            SourceLocation location(ctx.m_source, m_description->getToken());
            ctx.m_client.sourceError(location, "Expected an identifier after 'type'");
            return {};
        }

        // TODO: Extract base types.

        // Extract generics which are specified as "type name<...>".
        if (checked->getType() == Expression::Type::BracketOperator)
        {
            auto op = std::static_pointer_cast<BracketOperator>(checked);
            if (op->getType() == BracketOperator::Type::Generic)
            {
                m_generic = std::make_shared<Variable::Root>(Variable::Type::Generic);
                adopt(m_generic);

                checked = op->getContext();
                if (!m_generic->adoptExpression(op->getInnerRoot(), ctx))
                {
                    return {};
                }
            }

            else
            {
                SourceLocation location(ctx.m_source, op->getToken());
                ctx.m_client.sourceError(location, "Expected a generic declaration");
                return {};
            }
        }

        // TODO: What about injected names?
        // Extract the type name.
        if (checked->getType() == Expression::Type::Value)
        {
            auto value = std::static_pointer_cast<Value>(checked);
            if (value->getType() == Value::Type::Identifier)
            {
                m_name = std::static_pointer_cast<Identifier>(value)->getValue();
            }
        }

        if (m_name.empty())
        {
            SourceLocation location(ctx.m_source, checked->getToken());
            ctx.m_client.sourceError(location, "Expected an identifier after 'type'");
            return {};
        }

        // Expose this type declaration.
        assert(getParentScope());
        getParentScope()->declarations.add(std::static_pointer_cast<ClassType>(shared_from_this()));

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
