#include "SyntaxTreeNode.hh"

const char* Cap::SyntaxTreeNode::getTypeString(Type t)
{
	switch(t)
	{
		case Type::Assign: return "Assign";

		case Type::Or: return "Or";
		case Type::And: return "And";

		case Type::BitwiseOR: return "BitwiseOR";
		case Type::BitwiseAND: return "BitwiseAND";
		case Type::BitwiseNOT: return "BitwiseNOT";
		case Type::BitwiseXOR: return "BitwiseXOR";
		case Type::BitwiseShiftLeft: return "BitwiseShiftLeft";
		case Type::BitwiseShiftRight: return "BitwiseShiftRight";

		case Type::Equal: return "Equal";
		case Type::Inequal: return "Inequal";

		case Type::Not: return "Not";
		case Type::Less: return "Less";
		case Type::Greater: return "Greater";
		case Type::LessEqual: return "LessEqual";
		case Type::GreaterEqual: return "GreaterEqual";

		case Type::Addition: return "Addition";
		case Type::Subtraction: return "Subtraction";
		case Type::Multiplication: return "Multiplication";
		case Type::Division: return "Division";
		case Type::Modulus: return "Modulus";
		case Type::Power: return "Power";

		case Type::Access: return "Access";
		case Type::Reference: return "Reference";
		case Type::UnaryPositive: return "UnaryPositive";
		case Type::UnaryNegative: return "UnaryNegative";
		case Type::Ternary: return "Ternary";
		case Type::Condition: return "Condition";
		case Type::Value: return "Value";
	}

	return "";
}
