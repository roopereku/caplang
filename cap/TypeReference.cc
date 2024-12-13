#include <cap/TypeReference.hh>

namespace cap
{

std::weak_ptr <Node> TypeReference::handleToken(ParserContext& ctx, Token& token)
{
	return weak_from_this();
}

}
