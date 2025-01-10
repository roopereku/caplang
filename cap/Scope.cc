#include <cap/Scope.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>
#include <cap/ClassType.hh>
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

	else if(ctx.source.match(token, L"type"))
	{
		return appendNested(std::make_shared <ClassType> ());
	}

	else if(token.isOpeningBracket(ctx, '{'))
	{
		// TODO: Implement subscopes.
	}

	// TODO: When it's guaranteed that no comments can appear here,
	// it's almost given that anything else relates to an expression.
	else if(token.canBeValue() || token.getType() == Token::Type::Operator || token.getType() == Token::Type::OpeningBracket)
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

			if(expr->getType() != Expression::Type::DeclarationRoot)
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

std::shared_ptr <Declaration> Scope::findDeclaration(Source& source, Token name)
{
	// Is the declaration in this scope.
	for(auto decl : declarations)
	{
		if(source.match(name, decl->getName()))
		{
			return decl;
		}
	}

	// Look for the declaration in a parent scope if there is any.
	auto parent = getParentScope();
	return parent ? parent->findDeclaration(source, name) : nullptr;
}

void Scope::addDeclaration(std::shared_ptr <Declaration> node)
{
	adopt(node);
	declarations.emplace_back(std::move(node));
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
