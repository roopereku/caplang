#ifndef CAP_BRACKET_OPERATOR_HH
#define CAP_BRACKET_OPERATOR_HH

#include <cap/Expression.hh>

namespace cap
{

class BracketOperator : public Expression
{
public:
	enum class Type
	{
		Call,
		Subscript,
		Generic
	};

	BracketOperator(Type type);

	/// Creates a bracket operator based on the given token.
	///
	/// \param ctx The parsing context to get the source from.
	/// \param token The token to parse a bracket operator from.
	/// \return New bracket operator or null.
	static std::shared_ptr <BracketOperator> create(cap::ParserContext& ctx, Token token);

	/// Sets the context for the this operator and returns the inner root.
	///
	/// \param node The context or the the inner expression.
	void handleValue(std::shared_ptr <Expression> node) override;

	/// Checks whether this bracket operator should steal a context.
	///
	/// \return True if this bracket operator should steal a context.
	bool isComplete() const override;

	/// Returns 17.
	///
	/// \return 17.
	unsigned getPrecedence() override;

	std::shared_ptr <Expression> getContext() const;
	std::shared_ptr <Root> getInnerRoot() const;

	/// Gets the type of this bracket operator.
	///
	/// \return The type of this bracket operator.
	Type getType() const;

	static const char* getTypeString(Type type);
	const char* getTypeString() const override;

private:
	std::shared_ptr <Expression> context;
	std::shared_ptr <Root> innerRoot;

	Type type;
};

}

#endif
