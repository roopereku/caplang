#include <cap/DeclarationStorage.hh>
#include <cap/BinaryOperator.hh>
#include <cap/ParserContext.hh>
#include <cap/Validator.hh>
#include <cap/Function.hh>
#include <cap/Value.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

void DeclarationStorage::add(std::shared_ptr <Declaration> node)
{
	assert(isValid());

	// TODO: Should callers always be responsible of adoption?
	assert(!node->getParent().expired());
	declarations.emplace_back(std::move(node));
}

bool DeclarationStorage::checkEquivalent(std::shared_ptr <Declaration> node, Validator& validator) const
{
	for(const auto& decl : declarations)
	{
		if(decl == node || decl->getName() != node->getName())
		{
			continue;
		}

		// If both declarations are functions, they only equivalent if
		// the parameter types fully match.
		if(node->getType() == Declaration::Type::Function &&
			decl->getType() == Declaration::Type::Function)
		{
			auto funcNode = std::static_pointer_cast <Function> (node);
			auto funcDecl = std::static_pointer_cast <Function> (decl);

			if(!decl->validate(validator))
			{
				return false;
			}

			auto nodeParams = funcNode->getParameterRoot();
			auto [compatible, unidentical] = funcDecl->matchParameters(ArgumentAccessor(nodeParams));
			if(compatible && unidentical == 0)
			{
				// TODO: Give more context for the existing function?
				SourceLocation location(validator.getParserContext().source, node->getToken());
				validator.getParserContext().client.sourceError(location, "Function with the same parameters already exists");
				return true;
			}
		}

		else
		{
			// TODO: Indicate where the declaration already exists?
			SourceLocation location(validator.getParserContext().source, node->getToken());
			validator.getParserContext().client.sourceError(location, "'", node->getName(), "' already exists");
			return true;
		}
	}

	return false;
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

}
