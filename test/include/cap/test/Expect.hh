#ifndef CAP_TEST_EXPECT_HH
#define CAP_TEST_EXPECT_HH

#include <cap/node/FunctionDefinition.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/ParameterDefinition.hh>
#include <cap/node/VariableDefinition.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/CallOperator.hh>
#include <cap/node/Value.hh>

namespace cap::test
{

std::shared_ptr <ScopeDefinition> expectScope(std::shared_ptr <Node> node);
std::shared_ptr <FunctionDefinition> expectFunction(std::shared_ptr <Node> node);
std::shared_ptr <Expression> expectExpression(std::shared_ptr <Node> node);
std::shared_ptr <Operator> expectOperator(std::shared_ptr <Node> node);
std::shared_ptr <OneSidedOperator> expectOneSidedOperator(std::shared_ptr <Node> node);
std::shared_ptr <TwoSidedOperator> expectTwoSidedOperator(std::shared_ptr <Node> node);
std::shared_ptr <ExpressionRoot> expectExpressionRoot(std::shared_ptr <Node> node);
std::shared_ptr <VariableDefinition> expectVariable(std::shared_ptr <Node> node);
std::shared_ptr <ParameterDefinition> expectParameter(std::shared_ptr <Node> node);
std::shared_ptr <Value> expectValue(std::shared_ptr <Node> node);
std::shared_ptr <CallOperator> expectCall(std::shared_ptr <Node> node);

}

#endif
