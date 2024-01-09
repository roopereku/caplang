#include <cap/node/TypeDefinition.hh>

namespace cap
{

static std::shared_ptr <TypeDefinition> int64 = std::make_shared <TypeDefinition> (
	Token(Token::Type::Identifier, "int64", 0, 0), 8
);

std::shared_ptr <TypeDefinition> TypeDefinition::getPrimitive(Token token)
{
	switch(token.getType())
	{
		case Token::Type::Hexadecimal:
		case Token::Type::Integer:
		case Token::Type::Binary:
		{
			return int64;
		}

		default: return nullptr;
	}
}

void TypeDefinition::ensurePrimitivesAdopted()
{
	auto adoptPrimitive = [](std::shared_ptr <TypeDefinition> node)
	{
		auto sharedScope = getShared();
		sharedScope->adopt(node);

		if(sharedScope->getRoot())
		{
			sharedScope->getRoot()->findLast()->setNext(node);
		}

		else
		{
			sharedScope->initializeRoot(std::move(node));
		}
	};

	static bool adopted = false;

	if(!adopted)
	{
		adoptPrimitive(int64);

		adopted = true;
	}
}

}
