#include <cap/Scope.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/Expression.hh>
#include <cap/ParserContext.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Variable.hh>
#include <cap/Return.hh>

#include <cassert>

namespace cap
{

Scope::Scope()
	: Scope(true)
{
}

Scope::Scope(bool onlyDeclarations)
	: Node(Node::Type::Scope, declarations), onlyDeclarations(onlyDeclarations)
{
}

std::weak_ptr <Node> Scope::handleToken(ParserContext& ctx, Token& token)
{
	if(ctx.source.match(token, L"func"))
	{
		return appendNested(std::make_shared <Function> (), token);
	}

	else if(ctx.source.match(token, L"type"))
	{
		return appendNested(std::make_shared <ClassType> (), token);
	}

	else if(ctx.source.match(token, L"let"))
	{
		// TODO: Do fields for class members?
		return appendNested(std::make_shared <Variable::Root> (Variable::Type::Local), token);
	}

	else if(ctx.source.match(token, L"return"))
	{
		return appendNested(std::make_shared <Return> (), token);
	}

	else if(token.isOpeningBracket(ctx, '{'))
	{
		// TODO: Implement subscopes.
	}

	else if(token.isClosingBracket(ctx, '}'))
	{
		assert(!getParent().expired());
		ctx.exitedFrom = shared_from_this();
		return getParent().lock()->invokedNodeExited(ctx, token);
	}

	// Parse anything else as expressions.
	else
	{
		// If only declarations are allowed, forbid a top level expression.
		if(onlyDeclarations)
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Only declarations are allowed here");
			return {};
		}

		// Adopt the expression root and delegate the first token of
		// the expression to the root.
		auto exprRoot = std::make_shared <Expression::Root> ();
		appendNested(exprRoot, token);
		auto ret = exprRoot->handleToken(ctx, token);

		return ret;
	}

	return weak_from_this();
}

std::weak_ptr <Node> Scope::invokedNodeExited(ParserContext& ctx, Token& token)
{
	if(token.isClosingBracket(ctx, '}'))
	{
		DBG_MESSAGE(ctx.client, "HANDLE SCOPE EXIT. DELEGATE BRACKET TO ", getParent().lock()->getTypeString());

		// Let the parent node handle the closing bracket.
		assert(!getParent().expired());
		ctx.exitedFrom = shared_from_this();
		return getParent().lock()->invokedNodeExited(ctx, token);
	}

	return weak_from_this();
}

std::shared_ptr <Scope> Scope::startParsing(ParserContext& ctx, Token& token, bool onlyDeclarations)
{
	if(token.isOpeningBracket(ctx, '{'))
	{
		return std::make_shared <Scope> (onlyDeclarations);
	}

	SourceLocation location(ctx.source, token);
	ctx.client.sourceError(location, "Expected '{'");
	return nullptr;
}

const std::vector <std::shared_ptr <Node>>& Scope::getNested()
{
	return nested;
}

const char* Scope::getTypeString() const
{
	return "Scope";
}

std::weak_ptr <Node> Scope::appendNested(std::shared_ptr <Node> node, Token& token)
{
	adopt(node);
	node->setToken(token);
	nested.emplace_back(std::move(node));

	return nested.back();
}

}
