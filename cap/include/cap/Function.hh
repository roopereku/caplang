#ifndef CAP_FUNCTION_HH
#define CAP_FUNCTION_HH

#include <cap/Scope.hh>
#include <cap/Expression.hh>

namespace cap
{

/// Function is a node representing a scope with a
/// function signature.
class Function : public Scope
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

private:
	std::shared_ptr <Expression::Root> signature;
};

}

#endif
