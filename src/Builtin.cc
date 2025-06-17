#include <cap/Builtin.hh>
#include <cap/Source.hh>
#include <cap/ClassType.hh>

#include <cassert>

namespace cap
{

// TODO: Indicate type size somehow. Maybe attributes?
Builtin::Builtin() : Source(LR"SRC(
	type uint8
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

	type int8
	{
	}

	type int16
	{
	}

	type int32
	{
	}

	type int64
	{
	}

	type string
	{
	}

	type void
	{
	}

)SRC") {}

void Builtin::doCaching()
{
	assert(getGlobal());
	size_t index = 0;
	for(auto decl : getGlobal()->declarations)
	{
		assert(decl->getType() == Declaration::Type::Class);
		cachedDeclarations[index] = std::static_pointer_cast <ClassType> (decl);

		index++;
	}
}

TypeDefinition& Builtin::get(DataType type) const
{
	const size_t index = static_cast <size_t> (type);
	assert(index < cachedDeclarations.size());
	assert(!cachedDeclarations[index].expired());
	return *cachedDeclarations[index].lock();
}

}
