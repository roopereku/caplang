#include <gtest/gtest.h>

#include <cap/execution/ExecutionSequence.hh>
#include <cap/test/IsolatedTester.hh>
#include <cap/Client.hh>

using namespace cap::test;

class StructureTester : cap::test::IsolatedTester
{
public:
	void test(std::wstring&& src, std::vector <ExpectedNode>&& expected)
	{
		auto isolated = getIsolatedFunction(std::move(src));
		cap::ExecutionSequence sequence(isolated);

		size_t i = 0;
		for(auto& step : sequence)
		{
			ASSERT_LT(i, expected.size());
			auto& currentStep = expected[i];
			ASSERT_STREQ(step.action->getTypeString(), currentStep.nodeType.data());

			size_t currentResultIndex = std::stoull(currentStep.context);
			ASSERT_EQ(step.getResultIndex(), currentResultIndex);

			i++;
			for(auto& op : step.getOperands())
			{
				ASSERT_LT(i, expected.size());
				auto& currentOperand = expected[i];
				ASSERT_STREQ(op.getNode()->getTypeString(), currentOperand.nodeType.data());

				switch(op.getType())
				{
					case cap::ExecutionStep::Operand::Type::Result:
					{
						size_t operandResultIndex = std::stoull(currentOperand.context);
						ASSERT_EQ(op.asResultIndex(), operandResultIndex);
						break;
					}

					case cap::ExecutionStep::Operand::Type::Immediate:
					{
						auto value = op.asImmediate();
						ASSERT_STREQ(value->getString().c_str(), currentOperand.context.c_str());
						break;
					}
				}

				i++;
			}
		}
	}
};

template <typename T>
ExpectedNode Result(T actionType, size_t resultIndex)
{
	ExpectedNode result(actionType);
	result.context = std::to_wstring(resultIndex);

	return result;
}

TEST(ExecutionStructureTests, IndexStaysAtZero)
{
	StructureTester tester;

	tester.test(L"1 + 1",
	{
		Result(cap::BinaryOperator::Type::Add, 0),
			Integer(1),
			Integer(1),
	});

	tester.test(L"1 + 2 * 3 - 4",
	{
		Result(cap::BinaryOperator::Type::Multiply, 0),
			Integer(2),
			Integer(3),

		Result(cap::BinaryOperator::Type::Add, 0),
			Integer(1),
			Result(cap::BinaryOperator::Type::Multiply, 0),

		Result(cap::BinaryOperator::Type::Subtract, 0),
			Result(cap::BinaryOperator::Type::Add, 0),
			Integer(4),
	});
}

TEST(ExecutionStructureTests, IndexGoesAboveZero)
{
	StructureTester tester;

	tester.test(L"1 * 2 + 3 / 4",
	{
		Result(cap::BinaryOperator::Type::Multiply, 0),
			Integer(1),
			Integer(2),

		Result(cap::BinaryOperator::Type::Divide, 1),
			Integer(3),
			Integer(4),

		Result(cap::BinaryOperator::Type::Add, 0),
			Result(cap::BinaryOperator::Type::Multiply, 0),
			Result(cap::BinaryOperator::Type::Divide, 1)
	});

	tester.test(L"(1 * 2 + 3 / 4) - (5 * 6 + 7 / 8)",
	{
		Result(cap::BinaryOperator::Type::Multiply, 0),
			Integer(1),
			Integer(2),

		Result(cap::BinaryOperator::Type::Divide, 1),
			Integer(3),
			Integer(4),

		Result(cap::BinaryOperator::Type::Add, 0),
			Result(cap::BinaryOperator::Type::Multiply, 0),
			Result(cap::BinaryOperator::Type::Divide, 1),

		Result(cap::BinaryOperator::Type::Multiply, 1),
			Integer(5),
			Integer(6),

		Result(cap::BinaryOperator::Type::Divide, 2),
			Integer(7),
			Integer(8),

		Result(cap::BinaryOperator::Type::Add, 1),
			Result(cap::BinaryOperator::Type::Multiply, 1),
			Result(cap::BinaryOperator::Type::Divide, 2),

		Result(cap::BinaryOperator::Type::Subtract, 0),
			Result(cap::BinaryOperator::Type::Add, 0),
			Result(cap::BinaryOperator::Type::Add, 1),
	});
}
