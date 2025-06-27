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

	bool isFinished() const;

	void start(std::shared_ptr <Function> node);
	void step();

private:
	std::unordered_map <std::shared_ptr <Node>, ExecutionSequence> sequenceLookup;
	std::stack <Frame> frames;
};

class ExecutionContext::Frame
{
public:
	Frame(ExecutionSequence& sequence);

	bool step();

private:
	void executeBinaryOperator();
	void executeUnaryOperator();
	void executeBracketOperator();
	void executeStatement();

	ExecutionSequence& sequence;
	ExecutionSequence::iterator pointer;

	// TODO: How about member declarations in types?
	// TODO: How about global declarations?
	std::unordered_map <std::shared_ptr <Variable>, int> variables;
};

}

#endif
