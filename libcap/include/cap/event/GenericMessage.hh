#ifndef CAP_EVENT_GENERIC_MESSAGE_HH
#define CAP_EVENT_GENERIC_MESSAGE_HH

#include <cap/event/Message.hh>

namespace cap
{

class GenericMessage : public Message
{
public:
	GenericMessage(Token at, std::string&& msg, Message::Type type)
		: Message(at), msg(std::move(msg)), type(type)
	{
	}

	Message::Type getType() override
	{
		return type;
	}

	std::string getString() override
	{
		return msg;
	}

private:
	std::string msg;
	Message::Type type;
};

}

#endif
