#ifndef CAP_FUNCTION_HH
#define CAP_FUNCTION_HH

#include <cap/Scope.hh>
#include <cap/Declaration.hh>
#include <cap/Expression.hh>
#include <cap/CallableType.hh>

namespace cap
{

/// Function is a node representing a scope with a
/// function signature.
class Function : public Declaration, public CallableType
{
public:
	/// Constructs a function.
	Function();

	/// Assigns the name, signature and delegates anything
	/// else to the handler of scope.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return This function.
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

	/// Handles the end of parameters, return value and the body.
	///
	/// \param ctx The parser context.
	/// \param token The token triggering this function.
	/// \return This or the parent node.
	std::weak_ptr <Node> invokedNodeExited(ParserContext& ctx, Token& token) override;

	/// Gets the function body.
	///
	/// \return The function body.
	std::shared_ptr <Scope> getBody() const;

	/// Validates the signature and body of this function.
	///
	/// \param validator The validator used for traversal.
	/// \return True if validation succeeded.
	bool validate(Validator& validator) override;

	const char* getTypeString() const override;

	/// Declaration storage for function parameters.
	DeclarationStorage parameters;

private:
	std::shared_ptr <Scope> body;
};

}

#endif
