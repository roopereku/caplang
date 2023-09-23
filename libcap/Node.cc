#include <cap/Node.hh>

namespace cap
{

bool Node::handleToken(Token&& token, ParserState& state)
{
	printf("handleToken called for Node\n");
	return false;
}

void Node::adopt(std::shared_ptr <Node> node)
{
	node->parent = shared_from_this();
}

std::shared_ptr <Node> Node::findLastNode()
{
	return next ? next->findLastNode() : shared_from_this();
}

}
