#include "Scope.hh"
#include "Debug.hh"

Cap::Function& Cap::Scope::addFunction(Token* name, size_t begin, size_t end)
{
	functions.emplace_back(name);
	functions.back().scope = std::make_shared <Scope> (this, ScopeContext::Function, begin, end);
	DBG_LOG("Added function '%s'", name->getString().c_str());

	return functions.back();
}

Cap::Type& Cap::Scope::addType(Token* name, size_t begin, size_t end)
{
	types.emplace_back(name);
	types.back().scope = std::make_shared <Scope> (this, ScopeContext::Type, begin, end);
	DBG_LOG("Added type '%s'", name->getString().c_str());

	return types.back();
}

Cap::Variable& Cap::Scope::addVariable(Token* name)
{
	variables.emplace_back(name);
	DBG_LOG("Added variable '%s' in scope %lu", name->getString().c_str(), d);
	return variables.back();
}

Cap::Function* Cap::Scope::findFunction(Token* name)
{
	for(auto& it : functions)
	{
		if(it.name->tokenEquals(name))
			return &it;
	}

	return parent == nullptr ? nullptr : parent->findFunction(name);
}

Cap::Variable* Cap::Scope::findVariable(Token* name)
{
	DBG_LOG("Listing variables of scope %lu", d);
	for(auto& it : variables)
	{
		DBG_LOG("var '%s'", it.name->getString().c_str());
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

Cap::SyntaxTreeNode* Cap::Scope::validate(Cap::ValidationResult& result)
{
	DBG_LOG("Validating scope %lu", d);
	SyntaxTreeNode* errorAt = validateNode(&root, result);

	if(result != ValidationResult::Success)
		return errorAt;

	for(auto& f : functions)
	{
		DBG_LOG("Validating function %s", f.name->getString().c_str());

		errorAt = f.scope->validate(result);
		if(result != ValidationResult::Success)
			return errorAt;
	}

	return nullptr;
}

Cap::SyntaxTreeNode* Cap::Scope::validateNode(SyntaxTreeNode* n, ValidationResult& result)
{
	if(n->type == SyntaxTreeNode::Type::Value && n->value->type == TokenType::Identifier)
	{
		Variable* v = findVariable(n->value);
		Function* f = findFunction(n->value);
		Type* t = findType(n->value);
		
		//	Make sure that the given identifier actually exists
		if(!v && !f && !t)
		{
			result = ValidationResult::IdentifierNotFound;
			return n;
		}

		//	Only allow types in expressions when it's a variable type initialization
		if(t && n->parent->type != SyntaxTreeNode::Type::Assign)
		{
			result = ValidationResult::InvalidOperand;
			return n;
		}

		//	Check for assignments and initialize a type for a variable if necessary
		if(n == n->parent->left.get() && n->parent->type == SyntaxTreeNode::Type::Assign)
		{
			//	Forbid re-assigning a type or a function
			if(t || f)
			{
				result = ValidationResult::InvalidAssign;
				return n;
			}
			
			if(!v->type)
			{
				//	Find whatever value is right after the assignment
				SyntaxTreeNode* afterAssign = findLeftmostNode(n->parent->right.get());

				//	Look for variables or types
				if(afterAssign->value->type == TokenType::Identifier)
				{
					//	Was a type given by name?
					v->type = findType(afterAssign->value);
					if(!v->type)
					{
						Variable* v2 = findVariable(afterAssign->value);

						//	Was a variable assigned
						if(v2) v->type = v2->type;

						//	TODO look for functions as well
					}
				}

				//	Look for literal values
				else
				{
					v->type = Type::findPrimitiveType(afterAssign->value->type);

					if(!v->type)
					{
						DBG_LOG("UNIMPLEMENTED literal '%s'", afterAssign->value->getTypeString());
						result = ValidationResult::InvalidOperand;
						return afterAssign;
					}
				}

				DBG_LOG("Variable '%s' now has type '%s'", v->name->getString().c_str(), v->type->name->getString().c_str());
			}

			else
			{
			}
		}
	}

	SyntaxTreeNode* resultNode;

	//	Validate the left node
	if(n->left && (resultNode = validateNode(n->left.get(), result)))
		return resultNode;

	//	Validate the right node
	if(n->right && (resultNode = validateNode(n->right.get(), result)))
		return resultNode;

	return nullptr;
}

Cap::SyntaxTreeNode* Cap::Scope::findLeftmostNode(SyntaxTreeNode* n)
{
	if(!n->left)
		return n;

	//	TODO handle calls
	return findLeftmostNode(n->left.get());
}
