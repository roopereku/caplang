#include <cap/Scope.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
#include <cap/Expression.hh>
#include <cap/ParserContext.hh>
#include <cap/BinaryOperator.hh>
#include <cap/Variable.hh>
#include <cap/Value.hh>

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

	else if(ctx.source.match(token, L"type"))
	{
		return appendNested(std::make_shared <ClassType> ());
	}

	else if(token.isOpeningBracket(ctx, '{'))
	{
		// TODO: Implement subscopes.
	}

	// Parse anything else as expressions.
	else
	{
		// Adopt the expression root and delegate the first token of
		// the expression to the root.
		auto exprRoot = std::make_shared <Expression::Root> ();
		appendNested(exprRoot);
		auto ret = exprRoot->handleToken(ctx, token);

		// If only declarations are alloweds, check if the first
		// node of the expression starts a declaration.
		if(onlyDeclarations)
		{
			assert(!ret.expired());
			auto expr = std::static_pointer_cast <Expression> (ret.lock());

			if(expr->getType() != Expression::Type::VariableRoot)
			{
				SourceLocation location(ctx.source, token);
				ctx.client.sourceError(location, "Only declarations are allowed here");
				return {};
			}
		}

		return ret;
	}

	return weak_from_this();
}

const std::vector <std::shared_ptr <Node>>& Scope::getNested()
{
	return nested;
}

const char* Scope::getTypeString() const
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
