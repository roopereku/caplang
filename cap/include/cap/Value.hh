#ifndef CAP_VALUE_HH
#define CAP_VALUE_HH

#include <cap/Expression.hh>

namespace cap
{

class Declaration;

class Value : public Expression
{
public:
	Value(std::wstring&& value);

	/// Should never be called as values cannot be the "current node".
	///
	/// \return False.
	bool isComplete() const override;

	const std::wstring& getValue();

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
