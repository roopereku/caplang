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

	/// Sets the call target node.
	///
	/// \param The expression node to set as call target.
	void setTarget(std::shared_ptr <Expression>&& node);

	/// Gets the call target node.
	///
	/// \return The call target node.
	std::shared_ptr <Expression> getTarget();

private:
	std::shared_ptr <Expression> target;
};

}

#endif
