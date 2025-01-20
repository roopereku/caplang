#ifndef CAP_FUNCTION_HH
#define CAP_FUNCTION_HH

#include <cap/Scope.hh>
#include <cap/Declaration.hh>
#include <cap/Expression.hh>

namespace cap
{

/// Function is a node representing a scope with a
/// function signature.
class Function : public Declaration
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

	/// Gets the root of the expression representing the parameters if any.
	///
	/// \return The root of the parameter declaration expression or null.
	std::shared_ptr <Expression::Root> getParameterRoot();

	/// Gets the function body.
	///
	/// \return The function body.
	std::shared_ptr <Scope> getBody();

private:
	std::shared_ptr <Expression::Root> parameters;
	std::shared_ptr <Scope> body;
};

}

#endif
