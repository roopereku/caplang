#ifndef CAP_NODE_SCOPE_DEFINITON_HH
#define CAP_NODE_SCOPE_DEFINITON_HH

#include <cap/Node.hh>
#include <cap/Token.hh>

namespace cap
{

class ScopeDefinition : public Node
{
public:
	enum class Type
	{
		FunctionDefinition,
		TypeDefinition,
		None
	};

	ScopeDefinition(Type type, Token name);
	ScopeDefinition();

	std::shared_ptr <Node> findDefinition(Token name);

	std::shared_ptr <Node> getRoot() const;
	void initializeRoot(std::shared_ptr <Node>&& node);

	const char* getTypeString() override;

	/// Checks if this scope has the given name.
	///
	/// \param name The name of the definition.
	/// \return This scope.
	std::shared_ptr <Node> isDefinition(Token name) override;

	const Type type;
	const Token name;

private:
	std::shared_ptr <Node> root;
};

}

#endif
