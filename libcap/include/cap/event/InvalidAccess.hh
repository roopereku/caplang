#ifndef CAP_EVENT_INVALID_ACCESS_HH
#define CAP_EVENT_INVALID_ACCESS_HH

#include <cap/event/ErrorMessage.hh>
#include <cap/node/DeclarationReference.hh>

namespace cap
{

class InvalidAccess : public ErrorMessage
{
public:
	InvalidAccess(Token at, std::shared_ptr <DeclarationReference> accessed)
		: ErrorMessage(at, std::move(toString(accessed)))
	{
	}

	static std::string toString(std::shared_ptr <DeclarationReference> accessed)
	{
		if(accessed->getDeclaration()->isFunction())
		{
			return "Invalid access to function " + accessed->getDeclaration()->getName().getString();
		}

		return "Invalid access";
	}
};

}

#endif
