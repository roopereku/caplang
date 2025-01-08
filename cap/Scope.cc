#include <cap/Scope.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/Expression.hh>

#include <cassert>

namespace cap
{

Scope::Scope()
	: Node(Node::Type::Scope), onlyDeclarations(true)
{
}

Scope::Scope(bool onlyDeclarations)
	: Node(Node::Type::Scope), onlyDeclarations(onlyDeclarations)
{
}

std::weak_ptr <Node> Scope::handleToken(ParserContext& ctx, Token& token)
{
	if(token.isClosingBracket(ctx, '}'))
	{
		// Let the parent node handle the closing bracket.
		assert(!getParent().expired());
		return getParent().lock()->handleToken(ctx, token);
	}

	if(ctx.source.match(token, L"func"))
	{
		return appendNested(std::make_shared <Function> ());
	}

	else if(token.isOpeningBracket(ctx, '{'))
	{
		// TODO: Implement subscopes.
	}

	// TODO: When it's guaranteed that no comments can appear here,
	// it's almost given that anything else relates to an expression.
	else if(token.canBeValue() || token.getType() == Token::Type::Operator || token.getType() == Token::Type::OpeningBracket)
	{
		if(onlyDeclarations)
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Only declarations are allowed here");
			return {};
		}

		// Adopt the expression root and delegate the first token of
		// the expression to the root.
		auto exprRoot = std::make_shared <Expression::Root> ();
		appendNested(exprRoot);
		return exprRoot->handleToken(ctx, token);
	}

	return weak_from_this();
}

const std::vector <std::shared_ptr <Node>>& Scope::getNested()
{
	return nested;
}

const char* Scope::getTypeString()
{
	return "Scope";
}

std::weak_ptr <Node> Scope::appendNested(std::shared_ptr <Node> node)
{
	adopt(node);
	nested.emplace_back(std::move(node));

	return nested.back();
}

}
