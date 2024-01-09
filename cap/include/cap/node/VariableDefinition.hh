#ifndef CAP_NODE_VARIABLE_DEFINITION_HH
#define CAP_NODE_VARIABLE_DEFINITION_HH

#include <cap/node/ExpressionRoot.hh>
#include <cap/node/Value.hh>

namespace cap
{

class VariableDefinition : public ExpressionRoot
{
public:
	VariableDefinition(std::shared_ptr <Value> name, std::shared_ptr <Expression> initialization);

	/// Checks if a this variable is the given definition.
	/// 
	/// \param name The name of the variable definition.
	/// \return True if the given name matches this variable.
	bool isDefinition(Token name) override;

	const char* getTypeString() override;

	const std::shared_ptr <Value> name;
};

}

#endif
