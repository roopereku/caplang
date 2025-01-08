#ifndef CAP_DECLARATION_HH
#define CAP_DECLARATION_HH

#include <cap/Expression.hh>

namespace cap
{

class Declaration : public Node
{
public:
	enum class Type
	{
		ClassType,
		Function,
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

	const char* getTypeString() override;

protected:
	Declaration(Type type);

	std::wstring name;

private:
	Type type;
};

class Declaration::Root : public Expression::Root
{
public:
	Root() : Expression::Root(Type::DeclarationRoot)
	{
	}
};

}

#endif
