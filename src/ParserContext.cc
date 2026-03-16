#include <cap/ParserContext.hh>
#include <cap/Client.hh>

namespace cap
{

ParserContext::ParserContext(Client& client, Source& source)
	: m_client(client), m_source(source)
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
	: m_range(start, 0), m_depth(depth)
{
}

void Node::ParserContext::storeAttribute(std::shared_ptr <Attribute> attribute)
{
	size_t index = m_client.addAttribute(attribute);
	m_inAttribute = false;

	if(m_activeAttributes.empty() || m_activeAttributes.top().m_depth < m_subExpressionDepth)
	{
		m_activeAttributes.emplace(ActiveAttributes(index, m_subExpressionDepth));
	}

	m_activeAttributes.top().m_range.second++;
}

void Node::ParserContext::setMoreThanAttributes()
{
	// allowExpressionEndingInAttributes only makes a difference
	// at the root level of an expression.
	if(m_subExpressionDepth == 0)
	{
		m_allowExpressionEndingInAttributes = false;
	}
}

}
