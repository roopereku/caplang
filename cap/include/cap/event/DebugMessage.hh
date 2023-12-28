#ifndef CAP_EVENT_DEBUG_MESSAGE_HH
#define CAP_EVENT_DEBUG_MESSAGE_HH

#include <cap/event/Message.hh>

namespace cap
{

class DebugMessage : public Message
{
public:
	DebugMessage(std::string&& str, Token at)
		: Message(Type::Debug, std::move(str), at)
	{
	}
};

}

#endif
