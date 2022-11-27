#include "CodeGenerator.hh"
#include "Logger.hh"
#include "Scope.hh"
#include "Debug.hh"

Cap::CodeGenerator::Output Cap::CodeGenerator::outputType;

Cap::CodeGenerator::CodeGenerator(Scope& scope) : scope(scope)
{
	//	Only functions get a name in the code :-)
	if(scope.ctx == ScopeContext::Function)
	{
	}
}

void Cap::CodeGenerator::setOutput(Output type)
{
	outputType = type;
}

bool Cap::CodeGenerator::generateLine(SyntaxTreeNode& start)
{
	Logger::warning("generate line");
	accumulator = 0;
	SyntaxTreeNode* expr = &start;

	//	Skip nodes until we find an expression
	while(expr->left && expr->type != SyntaxTreeNode::Type::Expression)
		expr = expr->left.get();

	//	No expression here so do nothing
	if(!expr->left)
		return true;

	//	Generate instructions for the expression
	bool result = generateFromNode(*expr->left);

	DBG_LOG("CODE IS NOW\n%s", code.c_str());
	return result;
}

bool Cap::CodeGenerator::generateFromNode(SyntaxTreeNode& node)
{
	//	Ignore values
	if(node.type == SyntaxTreeNode::Type::Value)
		return true;

	DBG_LOG("Process '%s'", node.getTypeString());

	//	Generate instructions on the left side
	if(node.left && !generateFromNode(*node.left))
		return false;

	//	Generate instructions on the right side
	if(node.right && !generateFromNode(*node.right))
		return false;

	//	Which architecture are we generating for
	switch(outputType)
	{
		case Output::Test: return generateInstructionTest(node);
	}

	//	Should never get here
	return false;
}

bool Cap::CodeGenerator::generateInstructionTest(SyntaxTreeNode& node)
{
	using T = SyntaxTreeNode::Type;

	InstructionType t = getType(node.type);
	std::string op = "nop";

	//	On assignment, save to a slow corresponding to the variable's depth
	if(t == InstructionType::Assignment)
	{
		Variable* v = scope.findVariable(node.left->value);
		code += "save " + std::to_string(v->depth) + "\n";

		return true;
	}

	//	What's the name of the operation
	switch(node.type)
	{
		case T::Addition: op = "add"; break;
		case T::Division: op = "div"; break;
		case T::Subtraction: op = "sub"; break;
		case T::Multiplication: op = "mul"; break;
		case T::Power: op = "pow"; break;

		default:
			Logger::error("???: Passed '%s' to generateInstructionTest", node.getTypeString());
			return false;
	}

	int oldAccumulator = accumulator;

	if(t == InstructionType::Arithmetic)
	{
		//	Which side of the node has a literal or an identifier
		bool leftValue = node.left->type == T::Value;
		bool rightValue = node.right->type == T::Value;

		if(leftValue)
		{
			//	Increment and limit the accumulator index to 2
			if(++accumulator > 2)
				accumulator = 2;

			/*	Example: 48 - 12
			 *	Because the left side of the node has a value and the operation
			 *	could be subtraction, we need to store the value to some other
			 *	accumulator so that we can later load 12 to the first accumulator
			 *	and subtract with that accumulator as the value */
			code += "select " + std::to_string(accumulator) + "\n";

			//	If the value is an identifier, it should be a variable
			if(node.left->value->type == TokenType::Identifier)
			{
				//	Tell the interpreter to load a value from the storage
				Variable* v = scope.findVariable(node.left->value);
				code += "load " + std::to_string(v->depth) + "\n";
			}

			//	Store a literal value to the accumulator
			else code += "store " + node.left->value->getString() + "\n";
		}

		//	If neither side has a value, they contain operators
		if(!rightValue && !leftValue)
		{
			/*	Select the first accumulator so that asub can be used with
			 *	the second accumulator as the value */
			accumulator--;
			code += "select " + std::to_string(accumulator) + "\n";
		}

		//	If there's a value on the right side, call some operator with a literal
		if(rightValue)
			code += op + " " + node.right->value->getString() + "\n";

		//	If there's no value on the right side, call some operator with an accumulator
		else
			code += "a" + op + " " + std::to_string(oldAccumulator) + "\n";
	}

	return true;
}

const char* Cap::CodeGenerator::instructionTypeString(InstructionType t)
{
	switch(t)
	{
		case InstructionType::Arithmetic: return "Arithmetic";
		case InstructionType::Assignment: return "Assignment";
		case InstructionType::Comparison: return "Comparison";
		case InstructionType::Unary: return "Unary";
	}

	return "";
}

Cap::CodeGenerator::InstructionType Cap::CodeGenerator::getType(SyntaxTreeNode::Type t)
{
	switch(t)
	{
		case SyntaxTreeNode::Type::Assign:
			return InstructionType::Assignment;

		case SyntaxTreeNode::Type::Or:
		case SyntaxTreeNode::Type::And:
		case SyntaxTreeNode::Type::BitwiseOR:
		case SyntaxTreeNode::Type::BitwiseAND:
		case SyntaxTreeNode::Type::BitwiseXOR:
		case SyntaxTreeNode::Type::BitwiseShiftLeft:
		case SyntaxTreeNode::Type::BitwiseShiftRight:
		case SyntaxTreeNode::Type::Addition:
		case SyntaxTreeNode::Type::Subtraction:
		case SyntaxTreeNode::Type::Multiplication:
		case SyntaxTreeNode::Type::Division:
		case SyntaxTreeNode::Type::Modulus:
		case SyntaxTreeNode::Type::Power:
			return InstructionType::Arithmetic;

		case SyntaxTreeNode::Type::Equal:
		case SyntaxTreeNode::Type::Inequal:
		case SyntaxTreeNode::Type::Less:
		case SyntaxTreeNode::Type::Greater:
		case SyntaxTreeNode::Type::LessEqual:
		case SyntaxTreeNode::Type::GreaterEqual:
			return InstructionType::Comparison;

		case SyntaxTreeNode::Type::Not:
		case SyntaxTreeNode::Type::Reference:
		case SyntaxTreeNode::Type::BitwiseNOT:
		case SyntaxTreeNode::Type::UnaryPositive:
		case SyntaxTreeNode::Type::UnaryNegative:
			return InstructionType::Unary;

		default:
			Logger::error("???: Requested instruction type for '%s'", SyntaxTreeNode::getTypeString(t));
			return InstructionType::Arithmetic;
	}
}
