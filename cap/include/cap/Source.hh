#ifndef CAP_SOURCE_HH
#define CAP_SOURCE_HH

#include <cap/Token.hh>
#include <cap/Scope.hh>

#include <string_view>
#include <memory>

namespace cap
{

/// Source provides an interface for accessing source code.
class Source
{
public:
	bool parse(Client& client);

	virtual wchar_t operator[](size_t index) const = 0;
	virtual std::wstring getString(Token token) const = 0;
	virtual bool match(Token token, std::wstring_view value) const = 0;

protected:
	Source();

	virtual bool canParse(Client& client);

private:
	std::shared_ptr <Scope> root;
};

class SourceLocation
{
public:
	SourceLocation(const Source& source, Token token);
	SourceLocation(const SourceLocation& rhs);

	std::wstring getString() const;
	void setToken(Token token);

	unsigned getRow() const;
	unsigned getColumn() const;

	Token getToken() const;
	const Source& getSource() const;

private:
	Token at;
	const Source& source;

	unsigned row;
	unsigned column;
};

}

#endif
