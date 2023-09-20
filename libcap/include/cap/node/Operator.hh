#ifndef CAP_NODE_OPERATOR_HH
#define CAP_NODE_OPERATOR_HH

#include <cap/ParserState.hh>

#include <cap/node/Expression.hh>
#include <cap/node/Value.hh>

namespace cap
{

class Operator : public Expression
{
public:
	Operator(Token&& token) : Expression(std::move(token))
	{
	}

	bool handleToken(Token&& token, ParserState& state) final override;

	virtual bool applyCached(std::shared_ptr <Expression>&& cached) = 0;

	virtual const char* getTypeString() = 0;
	virtual unsigned getPrecedence() = 0;

	virtual bool isTwoSided()
	{
		return false;
	}

	virtual bool isOneSided()
	{
		return false;
	}

	bool isOperator() final override
	{
		return true;
	}

	bool handleExpressionNode(std::shared_ptr <Expression> node, ParserState& state) override;

protected:
	virtual bool handleLowerPrecedence(std::shared_ptr <Operator> op, ParserState& state) = 0;
	virtual bool handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state) = 0;
	virtual bool handleValue(std::shared_ptr <Expression> value, ParserState& state) = 0;
};

}

#endif
