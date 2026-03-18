#ifndef CAP_CLIENT_HH
#define CAP_CLIENT_HH

#include <cap/Attribute.hh>
#include <cap/Builtin.hh>
#include <cap/Source.hh>

#include <sstream>
#include <string>
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
    size_t addAttribute(std::shared_ptr<Attribute> attr);

    class AttributeRange;
    AttributeRange getAttributes(std::shared_ptr<Node> node) const;

protected:
    virtual void onSourceError(SourceLocation& at, const std::wstring& error);
    virtual void onError(const std::wstring& error);
    virtual void onDebugMessage(const std::wstring& msg);

private:
    Builtin m_builtin;
    std::vector<std::shared_ptr<Attribute>> m_attributes;
};

class Client::AttributeRange
{
public:
    using iterator = std::vector<std::shared_ptr<Attribute>>::const_iterator;

    AttributeRange(iterator start, iterator stop) :
        m_start(start),
        m_stop(stop)
    {
    }

    iterator cbegin() { return m_start; }

    iterator cend() { return m_stop; }

    iterator begin() { return m_start; }

    iterator end() { return m_stop; }

private:
    iterator m_start;
    iterator m_stop;
};

} // namespace cap

#endif
