#ifndef CAP_PARAMETER_ACCESSOR_HH
#define CAP_PARAMETER_ACCESSOR_HH

#include <cap/Expression.hh>

namespace cap
{

class ParameterAccessor
{
public:
	ParameterAccessor(std::shared_ptr <Expression::Root> root);

	std::shared_ptr <Expression> getNext();

private:
	std::shared_ptr <Expression> current;
	bool hasDeclarations;
};

}

#endif
