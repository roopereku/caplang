#include <cap/PrimitiveType.hh>

#include <array>

namespace cap
{

static std::array <PrimitiveType, 9> primitives
{
	PrimitiveType(L"uint8", 1),
	PrimitiveType(L"uint16", 2),
	PrimitiveType(L"uint32", 4),
	PrimitiveType(L"uint64", 8),

	PrimitiveType(L"int8", 1),
	PrimitiveType(L"int16", 2),
	PrimitiveType(L"int32", 4),
	PrimitiveType(L"int64", 8),

	PrimitiveType(L"<INVALID>", 0),
};

PrimitiveType::PrimitiveType(std::wstring_view name, size_t bytes)
	: name(name)
{
}

PrimitiveType PrimitiveType::getByName(Source& source, Token token)
{
	for(auto& t : primitives)
	{
		if(source.match(token, t.name))
		{
			return t;
		}
	}

	return primitives.back();	
}

}
