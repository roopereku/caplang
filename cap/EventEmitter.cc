#include <cap/EventEmitter.hh>

namespace cap
{

void EventEmitter::setCurrentFile(std::string_view name)
{
	currentFile = name;
}

}
