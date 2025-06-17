#ifndef CAP_ARGUMENT_ACCESSOR_HH
#define CAP_ARGUMENT_ACCESSOR_HH

#include <cap/Expression.hh>
#include <cap/Variable.hh>

namespace cap
{

/// ArgumentAccessor can be used to access arguments within an expression root.
class ArgumentAccessor
{
public:
	ArgumentAccessor(std::shared_ptr <Expression::Root> root);
	ArgumentAccessor(std::shared_ptr <Variable::Root> root);

	std::shared_ptr <Expression> getNext();

private:
	void locateFirst();

	std::shared_ptr <Expression> origin;
	std::shared_ptr <Expression> current;
};

}

#endif
