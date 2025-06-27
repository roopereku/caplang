#ifndef CAP_EXECUTION_SEQUENCE_HH
#define CAP_EXECUTION_SEQUENCE_HH

#include <cap/execution/ExecutionStep.hh>
#include <cap/Function.hh>
#include <cap/Expression.hh>

#include <vector>

namespace cap
{

class BinaryOperator;
class UnaryOperator;
class BracketOperator;
class Statement;

class ExecutionSequence
{
private:
	std::shared_ptr <Function> representedFunction;
	std::vector <ExecutionStep> steps;

public:
	ExecutionSequence(std::shared_ptr <Function> root);
	ExecutionSequence(std::shared_ptr <Expression> root);

	/// Gets the function that this sequence represents.
	///
	/// \return The represented function if any.
	std::shared_ptr <Function> getRepresentedFunction() const;

	/// Adds an execution step.
	///
	/// \param step The execution step to add.
	/// \return The added execution step.
	ExecutionStep& addStep(ExecutionStep&& step);

	using iterator = decltype(steps)::iterator;
	using const_iterator = decltype(steps)::const_iterator;

	iterator begin() { return steps.begin(); }
	iterator end() { return steps.end(); }
	const_iterator begin() const { return steps.begin(); }
	const_iterator end() const { return steps.end(); }
};

}

#endif
