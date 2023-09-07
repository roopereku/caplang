#ifndef CAP_NODE_TWO_SIDED_OPERATOR_HH
#define CAP_NODE_TWO_SIDED_OPERATOR_HH

#include <cap/node/Operator.hh>

namespace cap
{

class TwoSidedOperator : public Operator
{
public:
	enum class Type
	{
		Assignment,
		Addition,
		Multiplication,
	};

	TwoSidedOperator(Token&& token, Type type)
		: Operator(std::move(token)), type(type)
	{
	}

	bool handleToken(Token&& token, ParserState& state) override;
	bool applyCached(std::shared_ptr <Expression>&& cached) override;

	const char* getTypeString() override;
	unsigned getPrecedence() override;

	bool isTwoSidedOperator() override;

protected:
	std::shared_ptr <Expression> left;
	std::shared_ptr <Expression> right;

	Type type;
};

}

#endif
