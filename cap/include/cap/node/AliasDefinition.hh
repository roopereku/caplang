#ifndef CAP_NODE_ALIAS_DEFINITION_HH
#define CAP_NODE_ALIAS_DEFINITION_HH

#include <cap/node/ExpressionRoot.hh>
#include <cap/node/Value.hh>

namespace cap
{

class AliasDefinition : public ExpressionRoot
{
public:
	AliasDefinition(std::shared_ptr <Value> name, std::shared_ptr <Expression> initialization);

	/// Checks if a this alias is the given definition.
	/// 
	/// \param name The name of the alias definition.
	/// \return True if the given name matches this alias.
	bool isDefinition(std::string_view name) override;

	const char* getTypeString() override;

	const std::shared_ptr <Value> name;
};

}

#endif
