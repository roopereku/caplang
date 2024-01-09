#include <cap/node/ParameterDefinition.hh>
#include <cap/node/TwoSidedOperator.hh>

#include <cassert>

namespace cap
{

ParameterDefinition::ParameterDefinition(std::shared_ptr <Value> name, std::shared_ptr <Expression> initialization)
	: ExpressionRoot(ExpressionRoot::Type::ParameterDefinition, name->token, initialization), name(name)
{
}

bool ParameterDefinition::isDefinition(Token name)
{
	return this->name->token == name;
}

const char* ParameterDefinition::getTypeString()
{
	return "Parameter";
}

}
