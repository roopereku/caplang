#ifndef CAP_BUILTIN_HH
#define CAP_BUILTIN_HH

#include <cap/Source.hh>
#include <cap/TypeContext.hh>

#include <array>
#include <optional>

namespace cap
{

class ClassType;
class Declaration;
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
	/// \param value The string value to match a builtin attribute type to.
	/// \return The corresponding attribute type if any.
	static std::optional<AttributeType> getAttributeType(std::wstring_view value);

	/// Gets the declaration of the given builtin attribute type.
	/// \param The builtin attribute type to retrieve.
	/// \return The declaration of the given builtin attribute type.
	std::shared_ptr<Declaration> getAttributeTypeDeclaration(AttributeType type) const;

	/// Gets a datatype that can be associated with attribute definitions.
	///
	/// \return Datatype that can be associated with attribute definitions.
	TypeContext getTypeForAttributeDefinition() const;

private:
	std::array <std::weak_ptr <ClassType>, 10> cachedTypes;
	std::array <std::weak_ptr <Declaration>, 1> cachedAttributes;
};

}

#endif
