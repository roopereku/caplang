#ifndef CAP_BUILTIN_HH
#define CAP_BUILTIN_HH

#include <cap/Source.hh>

#include <array>

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

	Builtin();

	void doCaching();
	TypeDefinition& get(DataType type) const;

private:
	std::array <std::weak_ptr <ClassType>, 10> cachedDeclarations;
};

}

#endif
