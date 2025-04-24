#include <cap/DeclarationStorage.hh>
#include <cap/BinaryOperator.hh>
#include <cap/ParserContext.hh>
#include <cap/Value.hh>
#include <cap/Client.hh>

namespace cap
{

DeclarationStorage::Range DeclarationStorage::iterateDeclarations()
{
	return Range(*this);
}

bool DeclarationStorage::addDeclaration(cap::ParserContext& ctx, std::shared_ptr <Declaration> node)
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

	// TODO: Should this be done?
	//adopt(node);
	declarations.emplace_back(std::move(node));

	return true;
}

bool DeclarationStorage::createVariable(cap::ParserContext& ctx, std::shared_ptr <Expression> node, Variable::Type type)
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

			auto decl =  std::make_shared <Variable> (type, op);
			auto name = std::static_pointer_cast <Value> (op->getLeft());

			name->setReferred(decl);
			return addDeclaration(ctx, std::move(decl));
		}
	}

	SourceLocation location(ctx.source, node->getToken());
	ctx.client.sourceError(location, "Expected '='");
	return false;
}

bool DeclarationStorage::canAddDeclaration(std::shared_ptr <Declaration> node)
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

}
