#ifndef GOLOGPP_SCOPE_H_
#define GOLOGPP_SCOPE_H_

#include <vector>
#include <unordered_map>
#include <boost/variant.hpp>
#include <boost/variant/polymorphic_get.hpp>

#include "language.h"
#include "utilities.h"
#include "atoms.h"
#include "gologpp.h"
#include "reference.h"

namespace gologpp {


Scope &global_scope();


class Scope : public LanguageElement<Scope> {
private:
	template<class K, class V>
	using unordered_map = std::unordered_map<K, V>;

	template<class... GologTs>
	using globals_map = std::unordered_map<Identifier, boost::variant<shared_ptr<GologTs>...>>;

public:
	typedef globals_map<
		Procedure,
		BooleanFunction,
		NumericFunction,
		Action,
		ExogAction,
		BooleanFluent,
		NumericFluent
	> globals_map_t;

	Scope(Expression *owner, const vector<shared_ptr<AbstractVariable>> &variables = {}, Scope &parent_scope = global_scope());

	Scope(Scope &&);

	Scope(const Scope &) = delete;
	Scope &operator = (const Scope &) = delete;

	template<class ExpressionT>
	shared_ptr<Variable<ExpressionT>> variable(const string &name)
	{
		auto it = variables_.find(name);
		shared_ptr<Variable<ExpressionT>> rv;
		if (it != variables_.end())
			rv = std::dynamic_pointer_cast<Variable<ExpressionT>>(it->second);
		else {
			rv.reset(new Variable<ExpressionT>(name, *this));
			variables_.emplace(name, rv);
		}
		return rv;
	}

	template<class ExpressionT>
	Variable<ExpressionT> *variable_raw(const string &name)
	{ return variable<ExpressionT>(name).get(); }

	shared_ptr<AbstractVariable> variable(const string &name) const;

	vector<shared_ptr<Expression>> variables(const vector<string> &names) const;
	shared_ptr<Scope> parent_scope();

	void implement(Implementor &implementor);
	const unordered_map<string, shared_ptr<AbstractVariable>> &map() const;

	static Scope &global_scope()
	{ return global_scope_; }

	void set_owner(Expression *owner);
	shared_ptr<Expression> owner() const;

	template<class GologT = Global>
	shared_ptr<GologT> lookup_global(const Identifier &id)
	{ return std::dynamic_pointer_cast<GologT>((*globals_)[id]); }

private:
	Scope();

	static Scope global_scope_;
	Scope &parent_scope_;
	Expression *owner_;
	unordered_map<string, shared_ptr<AbstractVariable>> variables_;

	shared_ptr<unordered_map<Identifier, shared_ptr<Global>>> globals_;
};


} // namespace gologpp







#endif