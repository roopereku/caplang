#ifndef CAP_EVENT_ERROR_MESSAGE_HH
#define CAP_EVENT_ERROR_MESSAGE_HH

#include <cap/event/Message.hh>

namespace cap
{

class ErrorMessage : public Message
{
public:
	ErrorMessage(std::string&& str, Token at)
		: Message(Type::Error, std::move(str), at)
	{
	}
};

}

#endif
