#include <gtest/gtest.h>

#include <cap/SourceFile.hh>
#include <cap/test/Expect.hh>
#include <cap/test/ValueCheck.hh>
#include <cap/event/ErrorMessage.hh>

#include <iostream>


class EventValidator : public cap::EventEmitter
{
public:
	void emit(cap::Event&& event) override
	{
		if(event == cap::Event::Type::Message)
		{
			auto msg = event.as <cap::Message> ();

			if(msg.type == cap::Message::Type::Error)
			{
				std::cerr << "Error: " << msg.contents << '\n';
			}
		}
	}
};

class TestProgram
{
public:
	TestProgram(std::string_view code) : file(cap::SourceFile::fromSource(code))
	{
	}

	std::shared_ptr <cap::Node> compile()
	{
		EventValidator events;
		if(!file.prepare(events))
		{
			return nullptr;
		}

		return file.getGlobal();
	}

	cap::SourceFile file;
};

TEST(CapTest, SimpleProgram) {
	TestProgram program(R"(
		func main(argc = int64)
		{
			var foo = 12 * 50
			doSomething(foo, argc)
		}	
	)");

	// Make sure that the program compiles.
	auto globalScope = cap::test::expectScope(program.compile());
	ASSERT_TRUE(globalScope);	

	// Make sure that the main function is the first node.
	auto mainFunction = cap::test::expectFunction(globalScope->getRoot());
	ASSERT_TRUE(mainFunction);	
	ASSERT_TRUE(mainFunction->name == "main");

	// Make sure that the argc parameter is the first node in the function.
	auto parameterArgc = cap::test::expectParameter(mainFunction->getRoot());
	ASSERT_TRUE(parameterArgc);
	ASSERT_TRUE(parameterArgc->name->token == "argc");
	ASSERT_TRUE(parameterArgc->getResultType().lock()->name == "int64");

	// Make sure that the foo variable comes after argc.
	auto variableFoo = cap::test::expectVariable(parameterArgc->getNext());
	ASSERT_TRUE(variableFoo);
	ASSERT_TRUE(variableFoo->name->token == "foo");

	// Make sure that foo is initialized with a multiplication.
	auto multiplication = cap::test::expectTwoSidedOperator(variableFoo->getRoot());
	ASSERT_TRUE(multiplication);
	ASSERT_TRUE(multiplication->type == cap::TwoSidedOperator::Type::Multiplication);

	// Make sure that the operands are correct.
	ASSERT_TRUE(cap::test::isInteger("12", multiplication->getLeft()));
	ASSERT_TRUE(cap::test::isInteger("50", multiplication->getRight()));

	// There should be an expression root after variable foo.
	auto afterFoo = cap::test::expectExpressionRoot(variableFoo->getNext());	
	ASSERT_TRUE(afterFoo);

	// The expression should begin with a call to doSomething.
	auto call = cap::test::expectCall(afterFoo->getRoot());
	ASSERT_TRUE(call);
	ASSERT_TRUE(cap::test::isIdentifier("doSomething", call->getTarget()));

	// There should be multiple call parameters separated by a comma.
	auto paramSeparator = cap::test::expectTwoSidedOperator(call->getExpression());
	ASSERT_TRUE(paramSeparator);
	ASSERT_TRUE(paramSeparator->type == cap::TwoSidedOperator::Type::Comma);

	// Make sure that foo and argc are passed to the function.
	ASSERT_TRUE(cap::test::isIdentifier("foo", paramSeparator->getLeft()));
	ASSERT_TRUE(cap::test::isIdentifier("argc", paramSeparator->getRight()));
}
