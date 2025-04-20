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
	if(!builtin.getGlobal())
	{
		assert(parse(builtin));
		builtin.doCaching();
	}

	return builtin;
}

void Client::onSourceError(SourceLocation&, const std::wstring&) {}
void Client::onError(const std::wstring&) {}
void Client::onDebugMessage(const std::wstring&) {}

}
