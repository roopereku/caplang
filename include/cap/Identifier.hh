#ifndef CAP_IDENTIFIER_HH
#define CAP_IDENTIFIER_HH

#include <cap/Value.hh>

namespace cap
{

class Declaration;

class Identifier : public Value
{
public:
	Identifier(std::wstring&& value);

	/// Gets the identifier name as a string.
	///
	/// \return The identifier name as a string.
	const std::wstring& getValue() const;

	/// Gets the contained string value.
	/// Same as calling getValue.
	///
	/// \return The contained string value.
	std::wstring getString() const override;

	/// Returns the declaration that this value refers to if any.
	///
	/// \return The referred declaration or null.
	std::shared_ptr <Declaration> getReferred();

	/// Sets the declaration that this value refers to.
	///
	/// \param node The declaration to refer to.
	void setReferred(std::shared_ptr <Declaration> node);

    /// Updates the result type to that of the referred declaration.
    void updateResultType();

	const char* getTypeString() const override;

private:
	std::wstring value;
	std::weak_ptr <Declaration> referred;
};

}

#endif
