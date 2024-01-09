#ifndef CAP_NODE_INITIALIZATION_ROOT_HH
#define CAP_NODE_INITIALIZATION_ROOT_HH

#include <cap/node/ExpressionRoot.hh>
#include <cap/node/Value.hh>

namespace cap
{

/// InitializationRoot is the root node for unvalidated initializations
/// such as variables and aliases before they are expanded to their own nodes.
class InitializationRoot : public ExpressionRoot
{
public:
	enum class Type
	{
		Alias,
		Variable,
		Parameter,
	};

	InitializationRoot(Token& token);
	InitializationRoot(Token& token, Type type);

	/// Gets the initialization root type from a token.
	///
	/// \param token The token to determine type from.
	/// \return The initialization root type.
	static Type getType(Token& token);

	/// Creates a new definition depending on the current type.
	///
	/// \param name The value node containing the definition name.
	/// \param initialization The expression node containing the initialization.
	/// \return The newly created definition node.
	std::shared_ptr <ExpressionRoot> createDefinition(std::shared_ptr <Value> name,
														std::shared_ptr <Expression> initialization);	

	const Type type;
};

}

#endif
