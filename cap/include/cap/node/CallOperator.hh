#ifndef CAP_NODE_CALL_OPERATOR_HH
#define CAP_NODE_CALL_OPERATOR_HH

#include <cap/node/OneSidedOperator.hh>

namespace cap
{

class CallOperator : public OneSidedOperator
{
public:
	CallOperator(Token& at)
		: OneSidedOperator(token, OneSidedOperator::Type::Call)
	{
	}

	void setTarget(std::shared_ptr <Expression>&& node)
	{
		target = std::move(node);
	}

	std::shared_ptr <Expression> getTarget()
	{
		return target;
	}

private:
	std::shared_ptr <Expression> target;
};

}

#endif
