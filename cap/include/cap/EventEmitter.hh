#ifndef CAP_EVENT_EMITTER_HH
#define CAP_EVENT_EMITTER_HH

#include <cap/Token.hh>
#include <cap/Event.hh>

#include <string>
#include <string_view>

namespace cap
{

class EventEmitter
{
public:
	virtual void emit(Event&& event) = 0;
	void setCurrentFile(std::string_view name);

protected:
	std::string_view currentFile;
};

}

#endif
