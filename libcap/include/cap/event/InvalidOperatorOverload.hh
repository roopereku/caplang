#ifndef CAP_EVENT_INVALID_OPERATOR_OVERLOAD_HH
#define CAP_EVENT_INVALID_OPERATOR_OVERLOAD_HH

#include <cap/event/ErrorMessage.hh>
#include <cap/node/Operator.hh>
#include <cap/Type.hh>

namespace cap
{

class InvalidOperatorOverload : public ErrorMessage
{
public:
	InvalidOperatorOverload(std::shared_ptr <Operator> op, cap::Type& target)
		: ErrorMessage(op->getToken(),
				std::string("Operator '") + op->getTypeString() + "' not overloaded for type " + target.getName().getString())
	{
	}
};

}

#endif
