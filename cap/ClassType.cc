#include <cap/ClassType.hh>

namespace cap
{

ClassType::ClassType()
	: Scope(Type::ClassType, true)
{
}

std::weak_ptr <Node> ClassType::handleToken(ParserContext& ctx, Token& token)
{
	return weak_from_this();
}

}
