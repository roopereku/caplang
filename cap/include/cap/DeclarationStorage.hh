#ifndef CAP_DECLARATION_STORAGE_HH
#define CAP_DECLARATION_STORAGE_HH

#include <cap/Variable.hh>

#include <vector>

namespace cap
{

class Declaration;
class ParserContext;
class Expression;

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
	/// \param ctx The context to get the source and client from.
	/// \param node The declaration node to add.
	/// \return True if the declaration was added successfully.
	bool add(cap::ParserContext& ctx, std::shared_ptr <Declaration> node);

	// TODO: Delete createVariable when the creation of declaration is handled in Variable::Root.

	/// Creates a new variable based on an expression node.
	///
	/// \param ctx The context to get the source and client from.
	/// \param node The expression node to create a variable from.
	/// \param node The type of the variable to create.
	/// \return True if a variable was created successfully.
	bool createVariable(cap::ParserContext& ctx, std::shared_ptr <Expression> node, Variable::Type type);

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

private:
	bool canAddDeclaration(std::shared_ptr <Declaration> node);
};

}

#endif
