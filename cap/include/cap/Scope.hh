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
	enum class Type
	{
		Standalone,
		Function,
		ClassType,
		Custom
	};

	/// Constructs a global scope.
	Scope();

	/// Constructs a non-global scope.
	///
	/// \param onlyDeclarations If true, only declarations are allowed.
	Scope(Type type, bool onlyDeclarations);

	/// Checks if the token represents a nested node
	/// and constructs new nodes based on the input.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This scope, a new nested node or the parent node.
	virtual std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

	/// Gets the type of this scope.
	///
	/// \return The type of this scope.
	Type getType();

	/// Gets the nested nodes within this scope.
	///
	/// \return The nested nodes.
	const std::vector <std::shared_ptr <Node>>& getNested();

	/// Gets the name of this scope.
	///
	/// \return The name of this scope.
	const std::wstring& getName();

	const char* getTypeString();

protected:
	/// If no nested nodes exist, the first is initialized. Otherwise the
	/// given node is appended after the last nested node.
	std::weak_ptr <Node> appendNested(std::shared_ptr <Node> node);

	std::wstring name;
	std::vector <std::shared_ptr <Node>> nested;

	bool requiresBrackets;
	bool onlyDeclarations;

	Type type;
};

}

#endif
