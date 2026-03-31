#include <cap/BinaryOperator.hh>
#include <cap/BracketOperator.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/Identifier.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>
#include <cap/Validator.hh>

#include <cassert>

namespace cap
{

Function::Function() :
    Declaration(Declaration::Type::Function, m_parameters)
{
}

std::weak_ptr<Node> Function::handleToken(ParserContext& ctx, Token& token)
{
    if (!m_description)
    {
        m_description = std::make_shared<Expression::Root>();
        adopt(m_description);

        return m_description->handleToken(ctx, token);
    }

    else if (!m_body)
    {
        m_body = Scope::startParsing(ctx, token, false);

        if (m_body)
        {
            adopt(m_body);
        }

        return m_body;
    }

    assert(false);
    return {};
}

std::weak_ptr<Node> Function::invokedNodeExited(ParserContext& ctx, Token&)
{
    // Extract the function signature and the function name.
    if (ctx.m_exitedFrom == m_description)
    {
        std::shared_ptr<Expression> checked = m_description->getFirst();
        if (!checked)
        {
            SourceLocation location(ctx.m_source, m_description->getToken());
            ctx.m_client.sourceError(location, "Expected an identifier after 'func'");
            return {};
        }

        // Return type and the rest of the function signature can be delimited by a binary operator.
        // Extract the different sides if the binary operator is an arrow.
        if (checked->getType() == Expression::Type::BinaryOperator)
        {
            auto op = std::static_pointer_cast<BinaryOperator>(m_description->getFirst());
            if (op->getType() == BinaryOperator::Type::Arrow)
            {
                // The rest of the function signature is on the left of the arrow.
                checked = op->getLeft();

                // The return value is on the right of the arrow.
                initializeReturnType();
                adopt(getReturnTypeRoot());
                getReturnTypeRoot()->handleValue(op->getRight());
            }

            else
            {
                SourceLocation location(ctx.m_source, op->getToken());
                ctx.m_client.sourceError(location, "Expected binary operator in function signature to be '->'");
                return {};
            }
        }

        // Extract the function name and the parameters.
        std::shared_ptr<BracketOperator> callOp;
        if (checked->getType() == Expression::Type::BracketOperator)
        {
            // TODO: What about generics?
            auto op = std::static_pointer_cast<BracketOperator>(checked);
            if (op->getType() == BracketOperator::Type::Call)
            {
                callOp = op;
            }
        }

        if (!callOp)
        {
            // TODO: How about anonymous functions?
            SourceLocation location(ctx.m_source, checked->getToken());
            ctx.m_client.sourceError(location, "Expected a function name followed by '('");
            return {};
        }

        // Extract the function name.
        std::shared_ptr<Identifier> name;
        if (callOp->getContext()->getType() == Expression::Type::Value)
        {
            // TODO: What about dynamically injected names?
            auto value = std::static_pointer_cast<Value>(callOp->getContext());
            if (value->getType() == Value::Type::Identifier)
            {
                name = std::static_pointer_cast<Identifier>(value);
            }
        }

        if (!name)
        {
            SourceLocation location(ctx.m_source, callOp->getContext()->getToken());
            ctx.m_client.sourceError(location, "Expected an identifier after 'func'");
            return {};
        }

        // Inject the contents of the parentheses into a variable root and create declarations.
        initializeParameters();
        adopt(getParameterRoot());
        getParameterRoot()->adoptExpression(callOp->getInnerRoot(), ctx);

        // Expose this function declaration.
        assert(getParentScope());
        m_name = name->getValue();
        getParentScope()->declarations.add(std::static_pointer_cast<Function>(shared_from_this()));

        return weak_from_this();
    }

    else if (ctx.m_exitedFrom == m_body)
    {
        return getParent();
    }

    assert(false);
    return {};
}

std::shared_ptr<Scope> Function::getBody() const
{
    return m_body;
}

bool Function::validate(Validator& validator)
{
    if (!m_referredType.has_value())
    {
        if (!Declaration::validate(validator))
        {
            return false;
        }

        m_referredType.emplace(TypeContext(*this));

        if (!getReturnTypeRoot())
        {
            initializeReturnType();
        }

        if (!validator.traverseStatement(getParameterRoot()) || !validator.traverseExpression(getReturnTypeRoot()) ||
            !validator.traverseScope(m_body))
        {
            return false;
        }

        // If no return type still exists, default to void.
        if (!getReturnTypeRoot()->getResultType())
        {
            auto& voidType = validator.getParserContext().m_client.getBuiltin().get(Builtin::DataType::Void);
            getReturnTypeRoot()->setResultType(voidType);
        }
    }

    return true;
}

const char* Function::getTypeString() const
{
    return "Function";
}

} // namespace cap
