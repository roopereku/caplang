#ifndef CAP_DECLARATION_HH
#define CAP_DECLARATION_HH

#include <cap/Expression.hh>
#include <cap/TypeContext.hh>

#include <optional>

namespace cap
{

class Validator;

class Declaration : public Node
{
public:
	enum class Type
	{
		TypeDefinition,
		Function,
		Variable
	};

	class Root;

	/// Gets the name of this declaration.
	///
	/// \return The name of this declaration.
	const std::wstring& getName() const;

	/// Gets the type of this declaration.
	///
	/// \return The type of this declaration.
	Type getType();

	/// Gets the type referred to by this declaration.
	///
	/// \return The type referred to by this declaration.
	const TypeContext& getReferredType() const;

	/// Validates this declaration.
	///
	/// \param validator The validator used for traversal.
	/// \return True if validation succeeded.
	virtual bool validate(Validator& validator) = 0;

	/// Gets the full location of this declaration.
	///
	/// \param delimiter The delimiter to put between names.
	/// \return The full location delimited by a delimiter.
	std::wstring getLocation(wchar_t delimiter = '.') const;

	/// Gets the declaration that this declaration resides in.
	///
	/// \return The parent declaration if any.
	std::shared_ptr <Declaration> getParentDeclaration() const;

protected:
	Declaration(Type type);

	/// Implementation defined type such as the referred type of an
	/// alias, variable or a callable type with the signature for functions.
	std::optional <TypeContext> referredType;

	std::wstring name;

private:
	Type type;
};

}

#endif
