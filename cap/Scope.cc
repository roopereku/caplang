#include "Logger.hh"
#include "Scope.hh"
#include "Debug.hh"

Cap::Scope::Scope(Scope* parent, ScopeContext ctx)
		:	parent(parent), ctx(ctx),
			root(nullptr), node(&root)
{
	//	If this scope belongs to a function, the first node always contains parameters
	if(ctx == ScopeContext::Function)
	{
		root.type = SyntaxTreeNode::Type::Parameters;

		//	Create a Variable node so that the expression parser can add variables without "var"
		root.left = std::make_shared <SyntaxTreeNode> (&root, nullptr, SyntaxTreeNode::Type::Variable);

		//	This node is used for the contents of the function
		root.right = std::make_shared <SyntaxTreeNode> (&root, nullptr, SyntaxTreeNode::Type::None);

		//	Start with the parameters
		node = root.left.get();
	}

	else root.type = SyntaxTreeNode::Type::Expression;
}

Cap::Function& Cap::Scope::addFunction(Token* name)
{
	functions.emplace_back(name);
	functions.back().scope = std::make_shared <Scope> (this, ScopeContext::Function);
	DBG_LOG("Added function '%s'", name ? name->getString().c_str() : "anonymous");

	return functions.back();
}

Cap::Type& Cap::Scope::addType(Token* name)
{
	types.emplace_back(name);
	types.back().scope = std::make_shared <Scope> (this, ScopeContext::Type);
	DBG_LOG("Added type '%s'", name->getString().c_str());

	return types.back();
}

Cap::Variable& Cap::Scope::addVariable(Token* name)
{
	variables.emplace_back(name);
	DBG_LOG("Added variable '%s'", name->getString().c_str());
	return variables.back();
}

Cap::Scope& Cap::Scope::addBlock(ScopeContext ctx)
{
	blocks.emplace_back(this, ctx);
	return blocks.back();
}

Cap::Function* Cap::Scope::findFunction(size_t index)
{
	return &functions[index];
}

Cap::Function* Cap::Scope::findFunction(Token* name)
{
	for(auto& it : functions)
	{
		if(it.name && it.name->tokenEquals(name))
			return &it;
	}

	return parent == nullptr ? nullptr : parent->findFunction(name);
}

Cap::Variable* Cap::Scope::findVariable(Token* name)
{
	for(auto& it : variables)
	{
		//DBG_LOG("var '%s'", it.name->getString().c_str());
		if(it.name->tokenEquals(name))
			return &it;
	}

	return parent == nullptr ? nullptr : parent->findVariable(name);
}

Cap::Type* Cap::Scope::findType(Token* name)
{
	for(auto& it : types)
	{
		if(it.name->tokenEquals(name))
			return &it;
	}

	return parent == nullptr ? Type::findPrimitiveType(name) : parent->findType(name);
}

bool Cap::Scope::validate()
{
	/*	TODO
	 *	Once there is a way to look into other global scopes,
	 *	look for duplicate declarations */
	if(!validateNode(&root))
		return false;

	for(auto& t : types)
	{
		DBG_LOG("Validating type %s", t.name->getString().c_str());

		if(!t.scope->validate())
			return false;

	}

	for(auto& f : functions)
	{
		DBG_LOG("Validating function %s", f.name->getString().c_str());

		if(!f.scope->validate())
			return false;
	}

	return true;
}

bool Cap::Scope::validateNode(SyntaxTreeNode* n)
{
	//	Does the current node contains an operator
	if(n->type <= SyntaxTreeNode::Type::UnaryNegative)
	{
		//	Does the expression do anything
		if(	n->parent->type == SyntaxTreeNode::Type::Expression &&
			n->type != SyntaxTreeNode::Type::Assign)
	{
			printf("UNUSED EXPRESSION on line at %u:%u\n", n->value->line, n->value->column);
		}

		/*	If the current node contains an access operator, we only need to
		 *	get info starting from said node because of
		 *	the way getNodeInfoRecursive() works */
		SyntaxTreeNode* leftOrigin = n->type == SyntaxTreeNode::Type::Access ?
			n : n->left.get();

		//	Get information about the left node
		NodeInfo left = getNodeInfoRecursive(leftOrigin);

		if(!left.at)
			return false;

		//	The left node can only be a plain typename if an access is being made
		if(isNodeTypeName(left) && left.at->parent->type != SyntaxTreeNode::Type::Access)
		{
			Logger::error(*left.at->value, "Expected '.' after typename '%s'", getFullAccessName(left.at).c_str());
			return false;
		}

		//DBG_LOG("Left of '%s' is '%s'", n->value->getString().c_str(), left.at->value->getString().c_str());

		//	Does said operator have 2 operands?
		if(	n->type <= SyntaxTreeNode::Type::Power &&
			n->type != SyntaxTreeNode::Type::Access)
		{
			NodeInfo right = getNodeInfoRecursive(n->right.get());

			if(!right.at)
				return false;

			bool isRightType = isNodeTypeName(right);

			//	Does the right node have a conversion to the type of the left node
			if(!isRightType && left.t && right.t && !right.t->hasConversion(*left.t))
			{
				Logger::error(*right.at->value, "Type '%s' doesn't have a conversion to '%s'", right.t->name->getString().c_str(), left.t->name->getString().c_str());
				return false;
			}

			//DBG_LOG("Right of '%s' is '%s'", n->value->getString().c_str(), right.at->value->getString().c_str());

			if(n->type == SyntaxTreeNode::Type::Assign)
			{
				//	If the node on the right doesn't contain a plain type, it's a value
				if(!isRightType)
				{
					if(left.v)
					{
						//	Forbid assigning a variable it's own value before it's initialized
						if(!left.v->initialized && left.at->value->tokenEquals(right.at->value))
						{
							Logger::error("Can't use '%s' before it's initialized", getFullAccessName(right.at).c_str());
							return false;
						}

						left.v->type = right.t;
						left.v->initialized = true;

						DBG_LOG("Variable '%s' initialized with type '%s'", left.v->name->getString().c_str(), left.v->type->name->getString().c_str());
					}
				}

				//	The right node contains a plain type name
				else
				{
					//	Is the left node a variable that's initialized
					if(left.v)
					{
						//	Types cannot be assigned to variables if they already have types
						if(left.v->type)
						{
							Logger::error(*right.at->value, "Can't assign a type to '%s' because it already has one", getFullAccessName(left.at).c_str());
							return false;
						}

						//	If the variable doesn't have a type, give it one
						else if(!left.v->type)
						{
							DBG_LOG("Assigning type '%s' to '%s'", right.t->name->getString().c_str(), left.v->name->getString().c_str());
							left.v->type = right.t;
						}
					}
				}
			}

			else
			{
				//	The right node can't be a plain type if no assignment was done
				if(isRightType)
				{
					Logger::error(*right.at->value, "Type '%s' can only be used after '='", getFullAccessName(right.at).c_str());
					return false;
				}

				if(left.v && !left.v->initialized)
				{
					Logger::error(*left.at->value, "Can't use variable '%s' before it's initialized", getFullAccessName(left.at).c_str());
					return false;
				}

				if(right.v && !right.v->initialized)
				{
					Logger::error(*right.at->value, "Can't use variable '%s' before it's initialized", getFullAccessName(right.at).c_str());
					return false;
				}
			}
		}

		else
		{
		}
	}

	SyntaxTreeNode* resultNode;

	//	Validate the left node
	if(n->left && !validateNode(n->left.get()))
		return false;

	//	Validate the right node
	if(n->right && !validateNode(n->right.get()))
		return false;

	return true;
}

