#ifndef CAP_EVENT_MESSAGE_HH
#define CAP_EVENT_MESSAGE_HH

#include <cap/Event.hh>

#include <string>

namespace cap
{

class Message : public Event
{
public:
	enum class Type
	{
		Debug,
		Error
	};

	Message(Type type, std::string&& str, Token at)
		: Event(Event::Type::Message, at), type(type), contents(std::move(str))
	{
	}

	/// The type of the message.
	const Type type;

	/// The message contents.
	const std::string contents;
};

}

#endif
