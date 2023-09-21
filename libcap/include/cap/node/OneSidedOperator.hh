#ifndef CAP_NODE_ONE_SIDED_OPERATOR_HH
#define CAP_NODE_ONE_SIDED_OPERATOR_HH

#include <cap/node/Operator.hh>

namespace cap
{

class OneSidedOperator : public Operator
{
public:
	enum class Type
	{
		FunctionCall,
		Subscript,

		Not,
		Negate,
		BitwiseNot,
	};

	OneSidedOperator(Token&& token, Type type)
		: Operator(std::move(token)), type(type)
	{
	}

	bool applyCached(std::shared_ptr <Expression>&& cached) override;

	const char* getTypeString() override;
	unsigned getPrecedence() override;
	bool isOneSided() override;

	bool affectsPreviousValue();
	bool affectsNextValue();

	std::shared_ptr <Expression> getExpression()
	{
		return expression;
	}

	/// Parses a one sided operator from the given token.
	///
	/// \param token The token to parse a one sided operator from.
	/// \state The current parser state.
	/// \returns New one sided operator or nullptr if the token has an invalid operator.
	static std::shared_ptr <Operator> parse(Token&& token, ParserState& state);

	friend class TwoSidedOperator;

	// FIXME: Make this private.
	Type type;

protected:
	/// Replaces the expression node with the given operator.
	bool replaceExpression(std::shared_ptr <Expression> node) override;

	bool handleSamePrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleValue(std::shared_ptr <Expression> value, ParserState& state) override;

	std::shared_ptr <Expression> expression;
};

}

#endif
