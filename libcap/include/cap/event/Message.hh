#ifndef CAP_EVENT_MESSAGE_HH
#define CAP_EVENT_MESSAGE_HH

#include <cap/EventEmitter.hh>

#include <string>

namespace cap
{

class Message : public Event
{
public:
	enum class Type
	{
		Error
	};

	Message(Token at, std::string&& msg, Message::Type type)
		: Event(at), msg(std::move(msg)), type(type)
	{
	}

	virtual const char* getPrefix() = 0;

	Type getType()
	{
		return type;
	}

	const std::string& getString()
	{
		return msg;
	}

	bool isMessage() final override
	{
		return true;
	}

private:
	std::string msg;
	Message::Type type;
};

}

#endif
