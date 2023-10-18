#ifndef CAP_EVENT_INVALID_ACCESS_HH
#define CAP_EVENT_INVALID_ACCESS_HH

#include <cap/event/Message.hh>
#include <cap/node/DeclarationReference.hh>

namespace cap
{

class InvalidAccess : public Message
{
public:
	InvalidAccess(Token at, std::shared_ptr <DeclarationReference> accessed)
		: Message(at), accessed(accessed)
	{
	}

	Type getType() override
	{
		return Type::Error;
	}

	std::string getString() override
	{
		if(accessed->getDeclaration()->isFunction())
		{
			return "Invalid access to function " + accessed->getDeclaration()->getName().getString();
		}

		return "Invalid access";
	}

private:
	std::shared_ptr <DeclarationReference> accessed;
};

}

#endif
