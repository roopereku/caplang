#ifndef CAP_SCOPE_HH
#define CAP_SCOPE_HH

#include <cap/Node.hh>

#include <string>
#include <vector>

namespace cap
{

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

protected:
	/// If no nested nodes exist, the first is initialized. Otherwise the
	/// given node is appended after the last nested node.
	std::weak_ptr <Node> appendNested(std::shared_ptr <Node>&& node);

	std::wstring name;
	std::vector <std::shared_ptr <Node>> nested;

	bool requiresBrackets;
	bool onlyDeclarations;
};

}

#endif
