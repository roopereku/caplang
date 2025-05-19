#ifndef CAP_SOURCE_HH
#define CAP_SOURCE_HH

#include <cap/Token.hh>
#include <cap/Scope.hh>

#include <string_view>
#include <memory>

namespace cap
{

class Source
{
public:
	// TODO: Add a mechanism for loading parts of the source within the given range.
	// This is to support only storing chunks of the entire source to save memory.
	Source(std::wstring&& src);

	/// Parses and optionally validates this source file.
	///
	/// \return The global scope.
	bool parse(Client& client, bool validate = true);

	/// Gets the global scope of this source.
	///
	/// \return The global scope.
	std::shared_ptr <Scope> getGlobal();

	/// Gets the global scope of this source as a const.
	///
	/// \return The global scope as a const.
	const std::shared_ptr <Scope> getGlobal() const;

	/// Retrieves a character from the source.
	///
	/// \param index The index to get a character at.
	/// \return The character at the given index.
	wchar_t operator[](size_t index) const;

	/// Gets a string represented by a token.
	///
	/// \param token The token determining the range of the string.
	/// \return String containing characters in a range determined by a token.
	std::wstring getString(Token token) const;

	/// Matches the contents of the source within a token.
	///
	/// \param token The token determining a range within the source.
	/// \param value The string to compare against.
	/// \return True if the range is identical to the given string.
	bool match(Token token, std::wstring_view value) const;

protected:
	std::wstring src;

private:
	std::shared_ptr <Scope> global;
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
	const Source& source;
	Token at;

	unsigned row;
	unsigned column;
};

}

#endif
