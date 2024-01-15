#include <cap/node/PrimitiveType.hh>

namespace cap
{

static std::shared_ptr <PrimitiveType> int64 = std::make_shared <PrimitiveType> (
	Token(Token::Type::Identifier, "int64", 0, 0), 8
);

static std::shared_ptr <PrimitiveType> voidType = std::make_shared <PrimitiveType> (
	Token(Token::Type::Identifier, "void", 0, 0), 0
);

PrimitiveType::PrimitiveType(Token name, size_t size)
	: TypeDefinition(name, TypeDefinition::Type::PrimitiveType)
{
}

std::shared_ptr <TypeDefinition> PrimitiveType::fromToken(const Token& token)
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

std::shared_ptr <TypeDefinition> PrimitiveType::getVoid()
{
	return voidType;
}

void PrimitiveType::ensurePrimitivesRegistered()
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
		adoptPrimitive(voidType);

		adopted = true;
	}
}

}
