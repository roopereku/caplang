#ifndef CAP_PRIMITIVE_TYPE_HH
#define CAP_PRIMITIVE_TYPE_HH

#include <cap/Type.hh>

namespace cap
{

class PrimitiveType : public Type
{
public:
	PrimitiveType(Scope& parent, std::string_view name, size_t size);

	bool isValid();

	bool isPrimitive() final override
	{
		return true;
	}

	bool hasOperator(TwoSidedOperator::Type type) override;
	bool hasOperator(OneSidedOperator::Type type) override;

	/// Registers builtin primitive types to the shared scope.
	/// \param to Where to register builtin types.
	static void registerBuiltins(Scope& to);

private:
	size_t size;
};

}

#endif
