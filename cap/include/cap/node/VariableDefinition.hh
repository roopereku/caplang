#ifndef CAP_NODE_VARIABLE_DEFINITION_HH
#define CAP_NODE_VARIABLE_DEFINITION_HH

#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class VariableDefinition : public ExpressionRoot
{
public:
	VariableDefinition(Token& token);

	/// Checks if a variable within this node is of the given name.
	/// 
	/// \param name The name of the variable definition.
	/// \return The node containing a variable name if found.
	std::shared_ptr <Node> isDefinition(Token name) override;

private:
	std::shared_ptr <Expression> findVariableNode(std::shared_ptr <Expression> node, Token name);
};

}

#endif
