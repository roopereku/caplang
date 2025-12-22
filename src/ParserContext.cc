#include <cap/ParserContext.hh>
#include <cap/Client.hh>

namespace cap
{

ParserContext::ParserContext(Client& client, Source& source)
	: client(client), source(source)
{
}

Token::ParserContext::ParserContext(Client& client, Source& source)
	: cap::ParserContext(client, source)
{
}

Node::ParserContext::ParserContext(Client& client, Source& source)
	: cap::ParserContext(client, source)
{
}

Node::ParserContext::ActiveAttributes::ActiveAttributes(size_t start, size_t depth)
	: range(start, 0), depth(depth)
{
}

void Node::ParserContext::storeAttributeUsage(std::shared_ptr <Attribute::Root> usage)
{
	DBG_MESSAGE(client, "ADDING ATTRIBUTE");
	size_t index = client.addAttributeUsage(usage);
	inAttribute = false;

	if(activeAttributes.empty() || activeAttributes.top().depth < subExpressionDepth)
	{
		DBG_MESSAGE(client, "ADD CHECKPOINT");
		activeAttributes.emplace(index, subExpressionDepth);
	}

	activeAttributes.top().range.second++;
}

void Node::ParserContext::setMoreThanAttributes()
{
	// allowExpressionEndingInAttributes only makes a difference
	// at the root level of an expression.
	if(subExpressionDepth == 0)
	{
		allowExpressionEndingInAttributes = false;
	}
}

}
