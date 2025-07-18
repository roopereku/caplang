#ifndef CAP_TYPE_DEFINITION_HH
#define CAP_TYPE_DEFINITION_HH

#include <string>

namespace cap
{

class TypeDefinition
{
public:
	enum class Type
	{
		Class,
		Callable,
		TypeReference
	};

	TypeDefinition(const TypeDefinition&) = delete;

	/// Gets the type of this type definition.
	///
	/// \return The type of this type definition.
	Type getType();

	/// Gets an implementation defined string representation of this type.
	///
	/// \param If true, implementation defined details are included.
	/// \return Implementation defined.
	virtual std::wstring toString(bool detailed) const = 0;

protected:
	TypeDefinition(Type type);

private:
	Type type;
};

}

#endif
