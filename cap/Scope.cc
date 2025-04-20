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

bool Scope::canAddDeclaration(std::shared_ptr <Declaration> node)
{
	// Make sure that no other declaration of the same name already exists
	// within this scope. Some cases are allowed.
	for(auto decl : iterateDeclarations())
	{
		if(node->getName() == decl->getName())
		{
			// Allow multiple functions of the same name.
			if(node->getType() == Declaration::Type::Function &&
				decl->getType() == Declaration::Type::Function)
			{
				continue;
			}

			return false;
		}
	}

	return true;
}

bool Scope::addDeclaration(cap::ParserContext& ctx, std::shared_ptr <Declaration> node)
{
	// Make sure that the declaration doesn't conflict with anything in this
	// scope or the builtin scope.
	if(!canAddDeclaration(node) || !ctx.client.getBuiltin().getGlobal()->canAddDeclaration(node))
	{
		// TODO: Indicate where the declaration already exists?
		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, "'", node->getName(), "' already exists");
		return false;
	}

	adopt(node);
	declarations.emplace_back(std::move(node));

	return true;
}

bool Scope::createDeclaration(cap::ParserContext& ctx, std::shared_ptr <Expression> node)
{
	if(node->getType() == Expression::Type::BinaryOperator)
	{
		auto op = std::static_pointer_cast <BinaryOperator> (node);
		if(op->getType() == BinaryOperator::Type::Assign)
		{
			if(op->getLeft()->getToken().getType() != Token::Type::Identifier)
			{
				SourceLocation location(ctx.source, op->getLeft()->getToken());
				ctx.client.sourceError(location, "Expected an identifier");
				return false;
			}

			return addDeclaration(ctx, std::make_shared <Variable> (op));
		}
	}

	SourceLocation location(ctx.source, node->getToken());
	ctx.client.sourceError(location, "Expected '='");
	return false;
}

Scope::DeclarationRange Scope::recurseDeclarations()
{
	auto scope = std::static_pointer_cast <Scope> (shared_from_this());
	return DeclarationRange(scope, true);
}

Scope::DeclarationRange Scope::iterateDeclarations()
{
	auto scope = std::static_pointer_cast <Scope> (shared_from_this());
	return DeclarationRange(scope, false);
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

Scope::DeclarationIterator::DeclarationIterator(std::shared_ptr <Scope> scope, bool recursive)
	: scope(scope), recursive(recursive)
{
	// Bad things will happen if we begin on an empty scope.
	handleScopeChange();
}

Scope::DeclarationIterator::reference Scope::DeclarationIterator::operator*() const
{
	if(scope)
	{
		assert(index < scope->declarations.size());
		return scope->declarations[index];
	}

	return nullptr;
}

Scope::DeclarationIterator::pointer Scope::DeclarationIterator::operator->() const
{
	if(scope)
	{
		assert(index < scope->declarations.size());
		return scope->declarations[index].get();
	}

	return nullptr;
}

Scope::DeclarationIterator& Scope::DeclarationIterator::operator++()
{
	advance();
	return *this;
}

Scope::DeclarationIterator Scope::DeclarationIterator::operator++(int)
{
	auto temp = *this;
	advance();
	return temp;
}

bool Scope::DeclarationIterator::operator==(const DeclarationIterator& rhs) const
{
	return scope == rhs.scope && index == rhs.index;
}

bool Scope::DeclarationIterator::operator!=(const DeclarationIterator& rhs) const
{
	return scope != rhs.scope || index != rhs.index;
}

void Scope::DeclarationIterator::advance()
{
	index++;
	handleScopeChange();
}

void Scope::DeclarationIterator::handleScopeChange()
{
	if(recursive)
	{
		// If the current scope no longer has any declarations, switch to the parent.
		// If the parent has no declarations, go even further. This could happen with
		// scopes within expression where the parent scope has no declarations.
		while(scope && index >= scope->declarations.size())
		{
			index = 0;
			scope = scope->getParentScope();
		}
	}

	// If no recursion will happen and there's no more declarations, stop iterating.
	else if(scope && index >= scope->declarations.size())
	{
		index = 0;
		scope = nullptr;
	}
}

Scope::DeclarationRange::DeclarationRange(std::shared_ptr <Scope> scope, bool recursive)
	: scope(scope), recursive(recursive)
{
}

Scope::DeclarationIterator Scope::DeclarationRange::begin() const
{
	assert(!scope.expired());
	return Scope::DeclarationIterator(scope.lock(), recursive);
}

Scope::DeclarationIterator Scope::DeclarationRange::end() const
{
	return Scope::DeclarationIterator(nullptr, recursive);
}

}
