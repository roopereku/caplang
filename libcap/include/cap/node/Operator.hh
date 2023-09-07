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

	virtual bool applyCached(std::shared_ptr <Expression>&& cached) = 0;
	virtual const char* getTypeString() = 0;
	virtual unsigned getPrecedence() = 0;

	virtual bool isTwoSidedOperator()
	{
		return false;
	}

	virtual bool isOneSidedOperator()
	{
		return false;
	}

	bool isOperator() override
	{
		return true;
	}
};

}

#endif
