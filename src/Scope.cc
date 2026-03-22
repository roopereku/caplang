#include <cap/BinaryOperator.hh>
#include <cap/ClassType.hh>
#include <cap/Client.hh>
#include <cap/Conversion.hh>
#include <cap/Expression.hh>
#include <cap/Function.hh>
#include <cap/ParserContext.hh>
#include <cap/Return.hh>
#include <cap/Scope.hh>
#include <cap/Variable.hh>

#include <cassert>

namespace cap
{

Scope::Scope() :
    Scope(true)
{
}

Scope::Scope(bool onlyDeclarations) :
    Node(Node::Type::Scope, declarations),
    m_onlyDeclarations(onlyDeclarations)
{
}

std::weak_ptr<Node> Scope::handleToken(ParserContext& ctx, Token& token)
{
    if (ctx.m_source.match(token, L"func"))
    {
        return consumeAttributes(appendNested(std::make_shared<Function>(), token), ctx);
    }

    else if (ctx.m_source.match(token, L"conversion"))
    {
        return consumeAttributes(appendNested(std::make_shared <Conversion> (), token), ctx);
    }

    else if (ctx.m_source.match(token, L"type"))
    {
        return consumeAttributes(appendNested(std::make_shared<ClassType>(), token), ctx);
    }

    else if (ctx.m_source.match(token, L"let"))
    {
        // TODO: Do fields for class members?
        auto varDecl =
            consumeAttributes(appendNested(std::make_shared<Variable::Root>(Variable::Type::Local), token), ctx);
        return std::static_pointer_cast<Statement>(varDecl)->getContinuation(ctx);
    }

    else if (ctx.m_source.match(token, L"return"))
    {
        auto ret = consumeAttributes(appendNested(std::make_shared<Return>(), token), ctx);
        return std::static_pointer_cast<Statement>(ret)->getContinuation(ctx);
    }

    else if (token.isOpeningBracket(ctx, '{'))
    {
        // TODO: Implement subscopes.
    }

    else if (token.isClosingBracket(ctx, '}'))
    {
        assert(!getParent().expired());
        ctx.m_exitedFrom = shared_from_this();
        return getParent().lock()->invokedNodeExited(ctx, token);
    }

    // Parse anything else as expressions.
    else
    {
        // Attributes can be applied to declarations and thus are allowed.
        if (token.getType() == Token::Type::Attribute)
        {
            ctx.m_allowExpressionEndingInAttributes = true;
        }

        // If only declarations are allowed, forbid a top level expression.
        else if (m_onlyDeclarations)
        {
            SourceLocation location(ctx.m_source, token);
            ctx.m_client.sourceError(location, "Only declarations are allowed here");
            return {};
        }

        // Adopt the expression root and delegate the first token of
        // the expression to the root.
        auto exprRoot = std::make_shared<Expression::Root>();
        appendNested(exprRoot, token);
        return exprRoot->handleToken(ctx, token);
    }

    return weak_from_this();
}

std::weak_ptr<Node> Scope::invokedNodeExited(ParserContext& ctx, Token& token)
{
    // Expressions ending in attributes are only allowed when the expression is invoked
    // from a scope context and only attributes are present. In such an exit the attributes
    // are already stored and the corresponding expression root should be disposed of.
    if (ctx.m_exitedFrom->getType() == Node::Type::Expression && ctx.m_allowExpressionEndingInAttributes)
    {
        m_nested.pop_back();
        ctx.m_allowExpressionEndingInAttributes = false;
    }

    if (token.isClosingBracket(ctx, '}'))
    {
        // Let the parent node handle the closing bracket.
        assert(!getParent().expired());
        ctx.m_exitedFrom = shared_from_this();
        return getParent().lock()->invokedNodeExited(ctx, token);
    }

    return weak_from_this();
}

std::shared_ptr<Scope> Scope::startParsing(ParserContext& ctx, Token& token, bool onlyDeclarations)
{
    if (token.isOpeningBracket(ctx, '{'))
    {
        return std::make_shared<Scope>(onlyDeclarations);
    }

    SourceLocation location(ctx.m_source, token);
    ctx.m_client.sourceError(location, "Expected '{'");
    return nullptr;
}

const std::vector<std::shared_ptr<Node>>& Scope::getNested()
{
    return m_nested;
}

const char* Scope::getTypeString() const
{
    return "Scope";
}

std::shared_ptr<Node> Scope::appendNested(std::shared_ptr<Node> node, Token& token)
{
	// TODO: Make sure that the token isn't the last on the line.
	// Might not make sense in the case of opening/closing brackets.

    adopt(node);
    node->setToken(token);
    return m_nested.emplace_back(std::move(node));
}

std::shared_ptr<Node> Scope::consumeAttributes(std::shared_ptr<Node> node, ParserContext& ctx)
{
    if (!ctx.m_activeAttributes.empty())
    {
        assert(ctx.m_activeAttributes.size() == 1);
        node->setAttributeRange(ctx.m_activeAttributes.top().m_range);
        ctx.m_activeAttributes.pop();

        // The node that consumes the attributes shall also adopt them
        // in order for their validation to happen in the context of the given node.
        auto attributes = ctx.m_client.getAttributes(node);
        for (auto& attribute : attributes)
        {
            node->adopt(attribute);
        }
    }

    return node;
}

} // namespace cap
