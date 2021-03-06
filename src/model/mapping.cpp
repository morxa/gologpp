#include "mapping.h"
#include "error.h"
#include "action.h"
#include "utilities.h"

#include <boost/fusion/include/at_c.hpp>


namespace gologpp{

ActionMapping::ActionMapping(
	const string &backend_name,
	vector<fusion_wtf_vector<string, Expression *>> arg_mapping
)
: backend_name_(backend_name)
{
	for (auto &pair : arg_mapping)
		arg_mapping_.emplace(
			boost::fusion::at_c<0>(pair),
			unique_ptr<Expression>(boost::fusion::at_c<1>(pair))
		);
}


const string &ActionMapping::backend_name() const
{ return backend_name_; }


const Expression &ActionMapping::mapped_expr(const string &name) const
{
	auto it = arg_mapping_.find(name);
	if (it == arg_mapping_.end())
		throw Bug("Missing mapping entry for backend argument name '" + name + "`");

	return *it->second;
}

bool ActionMapping::is_mapped(const string &arg_name) const
{ return arg_mapping_.find(arg_name) != arg_mapping_.end(); }


void ActionMapping::attach_semantics(SemanticsFactory &f)
{
	for (auto &pair : arg_mapping_)
		pair.second->attach_semantics(f);
}

Scope &ActionMapping::parent_scope()
{ return action_->scope(); }

const Scope &ActionMapping::parent_scope() const
{ return action_->scope(); }

void ActionMapping::set_action(AbstractAction *action)
{ action_ = action; }

const ActionMapping::ArgMapping &ActionMapping::arg_mapping() const
{ return arg_mapping_; }


string ActionMapping::to_string(const string &pfx) const
{
	string rv = linesep + pfx + '"' + backend_name_ + "\" {" linesep;
	for (auto &pair : arg_mapping_)
		rv += pfx + indent + pair.first + " = " + pair.second->str() + "," linesep;

	if (arg_mapping_.size() > 0)
		rv = rv.substr(0, rv.size() - 2);

	return rv + linesep + pfx + "}" linesep;
}


}
