#include <cap/Builtin.hh>
#include <cap/Source.hh>
#include <cap/ClassType.hh>

#include <cassert>

namespace cap
{

// TODO: Indicate type size somehow. Maybe attributes?
Builtin::Builtin() : Source(LR"SRC(
	type void
	{
	}

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

)SRC") {}

void Builtin::doCaching()
{
	assert(getGlobal());
	for(auto decl : getGlobal()->declarations)
	{
		if(decl->getName() == L"void")
		{
			voidDecl = std::static_pointer_cast <ClassType> (decl);
		}

		else if(decl->getName() == L"int64")
		{
			defaultIntegerDecl = std::static_pointer_cast <ClassType> (decl);
		}

		else if(decl->getName() == L"string")
		{
			stringDecl = std::static_pointer_cast <ClassType> (decl);
		}
	}
}

TypeDefinition& Builtin::getVoidType() const
{
	assert(!voidDecl.expired());
	return *voidDecl.lock();
}

TypeDefinition& Builtin::getDefaultIntegerType() const
{
	assert(!defaultIntegerDecl.expired());
	return *defaultIntegerDecl.lock();
}

TypeDefinition& Builtin::getStringType() const
{
	assert(!stringDecl.expired());
	return *stringDecl.lock();
}

}
