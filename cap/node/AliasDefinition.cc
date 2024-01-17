#include <cap/node/AliasDefinition.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cassert>

namespace cap
{

AliasDefinition::AliasDefinition(std::shared_ptr <Value> name, std::shared_ptr <Expression> initialization)
	: ExpressionRoot(ExpressionRoot::Type::AliasDefinition, name->token, initialization), name(name)
{
}

bool AliasDefinition::isDefinition(std::string_view name)
{
	return this->name->token == name;
}

const char* AliasDefinition::getTypeString()
{
	return "Alias";
}

}
