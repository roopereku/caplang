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

	return parent == nullptr ? nullptr : parent->findType(name);
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

	return errorAt;
}

Cap::SyntaxTreeNode* Cap::Scope::validateNode(SyntaxTreeNode* n, ValidationResult& result)
{
	/*	TODO when we get the pooling of nodes done, the validation
	 *	could be done by just looping through the pool instead of recursion */

	DBG_LOG("Validating node '%s' in scope %lu", n->getTypeString(), d);

	//	Identifiers be it values or calls require validation
	if(n->value->type == TokenType::Identifier)
	{
		Function* f = findFunction(n->value);
		Variable* v = findVariable(n->value);
		Type* t = findType(n->value);

		//	Does the identifier exist at all
		if(!v && !f && !t)
		{
			result = ValidationResult::IdentifierNotFound;
			return n;
		}
	}

	SyntaxTreeNode* resultNode;

	if(n->left && (resultNode = validateNode(n->left.get(), result)))
		return resultNode;

	if(n->right && (resultNode = validateNode(n->right.get(), result)))
		return resultNode;

	return nullptr;
}
