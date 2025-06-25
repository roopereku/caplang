#ifndef CAP_EXECUTION_CONTEXT_HH
#define CAP_EXECUTION_CONTEXT_HH

#include <cap/execution/ExecutionSequence.hh>
#include <cap/Variable.hh>

#include <stack>
#include <unordered_map>

namespace cap
{

class ExecutionContext
{
public:
	class Frame;

	void tick();

private:
	std::stack <Frame> frames;
};

class ExecutionContext::Frame
{
public:

private:
	std::unordered_map <std::shared_ptr <Variable>, int> variables;
};

}

#endif
