#include <cap/node/VariableDefinition.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cassert>

namespace cap
{

VariableDefinition::VariableDefinition(std::shared_ptr <Value> name, std::shared_ptr <Expression> initialization)
	: ExpressionRoot(ExpressionRoot::Type::VariableDefinition, name->token, initialization), name(name)
{
}

bool VariableDefinition::isDefinition(std::string_view name)
{
	return this->name->token == name;
}

const char* VariableDefinition::getTypeString()
{
	return "Variable";
}

}
