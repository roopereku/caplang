#include <cap/Builtin.hh>
#include <cap/Source.hh>
#include <cap/ClassType.hh>

#include <cassert>
#include <unordered_map>

namespace cap
{

// TODO: Indicate type size somehow. Maybe attributes?
Builtin::Builtin() : Source(LR"SRC(
	type attribute
	{
	}

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
	size_t builtinTypeIndex = 0;

	for(auto decl : getGlobal()->declarations)
	{
		if(auto attributeType = getAttributeType(decl->getName()))
		{
			cachedAttributes[static_cast<size_t>(*attributeType)] = decl;
		}

		// TODO: Would name lookup be okay for builtin types in terms of performance?
		else
		{
			assert(decl->getType() == Declaration::Type::Class);
			cachedTypes[builtinTypeIndex] = std::static_pointer_cast <ClassType> (decl);

			builtinTypeIndex++;
		}
	}
}

TypeDefinition& Builtin::get(DataType type) const
{
	const size_t index = static_cast <size_t> (type);
	assert(index < cachedTypes.size());
	assert(!cachedTypes[index].expired());
	return *cachedTypes[index].lock();
}

std::optional<Builtin::AttributeType> Builtin::getAttributeType(std::wstring_view value)
{
	static std::unordered_map <std::wstring_view, AttributeType> lookup
	{
		{ L"attribute", AttributeType::Definition }
	};

	auto it = lookup.find(value);
	if (it != lookup.end())
	{
		return it->second;
	}

	return std::nullopt;
}

std::shared_ptr<Declaration> Builtin::getAttributeTypeDeclaration(AttributeType type) const
{
	const size_t index = static_cast <size_t> (type);
	assert(index < cachedAttributes.size());
	assert(!cachedAttributes[index].expired());
	return cachedAttributes[index].lock();
}

TypeContext Builtin::getTypeForAttributeDefinition() const
{
	assert(std::dynamic_pointer_cast <ClassType> (getAttributeTypeDeclaration(AttributeType::Definition)));
	TypeContext ret(*std::static_pointer_cast <ClassType> (getAttributeTypeDeclaration(AttributeType::Definition)));

	ret.isParseTime = true;
	// TODO: Should there be some flag that can be used to avoid the usage of an attribute type outside
	// of an attribute context?

	return ret;
}

}
