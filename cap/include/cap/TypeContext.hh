#ifndef CAP_TYPE_CONTEXT_HH
#define CAP_TYPE_CONTEXT_HH

#include <memory>

namespace cap
{

class TypeDefinition;

/// Type context defines how any given node is referring to
/// a type and which type is it referring to.
class TypeContext
{
public:
	TypeContext(std::shared_ptr <TypeDefinition> referenced)
		: isImmutable(false), isParseTime(false), isTypeName(false), referenced(referenced)
	{
	}

	TypeContext() : TypeContext(nullptr)
	{
	}

	std::shared_ptr <TypeDefinition> getReferenced() const
	{
		return referenced.expired() ? nullptr : referenced.lock();
	}

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
