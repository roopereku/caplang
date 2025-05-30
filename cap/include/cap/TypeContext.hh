#ifndef CAP_TYPE_CONTEXT_HH
#define CAP_TYPE_CONTEXT_HH

#include <memory>
#include <string>

namespace cap
{

class TypeDefinition;

/// Type context defines how any given node is referring to
/// a type and which type is it referring to.
class TypeContext
{
public:
	TypeContext(std::shared_ptr <TypeDefinition> referenced);
	TypeContext();

	/// Determines whether this type context is compatible with another.
	///
	/// \param other The type context to compare against.
	/// \return True if compatible.
	bool isCompatible(const TypeContext& other) const;

	/// Determines whether this type context is identical with another.
	///
	/// \param other The type context to compare against.
	/// \return True if identical.
	bool isIdentical(const TypeContext& other) const;

	std::shared_ptr <TypeDefinition> getReferenced() const;
	std::wstring toString() const;

	/// Is a type instance referred whose values are immutable?
	bool isImmutable;

	/// Is a type instance referred to in a parse time context?
	bool isParseTime;

	/// Is a type being referred to through its name?
	bool isTypeName;

	// TODO: Add generic instantiation.

private:
	std::weak_ptr <TypeDefinition> referenced;
};

}

#endif
