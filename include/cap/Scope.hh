#ifndef CAP_SCOPE_HH
#define CAP_SCOPE_HH

#include <cap/Node.hh>
#include <cap/Variable.hh>
#include <cap/DeclarationStorage.hh>

namespace cap
{

class Declaration;
class Expression;

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
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

	/// Handles scope exit.
	///
	/// \param ctx The parser context.
	/// \param token The token triggering this function.
	/// \return This scope or the result of the parent node upon a scope exit.
	std::weak_ptr <Node> invokedNodeExited(ParserContext& ctx, Token& token) override;

	/// Starts the parsing of a scope and checks for possible errors.
	///
	/// \param ctx The parser context.
	/// \param token The token starting the scope.
	/// \param onlyDeclarations True if the scope only allows declarations.
	/// \return New scope or null on an error.
	static std::shared_ptr <Scope> startParsing(ParserContext& ctx, Token& token, bool onlyDeclarations);

	/// Gets the nested nodes within this scope.
	///
	/// \return The nested nodes.
	const std::vector <std::shared_ptr <Node>>& getNested();

	const char* getTypeString() const override;

	/// The nested declarations within this scope.
	DeclarationStorage declarations;

private:
	/// If no nested nodes exist, the first is initialized. Otherwise the
	/// given node is appended after the last nested node.
	std::weak_ptr <Node> appendNested(std::shared_ptr <Node> node, Token& token);

	std::vector <std::shared_ptr <Node>> nested;
	bool onlyDeclarations;
};

}

#endif
