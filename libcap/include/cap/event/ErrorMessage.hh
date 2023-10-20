#ifndef CAP_EVENT_ERROR_MESSAGE_HH
#define CAP_EVENT_ERROR_MESSAGE_HH

#include <cap/event/Message.hh>

namespace cap
{

class ErrorMessage : public Message
{
public:
	ErrorMessage(Token at, std::string&& msg)
		: Message(at, std::move(msg), Message::Type::Error)
	{
	}

	const char* getPrefix() override
	{
		return "Error";
	}
};

}

#endif
