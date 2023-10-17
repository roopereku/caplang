#include <cap/EventEmitter.hh>
#include <cap/event/Message.hh>

namespace cap
{

void EventEmitter::emit(Event&& event)
{
	if(event.isMessage())
	{
		onMessageReceived(static_cast <Message&> (event));
	}
}

void EventEmitter::onMessageReceived(Message& message)
{
}

}
