#include <cap/Client.hh>

namespace cap
{

bool Client::parse(Source& source)
{
	return source.parse(*this);
}

void Client::onSourceError(SourceLocation&, const std::wstring&) {}
void Client::onError(const std::wstring&) {}

}
