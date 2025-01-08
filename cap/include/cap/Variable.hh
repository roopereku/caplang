#ifndef CAP_VARIABLE_HH
#define CAP_VARIABLE_HH

#include <cap/Declaration.hh>

namespace cap
{

class Value;

class Variable : public Declaration
{
public:
	Variable(std::weak_ptr <Value> at);

private:
	std::weak_ptr <Value> at;
};

}

#endif
