#include <cap/Scope.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/Expression.hh>

#include <cassert>

namespace cap
{

Scope::Scope()
	: Node(Node::Type::Scope), type(Type::Standalone), requiresBrackets(false), onlyDeclarations(true)
{
}

Scope::Scope(Type type, bool onlyDeclarations)
	: Node(Node::Type::Scope), type(type), requiresBrackets(true), onlyDeclarations(onlyDeclarations)
{
}

std::weak_ptr <Node> Scope::handleToken(ParserContext& ctx, Token& token)
{
	if(requiresBrackets)
	{
		// TODO: Implement the brace checker in a nicer way.
		// The first token of a scope isolated by
		// by brackets requires an opener.
		/*
		if(ctx.tokensProcessed == 0)
		{
			if(!token.isOpeningBracket(ctx, '{'))
			{
				SourceLocation location(ctx.source, token);
				ctx.client.sourceError(location, "Expected '{' to indicate the beginning of a scope");
				return {};
			}
		}

		else*/ if(token.isClosingBracket(ctx, '}'))
		{
			printf("Switch to parent of scope\n");
			assert(!getParent().expired());
			return getParent();
		}
	}

	if(ctx.source.match(token, L"func"))
	{
		return appendNested(std::make_shared <Function> ());
	}

	else if(token.isOpeningBracket(ctx, '{'))
	{
		// TODO: Implement scopes.
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

Scope::Type Scope::getType()
{
	return type;
}

const std::vector <std::shared_ptr <Node>>& Scope::getNested()
{
	return nested;
}

const std::wstring& Scope::getName()
{
	return name;
}

const char* Scope::getTypeString()
{
	switch(type)
	{
		case Type::Standalone: return "Standalone";
		case Type::Function: return "Function";
		case Type::ClassType: return "ClassType";
		case Type::Custom: return "Custom";
	}

	return "???";
}

std::weak_ptr <Node> Scope::appendNested(std::shared_ptr <Node> node)
{
	adopt(node);
	nested.emplace_back(std::move(node));

	return nested.back();
}

}
