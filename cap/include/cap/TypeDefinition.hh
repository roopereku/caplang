#ifndef CAP_TYPE_DEFINITION_HH
#define CAP_TYPE_DEFINITION_HH

#include <cap/Declaration.hh>

namespace cap
{

class TypeDefinition : public Declaration
{
public:
	enum class Type
	{
		Class,
		Callable
	};

	/// Gets the type of this type definition.
	///
	/// \return The type of this type definition.
	Type getType();

protected:
	TypeDefinition(Type type);

private:
	Type type;
};

}

#endif
