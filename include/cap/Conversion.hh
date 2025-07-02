#ifndef CAP_CONVERSION_HH
#define CAP_CONVERSION_HH

#include <cap/Function.hh>

namespace cap
{

class Conversion : public Function
{
public:
	enum class Type
	{
		BinaryOperator,
		UnaryOperator,
		BracketOperator,
		TypeConversion,
		Invalid
	};

	/// Assigns the context of the conversion such as the operands and types.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return The conversion context root or this conversion.
	std::weak_ptr <Node> handleToken(ParserContext& ctx, Token& token) override;

	/// Determines the type of the conversion and creates implicit parameters.
	///
	/// \param ctx The parser context.
	/// \param token The token triggering this function.
	/// \return This node or the result of Function::invokedNodeExited.
	std::weak_ptr <Node> invokedNodeExited(ParserContext& ctx, Token& token) override;

	const char* getTypeString() const override;

private:
	std::shared_ptr <Expression::Root> context;
	Type type = Type::Invalid;
};

}

#endif
