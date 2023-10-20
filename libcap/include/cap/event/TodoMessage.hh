#ifndef CAP_EVENT_TODO_MESSAGE_HH
#define CAP_EVENT_TODO_MESSAGE_HH

#include <cap/event/Message.hh>

namespace cap
{

class TodoMessage : public Message
{
public:
	TodoMessage(Token at, std::string&& msg)
		: Message(at, std::move(msg), Message::Type::Error)
	{
	}

	const char* getPrefix() override
	{
		return "TODO";
	}
};

}

#endif
