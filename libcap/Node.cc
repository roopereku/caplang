#include <cap/Node.hh>

namespace cap
{

void Node::associateToken(Token&& token)
{
	tokens.emplace_back(std::move(token));
}

}
