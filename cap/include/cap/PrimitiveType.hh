#ifndef CAP_PRIMITIVE_TYPE
#define CAP_PRIMITIVE_TYPE

#include <cap/TypeDefinition.hh>
#include <cap/Source.hh>

namespace cap
{

class PrimitiveType : public TypeDefinition
{
public:
	PrimitiveType(std::wstring_view name, size_t bytes);

	static PrimitiveType getByName(Source& source, Token token);

private:
	std::wstring_view name;
};

}

#endif
