#include <cap/ClassType.hh>

namespace cap
{

ClassType::ClassType()
	: TypeDefinition(Type::Class)
{
}

std::weak_ptr <Node> ClassType::handleToken(ParserContext& ctx, Token& token)
{
	return weak_from_this();
}

}
