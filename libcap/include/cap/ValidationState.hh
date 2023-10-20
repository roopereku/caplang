#ifndef CAP_VALIDATION_STATE_HH
#define CAP_VALIDATION_STATE_HH

#include <cap/EventEmitter.hh>

namespace cap
{

class Type;

class ValidationState
{
public:
	ValidationState(EventEmitter& events) : events(events)
	{
	}

	bool inVariable = false;
	bool inFunction = false;
	bool findMembersInParent = false;

	Type* returnType = nullptr;
	EventEmitter& events;
};

};

#endif
