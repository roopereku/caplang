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
		Negate
	};

	OneSidedOperator(Token&& token, Type type)
		: Operator(std::move(token)), type(type)
	{
	}

	bool applyCached(std::shared_ptr <Expression>&& cached) override;

	const char* getTypeString() override;
	unsigned getPrecedence() override;

	bool isOneSided() override;

	std::shared_ptr <Expression> getExpression()
	{
		return expression;
	}

protected:
	bool handleLowerPrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleValue(std::shared_ptr <Value> value, ParserState& state) override;

	std::shared_ptr <Expression> expression;
	Type type;
};

}

#endif
