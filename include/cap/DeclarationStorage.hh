#ifndef CAP_DECLARATION_STORAGE_HH
#define CAP_DECLARATION_STORAGE_HH

#include <cap/Variable.hh>

#include <vector>

namespace cap
{

class Declaration;
class Expression;
class Validator;

class DeclarationStorage
{
private:
	std::vector <std::shared_ptr <Declaration>> declarations;

public:
	DeclarationStorage() {}

	// Forbid copying.
	DeclarationStorage(const DeclarationStorage& rhs) = delete;

	/// Adds a new declaration into this scope.
	///
	/// \param node The declaration node to add.
	void add(std::shared_ptr <Declaration> node);

	/// Checks if this declaration storage contains a declaration which
	/// is equivalent to the given one. Error are logged if any.
	///
	/// \param node The declaration to match against.
	/// \param validator Used to ensure that all compared declarations have been validated if needed.
	/// \return True if an equivalent declaration is found.
	bool checkEquivalent(std::shared_ptr <Declaration> node, Validator& validator) const;

	/// Checks if this declaration storage is valid.
	///
	/// \return True if this declaration storage isn't the invalid representation.
	bool isValid();

	/// Gets a declaration storage that represents an invalid state.
	///
	/// \return An invalid declaration storage.
	static DeclarationStorage& getInvalid();

	decltype(declarations)::iterator begin()
	{
		return declarations.begin();
	}

	decltype(declarations)::iterator end()
	{
		return declarations.end();
	}

	decltype(declarations)::const_iterator begin() const
	{
		return declarations.begin();
	}

	decltype(declarations)::const_iterator end() const
	{
		return declarations.end();
	}
};

}

#endif
