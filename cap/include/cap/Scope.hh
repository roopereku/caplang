#ifndef CAP_SCOPE_HH
#define CAP_SCOPE_HH

#include <cap/Node.hh>

#include <vector>

namespace cap
{

class Declaration;
class Variable;

/// Scope is a node representing an area within the source
/// code in which declarations, expressions and statements live in.
class Scope : public Node
{
public:
	/// Constructs a global scope.
	Scope();

	/// Constructs a non-global scope.
	///
	/// \param onlyDeclarations If true, only declarations are allowed.
	Scope(bool onlyDeclarations);

	/// Checks if the token represents a nested node
	/// and constructs new nodes based on the input.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This scope, a new nested node or the parent node.
	virtual std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

	/// Gets the nested nodes within this scope.
	///
	/// \return The nested nodes.
	const std::vector <std::shared_ptr <Node>>& getNested();

	/// Find a declaration of the given name within this
	/// scope or a parent scope.
	///
	/// \return The declaration if it is found.
	std::shared_ptr <Declaration> findDeclaration(Source& source, Token name);

	/// Adds a new declaration into this scope. NOTE: Duplicates are allowed.
	///
	/// \param variable The declaration node to add.
	void addDeclaration(std::shared_ptr <Declaration> node);

	const char* getTypeString() override;

protected:
	/// If no nested nodes exist, the first is initialized. Otherwise the
	/// given node is appended after the last nested node.
	std::weak_ptr <Node> appendNested(std::shared_ptr <Node> node);

	std::vector <std::shared_ptr <Node>> nested;
	std::vector <std::shared_ptr <Declaration>> declarations;

	bool onlyDeclarations;
};

}

#endif
