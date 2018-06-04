#include "scope.h"
#include "atoms.h"

namespace gologpp {

Scope Scope::global_scope_;


Scope::Scope(Expression *owner, const vector<shared_ptr<AbstractVariable>> &variables, Scope &parent_scope)
: parent_scope_(parent_scope)
, owner_(owner)
{
	for (const shared_ptr<AbstractVariable> &v : variables)
		variables_.emplace(v->name(), v);
}


Scope::Scope(Scope &&other)
: parent_scope_(other.parent_scope_)
, variables_(std::move(other.variables_))
{}


shared_ptr<AbstractVariable> Scope::variable(const string &name) const
{
	auto it = variables_.find(name);
	shared_ptr<AbstractVariable> rv;
	if (it != variables_.end())
		rv = it->second;
	return rv;
}


vector<shared_ptr<Expression>> Scope::variables(const vector<string> &names) const
{
	vector<shared_ptr<Expression>> rv;
	for (const string &name : names)
		rv.push_back(std::dynamic_pointer_cast<Expression>(variable(name)));
	return rv;
}


void Scope::implement(Implementor &implementor)
{
	if (!impl_) {
		impl_ = implementor.make_impl(*this);
		for (auto &entry : variables_)
			entry.second->implement(implementor);
	}
}


const std::unordered_map<string, shared_ptr<AbstractVariable>> &Scope::map() const
{ return variables_; }


void Scope::set_owner(Expression *owner)
{ owner_ = owner; }


shared_ptr<Expression> Scope::owner() const
{ return owner_->shared_from_this(); }


Scope &global_scope()
{ return Scope::global_scope(); }


} // namespace gologpp
