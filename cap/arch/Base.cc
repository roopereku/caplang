#include "Base.hh"
#include "../Logger.hh"

const char* Cap::Arch::Base::instructionTypeString(InstructionType t)
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

Cap::Arch::Base::InstructionType Cap::Arch::Base::getType(SyntaxTreeNode::Type t)
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
