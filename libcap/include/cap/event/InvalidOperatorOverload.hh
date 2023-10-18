#ifndef CAP_EVENT_INVALID_OPERATOR_OVERLOAD_HH
#define CAP_EVENT_INVALID_OPERATOR_OVERLOAD_HH

#include <cap/event/Message.hh>
#include <cap/Type.hh>

namespace cap
{

class InvalidOperatorOverload : public Message
{
public:
	InvalidOperatorOverload(Token at, cap::Type& target)
		: Message(at), target(target)
	{
	}

	Type getType() override
	{
		return Type::Error;
	}

	std::string getString() override
	{
		return "Operator '" + at.getString() + "' not overloaded for type " + target.getName().getString();
	}

private:
	cap::Type& target;
};

}

#endif
