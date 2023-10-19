#ifndef CAP_EVENT_INVALID_OPERATOR_OVERLOAD_HH
#define CAP_EVENT_INVALID_OPERATOR_OVERLOAD_HH

#include <cap/event/Message.hh>
#include <cap/node/Operator.hh>
#include <cap/Type.hh>

namespace cap
{

class InvalidOperatorOverload : public Message
{
public:
	InvalidOperatorOverload(std::shared_ptr <Operator> op, cap::Type& target)
		: Message(op->getToken()), op(op), target(target)
	{
	}

	Type getType() override
	{
		return Type::Error;
	}

	std::string getString() override
	{
		return std::string("Operator '") + op->getTypeString() + "' not overloaded for type " + target.getName().getString();
	}

private:
	std::shared_ptr <Operator> op;
	cap::Type& target;
};

}

#endif
