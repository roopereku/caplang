#ifndef CAP_EVENT_EMITTER_HH
#define CAP_EVENT_EMITTER_HH

#include <cap/Token.hh>

#include <string>

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

protected:
	const Token at;
};

class Message;

class EventEmitter
{
public:
	void emit(Event&& event);

protected:
	virtual void onMessageReceived(Message& message);
};

}

#endif
