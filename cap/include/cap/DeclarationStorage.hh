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
public:
	class Range;
	Range iterateDeclarations();

	/// Adds a new declaration into this scope.
	///
	/// \param ctx The context to get the source and client from.
	/// \param node The declaration node to add.
	/// \return True if the declaration was added successfully.
	bool addDeclaration(cap::ParserContext& ctx, std::shared_ptr <Declaration> node);

	// TODO: Delete createVariable when the creation of declaration is handled in Variable::Root.

	/// Creates a new variable based on an expression node.
	///
	/// \param ctx The context to get the source and client from.
	/// \param node The expression node to create a variable from.
	/// \param node The type of the variable to create.
	/// \return True if a variable was created successfully.
	bool createVariable(cap::ParserContext& ctx, std::shared_ptr <Expression> node, Variable::Type type);

private:
	bool canAddDeclaration(std::shared_ptr <Declaration> node);

	std::vector <std::shared_ptr <Declaration>> declarations;
};

class DeclarationStorage::Range
{
public:
	using Iterator = decltype(DeclarationStorage::declarations)::iterator;

	Range(DeclarationStorage& storage)
		: storage(storage)
	{
	}

	Iterator begin()
	{
		return storage.declarations.begin();
	}

	Iterator end()
	{
		return storage.declarations.end();
	}

private:
	DeclarationStorage& storage;
};

}

#endif
