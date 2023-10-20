#ifndef CAP_EVENT_UNKNOWN_IDENTIFIER_HH
#define CAP_EVENT_UNKNOWN_IDENTIFIER_HH

#include <cap/event/ErrorMessage.hh>
#include <cap/Scope.hh>

namespace cap
{

class UnknownIdentifier : public ErrorMessage
{
public:
	UnknownIdentifier(Token at, Scope& in)
		: ErrorMessage(at, "Unknown identifier " + at.getString())
	{
	}
};

}

#endif
