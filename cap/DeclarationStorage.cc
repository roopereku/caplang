#include <cap/DeclarationStorage.hh>
#include <cap/BinaryOperator.hh>
#include <cap/ParserContext.hh>
#include <cap/Value.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

bool DeclarationStorage::add(cap::ParserContext& ctx, std::shared_ptr <Declaration> node)
{
	assert(isValid());

	// Make sure that the declaration doesn't conflict with anything in this
	// scope or the builtin scope.
	if(!canAddDeclaration(node) || !ctx.client.getBuiltin().getGlobal()->declarations.canAddDeclaration(node))
	{
		// TODO: Indicate where the declaration already exists?
		SourceLocation location(ctx.source, node->getToken());
		ctx.client.sourceError(location, "'", node->getName(), "' already exists");
		return false;
	}

	// TODO: Should callers always be responsible of adoption?
	assert(!node->getParent().expired());
	declarations.emplace_back(std::move(node));

	return true;
}

bool DeclarationStorage::isValid()
{
	return this != &getInvalid();
}

DeclarationStorage& DeclarationStorage::getInvalid()
{
	static DeclarationStorage invalid;
	return invalid;
}

bool DeclarationStorage::canAddDeclaration(std::shared_ptr <Declaration> node)
{
	assert(isValid());

	// Make sure that no other declaration of the same name already exists
	// within this scope. Some cases are allowed.
	for(auto decl : declarations)
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
