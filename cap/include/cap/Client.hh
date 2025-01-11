#ifndef CAP_CLIENT_HH
#define CAP_CLIENT_HH

#include <cap/Source.hh>

#include <string>
#include <sstream>

// TODO: Add DBG_MESSAGE_AT
#ifdef CAP_DEBUG
#define DBG_MESSAGE(client, ...) client.debugMessage(__VA_ARGS__)
#else
#define DBG_MESSAGE(client, ...)
#endif

namespace cap
{

/// Client is a convenience class for interfacing
/// with the parsing process.
class Client
{
public:
	bool parse(Source& source);

	template <typename... Args>
	void sourceError(SourceLocation& at, Args&&... args)
	{
		std::wostringstream ss;
		((ss << args), ...);
		onSourceError(at, ss.str());
	}

	template <typename... Args>
	void debugMessage(Args&&... args)
	{
		std::wostringstream ss;
		((ss << args), ...);
		onDebugMessage(ss.str());
	}

protected:
	virtual void onSourceError(SourceLocation& at, const std::wstring& error);
	virtual void onError(const std::wstring& error);
	virtual void onDebugMessage(const std::wstring& msg);
};

}

#endif