Cap::SyntaxTreeNode* Cap::Scope::findAppropriateNode(SyntaxTreeNode* n)
{
	if(	n->type == SyntaxTreeNode::Type::Value ||
		n->type == SyntaxTreeNode::Type::Call ||
		n->type == SyntaxTreeNode::Type::Subscript)
	{
		return n;
	}

	SyntaxTreeNode* current = n;

	while(current->left)
	{
		if(current->type == SyntaxTreeNode::Type::Access)
			return current;

		current = current->left.get();
	}

	return current;
}

Cap::Scope::NodeInfo Cap::Scope::getNodeInfoRecursive(SyntaxTreeNode* n)
{
	Scope* scope = this;
	NodeInfo info;
	info.at = findAppropriateNode(n);

	while(info.at->type == SyntaxTreeNode::Type::Access)
	{
		NodeInfo left = scope->getNodeInfo(info.at->left.get());

		if(!left.at)
			return left;

		//	Accessing a function scope is forbidden
		if(left.f)
		{
			Logger::error(*left.at->value, "Can't access the scope of function '%s'", left.f->name->getString().c_str());
			left.at = nullptr;
			return left;
		}

		else if(left.v)
		{
			//	The left operand has to be initialized
			if(!left.v->initialized)
			{
				Logger::error(*left.at->value, "Can't use '%s' before it's initialized", left.v->name->getString().c_str());
				left.at = nullptr;
				return left;
			}

			//	TODO use the scope of the type of the variable
		}

		if(left.t)
		{
			//	Primitive types don't have visible members
			if(left.t->isPrimitive)
			{
				Logger::error(*left.at->value, "Can't use '.' after primitive type '%s'", left.t->name->getString().c_str());
				left.at = nullptr;
				return left;
			}

			//DBG_LOG("UPDATE SCOPE TO '%s'", left.at->value->getString().c_str());
			scope = left.t->scope.get();
		}

		info.at = info.at->right.get();
	}

	return scope->getNodeInfo(info.at);
}

Cap::Scope::NodeInfo Cap::Scope::getNodeInfo(SyntaxTreeNode* n)
{
	NodeInfo info;
	info.at = n;

	info.v = findVariable(info.at->value);
	info.t = findType(info.at->value);
	info.f = findFunction(info.at->value);

	//	Is the given node a known identifier?
	if(!info.v && !info.t && !info.f)
	{
		Logger::error(*info.at->value, "Unknown identifier '%s'", getFullAccessName(info.at).c_str());
		info.at = nullptr;
		return info;
	}

	if(info.v)
		info.t = info.v->type;

	return info;
}

std::string Cap::Scope::getFullAccessName(SyntaxTreeNode* last)
{
	//	First we need to know where the first access node is
	SyntaxTreeNode* current = last;
	while(current->parent->type == SyntaxTreeNode::Type::Access)
		current = current->parent;

	//	If there's no accesses, return a single identifier
	if(current->type != SyntaxTreeNode::Type::Access)
		return current->value->getString();

	std::string name = current->left->value->getString();

	//	Add the rest of the access nodes to the string
	while(current->left)
	{
		name += '.' + current->right->value->getString();
		current = current->right.get();
	}

	return name;
}

bool Cap::Scope::isNodeTypeName(NodeInfo& info)
{
	return	info.at->value->type == TokenType::Identifier &&
			info.at->type == SyntaxTreeNode::Type::Value &&
			!info.v && info.t;
}
