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

	/// Gets the signature of this function.
	///
	/// \return The signature of this function.
	std::shared_ptr <CallableType> getSignature() const;

	/// Gets the function body.
	///
	/// \return The function body.
	std::shared_ptr <Scope> getBody() const;

private:
	std::shared_ptr <CallableType> signature;
	std::shared_ptr <Scope> body;
};

}

#endif
