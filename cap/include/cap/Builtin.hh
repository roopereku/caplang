#ifndef CAP_BUILTIN_HH
#define CAP_BUILTIN_HH

#include <cap/Source.hh>
#include <cap/TypeDefinition.hh>

namespace cap
{

class Builtin : public Source
{
public:
	wchar_t operator[](size_t index) const override;
	std::wstring getString(cap::Token token) const override;
	bool match(cap::Token token, std::wstring_view value) const override;

	void doCaching();

	std::shared_ptr <TypeDefinition> getDefaultIntegerType() const;
	std::shared_ptr <TypeDefinition> getStringType() const;

private:
	std::weak_ptr <TypeDefinition> defaultIntegerType;
	std::weak_ptr <TypeDefinition> stringType;
};

}

#endif
