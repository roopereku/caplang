#ifndef CAP_BUILTIN_HH
#define CAP_BUILTIN_HH

#include <cap/Source.hh>

#include <array>
#include <optional>

namespace cap
{

class ClassType;
class TypeDefinition;

class Builtin : public Source
{
public:
	enum class DataType
	{
		Uint8,
		Uint16,
		Uint32,
		Uint64,
		Int8,
		Int16,
		Int32,
		Int64,
		String,
		Void
	};

	enum class AttributeType
	{
		Definition
	};

	Builtin();

	void doCaching();
	TypeDefinition& get(DataType type) const;

	/// Gets the attribute type that corresponds to the given string if any.
	static std::optional<AttributeType> getAttributeType(std::wstring_view value);

private:
	std::array <std::weak_ptr <ClassType>, 10> cachedDeclarations;
};

}

#endif
