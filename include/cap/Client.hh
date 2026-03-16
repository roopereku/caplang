#ifndef CAP_CLIENT_HH
#define CAP_CLIENT_HH

#include <cap/Source.hh>
#include <cap/Builtin.hh>
#include <cap/Attribute.hh>

#include <string>
#include <sstream>
#include <vector>

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
	/// Parser the given source using this client.
	///
	/// \param source The source to parse.
	/// \param validate True if should validation be performed.
	/// \return True if the parsing was successful.
	bool parse(Source& source, bool validate = true);

	/// Gets the builtin code wrapper.
	///
	/// \return The builtin code wrapper.
	const Builtin& getBuiltin();

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

	/// Adds an attribute to the attribute cache.
	///
	/// \param attr The attribute to add.
	/// \return The index at which the attribute was added.
	size_t addAttribute(std::shared_ptr <Attribute> attr);

	class AttributeRange;
	AttributeRange getAttributes(std::shared_ptr <Node> node) const;

protected:
	virtual void onSourceError(SourceLocation& at, const std::wstring& error);
	virtual void onError(const std::wstring& error);
	virtual void onDebugMessage(const std::wstring& msg);

private:
	Builtin builtin;
	std::vector <std::shared_ptr <Attribute>> attributes;
};

class Client::AttributeRange
{
public:
	using iterator = std::vector <std::shared_ptr <Attribute>>::const_iterator;

	AttributeRange(iterator start, iterator stop)
		: start(start), stop(stop)
	{
	}

	iterator cbegin() { return start; }
	iterator cend() { return stop; }
	iterator begin() { return start; }
	iterator end() { return stop; }

private:
	iterator start;
	iterator stop;
};

}

#endif
