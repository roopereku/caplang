#ifndef CAP_BUILTIN_HH
#define CAP_BUILTIN_HH

#include <cap/Source.hh>
#include <cap/TypeDefinition.hh>

namespace cap
{

class Builtin : public Source
{
public:
	Builtin();

	void doCaching();

	std::shared_ptr <TypeDefinition> getDefaultIntegerType() const;
	std::shared_ptr <TypeDefinition> getStringType() const;

private:
	std::weak_ptr <TypeDefinition> defaultIntegerType;
	std::weak_ptr <TypeDefinition> stringType;
};

}

#endif
