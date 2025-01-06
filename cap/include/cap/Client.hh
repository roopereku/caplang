#ifndef CAP_CLIENT_HH
#define CAP_CLIENT_HH

#include <cap/Source.hh>

#include <string>
#include <sstream>

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

protected:
	virtual void onSourceError(SourceLocation& at, const std::wstring& error);
	virtual void onError(const std::wstring& error);
};

}

#endif
