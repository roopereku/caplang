#include <cap/Type.hh>

namespace cap
{

std::weak_ptr <Node> Type::handleToken(ParserContext& ctx, Token& token)
{
	return weak_from_this();
}

}
