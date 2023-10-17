#ifndef CAP_EVENT_UNKNOWN_IDENTIFIER_HH
#define CAP_EVENT_UNKNOWN_IDENTIFIER_HH

#include <cap/event/Message.hh>
#include <cap/Scope.hh>

namespace cap
{

class UnknownIdentifier : public Message
{
public:
	UnknownIdentifier(Token at, Scope& in) : Message(at), scope(in)
	{
	}

	Type getType() override
	{
		return Type::Error;
	}

	std::string getString() override
	{
		return "Unknown identifier";
	}

private:
	Scope& scope;
};

}

#endif
