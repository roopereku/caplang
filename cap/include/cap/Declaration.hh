#ifndef CAP_DECLARATION_HH
#define CAP_DECLARATION_HH

#include <cap/Expression.hh>
#include <cap/TypeContext.hh>

namespace cap
{

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
	const std::wstring& getName();

	/// Gets the type of this declaration.
	///
	/// \return The type of this declaration.
	Type getType();

	/// Gets the type referred to by this declaration.
	///
	/// \return The type referred to by this declaration.
	const TypeContext& getReferredType() const;

	const char* getTypeString() override;

protected:
	Declaration(Type type);

	/// Implementation defined type such as the referred type of an
	/// alias, variable or a callable type with the signature for functions.
	TypeContext referredType;

	std::wstring name;

private:
	Type type;
};

class Declaration::Root : public Expression::Root
{
public:
	enum class Type
	{
		Local,
		Generic,
		Parameter,

		// Used to control implicit declarations.
		None
	};

	Root(Type type);

	/// Find the scope which the declarations are done for.
	///
	/// \return The scope that the declarations target.
	std::shared_ptr <Scope> findTargetScope();

	/// Gets the type of this declaration root.
	///
	/// \return The type of this declaration root.
	Type getType() const;

	static const char* getTypeString(Type type);
	const char* getTypeString() override;

private:
	Type type;
};

}

#endif
