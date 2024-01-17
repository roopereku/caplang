#ifndef CAP_NODE_SCOPE_DEFINITON_HH
#define CAP_NODE_SCOPE_DEFINITON_HH

#include <cap/Node.hh>
#include <cap/Token.hh>

namespace cap
{

class Reference;

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

	Reference findDefinition(std::string_view name, std::shared_ptr <Node> exclusion);
	Reference findDefinition(std::string_view name);

	std::shared_ptr <Node> getRoot() const;
	void initializeRoot(std::shared_ptr <Node>&& node);

	void removeChildNode(std::shared_ptr <Node> node) override;

	const char* getTypeString() override;

	/// Checks if this scope has the given name.
	///
	/// \param name The name of the definition.
	/// \return True if the name of this scope matches the given name.
	bool isDefinition(std::string_view name) override;

	/// Checks if this scope is validated.
	///
	/// \returns True if validation is done.
	bool isValidationComplete();

	/// Makes isValidationComplete() return true.
	void complete();

	/// Gets the shared scope.
	///
	/// \return The shared scope.
	static std::shared_ptr <ScopeDefinition> getShared();

	const Type type;
	const Token name;

private:
	bool validationComplete = false;
	std::shared_ptr <Node> root;
};

}

#endif
