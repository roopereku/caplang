#include <cap/Client.hh>

#include <cassert>

namespace cap
{

bool Client::parse(Source& source, bool validate)
{
	if(source.parse(*this, validate))
	{
		return true;
	}

	return false;
}

const Builtin& Client::getBuiltin()
{
	// Make sure that the builtins are ready.
	if(!m_builtin.getGlobal())
	{
		assert(parse(m_builtin));
		m_builtin.doCaching();
	}

	return m_builtin;
}

size_t Client::addAttribute(std::shared_ptr <Attribute> attr)
{
	size_t index = m_attributes.size();
	m_attributes.emplace_back(std::move(attr));
	return index;
}

Client::AttributeRange Client::getAttributes(std::shared_ptr <Node> node) const
{
	auto range = node->getAttributeRange();
	auto start = m_attributes.begin() + range.first;

	return AttributeRange(start, start + range.second);
}

void Client::onSourceError(SourceLocation&, const std::wstring&) {}
void Client::onError(const std::wstring&) {}
void Client::onDebugMessage(const std::wstring&) {}

}
