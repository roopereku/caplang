#ifndef CAP_ARGUMENT_ACCESSOR_HH
#define CAP_ARGUMENT_ACCESSOR_HH

#include <cap/Expression.hh>
#include <cap/Variable.hh>

namespace cap
{

/// ArgumentAccessor is an interface used to retrieve argument expressions
/// within some implementation defined context.
class ArgumentAccessor
{
public:
	/// Gets the next argument.
	///
	/// \return The next argument expression or null.
	virtual std::shared_ptr <Expression> getNext() = 0;
};

/// CommaAccessor is used to retrieve expressions delimited by commas.
class CommaAccessor : public ArgumentAccessor
{
public:
	CommaAccessor(std::shared_ptr <Expression::Root> root);
	CommaAccessor(std::shared_ptr <Variable::Root> root);

	/// Gets the next argument delimited by a comma.
	///
	/// \return The next argument expression or null.
	std::shared_ptr <Expression> getNext() override;

private:
	void locateFirst();

	std::shared_ptr <Expression> origin;
	std::shared_ptr <Expression> current;
};

}

#endif
