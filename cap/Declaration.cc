#include <cap/Declaration.hh>

namespace cap
{

Declaration::Declaration(Type type)
	: Node(Node::Type::Declaration), type(type)
{
}

const std::wstring& Declaration::getName()
{
	return name;
}

Declaration::Type Declaration::getType()
{
	return type;
}

const char* Declaration::getTypeString()
{
	switch(type)
	{
		case Type::ClassType: return "Class Type";
		case Type::Function: return "Function";
		case Type::Variable: return "Variable";
	}

	return "(decl) ???";
}

}
