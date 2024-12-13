#include <cap/Client.hh>

namespace cap
{

void Client::parse(Source& source)
{
	source.parse(*this);
}

void Client::onSourceError(SourceLocation&, const std::wstring&) {}
void Client::onError(const std::wstring&) {}

}
