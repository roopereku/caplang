#ifndef CAP_PARAMETERS_HH
#define CAP_PARAMETERS_HH

#include <cap/Scope.hh>

namespace cap
{

class Parameters : public Scope
{
public:
	Parameters(Scope& parent) : Scope(parent)
	{
	}
};

}

#endif
