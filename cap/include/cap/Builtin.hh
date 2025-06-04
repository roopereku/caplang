#ifndef CAP_BUILTIN_HH
#define CAP_BUILTIN_HH

#include <cap/Source.hh>

namespace cap
{

class ClassType;
class TypeDefinition;

class Builtin : public Source
{
public:
	Builtin();

	void doCaching();

	TypeDefinition& getVoidType() const;
	TypeDefinition& getDefaultIntegerType() const;
	TypeDefinition& getStringType() const;

private:
	std::weak_ptr <ClassType> voidDecl;
	std::weak_ptr <ClassType> defaultIntegerDecl;
	std::weak_ptr <ClassType> stringDecl;
};

}

#endif
