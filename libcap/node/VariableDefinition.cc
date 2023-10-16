#include <cap/node/VariableDefinition.hh>

namespace cap
{

VariableDefinition::VariableDefinition(Variable&& variable)
	: Declaration(Token(variable.getName())), variable(std::move(variable))
{
}

}
