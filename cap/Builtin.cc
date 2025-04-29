#include <cap/Builtin.hh>
#include <cap/Source.hh>

#include <cassert>

namespace cap
{

// TODO: Indicate type size somehow. Maybe attributes?
static std::wstring_view src = LR"SRC(
	type int8
	{
	}

	type int32
	{
	}

	type int64
	{
	}

	type uint16
	{
	}

	type uint32
	{
	}

	type uint64
	{
	}

	type string
	{
	}
)SRC";

wchar_t Builtin::operator[](size_t index) const
{
	return src[index];
}

std::wstring Builtin::getString(cap::Token token) const
{
	return std::wstring(src.substr(token.getIndex(), token.getLength()));
}

bool Builtin::match(cap::Token token, std::wstring_view value) const
{
	return src.substr(token.getIndex(), token.getLength()) == value;
}

void Builtin::doCaching()
{
	assert(getGlobal());
	for(auto decl : getGlobal()->declarations)
	{
		if(decl->getName() == L"int64")
		{
			defaultIntegerType = std::static_pointer_cast <TypeDefinition> (decl);
		}

		else if(decl->getName() == L"string")
		{
			stringType = std::static_pointer_cast <TypeDefinition> (decl);
		}
	}
}

std::shared_ptr <TypeDefinition> Builtin::getDefaultIntegerType() const
{
	assert(!defaultIntegerType.expired());
	return defaultIntegerType.lock();
}

std::shared_ptr <TypeDefinition> Builtin::getStringType() const
{
	assert(!stringType.expired());
	return stringType.lock();
}

}
