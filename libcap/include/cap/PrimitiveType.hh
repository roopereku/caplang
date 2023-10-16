#ifndef CAP_PRIMITIVE_TYPE_HH
#define CAP_PRIMITIVE_TYPE_HH

#include <cap/Type.hh>

namespace cap
{

class PrimitiveType : public Type
{
public:
	PrimitiveType(std::string_view name, size_t size);

	bool isValid();

	bool isPrimitive() final override
	{
		return true;
	}

	bool hasOperator(TwoSidedOperator::Type type) override;
	bool hasOperator(OneSidedOperator::Type type) override;

private:
	size_t size;
};

}

#endif
