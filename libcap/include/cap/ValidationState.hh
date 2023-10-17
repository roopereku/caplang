#ifndef CAP_VALIDATION_STATE_HH
#define CAP_VALIDATION_STATE_HH

#include <cap/EventEmitter.hh>

namespace cap
{

class ValidationState
{
public:
	ValidationState(EventEmitter& events) : events(events)
	{
	}

	bool inVariable = false;
	bool findMembersInParent = false;

	EventEmitter& events;
};

};

#endif
