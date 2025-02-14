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

private:
	std::weak_ptr <TypeDefinition> referenced;

	/// Is a type instance referred whose values are immutable?
	bool immutable;

	/// Is a type instance referred to in a parse time context?
	bool parseTime;

	/// Is a type being referred to through a literal value?
	bool literal;
};

}

#endif
