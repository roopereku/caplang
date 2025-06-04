#ifndef CAP_TYPE_CONTEXT_HH
#define CAP_TYPE_CONTEXT_HH

#include <string>

namespace cap
{

class TypeDefinition;

/// Type context defines how any given node is referring to
/// a type and which type is it referring to.
class TypeContext
{
public:
	TypeContext(TypeDefinition& referenced);

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

	std::wstring toString() const;

	/// Is a type instance referred whose values are immutable?
	bool isImmutable = false;

	/// Is a type instance referred to in a parse time context?
	bool isParseTime = false;

	/// Is a type being referred to through its name?
	bool isTypeName = false;

	// TODO: Add generic instantiation.

	TypeDefinition& referenced;
};

}

#endif
