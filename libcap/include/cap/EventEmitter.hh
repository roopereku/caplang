#ifndef CAP_EVENT_EMITTER_HH
#define CAP_EVENT_EMITTER_HH

#include <cap/Token.hh>

#include <string>
#include <string_view>

namespace cap
{

class Event
{
public:
	Event(Token at) : at(at)
	{
	}

	virtual bool isMessage()
	{
		return false;
	}

	const Token& getLocation()
	{
		return at;
	}

protected:
	const Token at;
};

class Message;

class EventEmitter
{
public:
	void emit(Event&& event);
	void setCurrentFile(std::string_view name);

protected:
	virtual void onMessageReceived(Message& message);

	std::string_view currentFile;
};

}

#endif
