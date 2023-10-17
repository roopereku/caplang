#ifndef CAP_EVENT_MESSAGE_HH
#define CAP_EVENT_MESSAGE_HH

#include <cap/EventEmitter.hh>

#include <string>

namespace cap
{

class Message : public Event
{
public:
	Message(Token at) : Event(at)
	{
	}

	enum class Type
	{
		Error
	};

	virtual Type getType() = 0;
	virtual std::string getString() = 0;

	bool isMessage() final override
	{
		return true;
	}
};

}

#endif
