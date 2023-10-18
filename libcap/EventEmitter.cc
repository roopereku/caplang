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

void EventEmitter::setCurrentFile(std::string_view name)
{
	currentFile = name;
}

void EventEmitter::onMessageReceived(Message& message)
{
}

}
