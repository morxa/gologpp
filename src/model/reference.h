#ifndef GOLOGPP_REFERENCE_H_
#define GOLOGPP_REFERENCE_H_

#include "language.h"
#include "expressions.h"
#include "utilities.h"
#include "gologpp.h"
#include "scope.h"
#include "error.h"
#include "variable.h"
#include "domain.h"

#include <memory>
#include <vector>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>

namespace gologpp {


class AbstractReference
: public virtual AbstractLanguageElement
{
public:
	AbstractReference();
	virtual ~AbstractReference();

	virtual bool bound() const = 0;
	virtual bool consistent() const = 0;

	void ensure_consistent();
};



template<class TargetT, class ArgsT>
class ReferenceBase
: public Expression
, public virtual AbstractReference
, public NoScopeOwner
{
public:
	ReferenceBase(const shared_ptr<TargetT> &target, vector<unique_ptr<ArgsT>> &&args)
	: args_(std::move(args))
	, target_(target)
	{
		size_t idx = 0;
		while (idx < this->args().size() && idx < this->target()->params().size()) {
			ArgsT &arg = *this->args()[idx];
			shared_ptr<Variable> param = this->target()->params()[idx];
			params_to_args_.insert( { param, arg } );
			dynamic_cast<Expression &>(arg).set_parent(this);
			++idx;
		}
		ensure_consistent();
	}


	ReferenceBase(const shared_ptr<TargetT> &target, const vector<ArgsT *> &args)
	: ReferenceBase(target, vector<unique_ptr<ArgsT>>(args.begin(), args.end()))
	{}


	ReferenceBase(const string &target_name, const vector<ArgsT *> &args)
	: ReferenceBase(
		global_scope().lookup_global<TargetT>(target_name, arity_t(args.size())),
		args
	)
	{}

	ReferenceBase(const string &target_name, const boost::optional<vector<ArgsT *>> &args)
	: ReferenceBase(target_name, args.get_value_or({}))
	{}

	ReferenceBase(ReferenceBase<TargetT, ArgsT> &&other)
	: args_(std::move(other.args_))
	, target_(std::move(other.target_))
	{ ensure_consistent(); }


	virtual ~ReferenceBase() override = default;

	TargetT &operator * () const
	{ return target(); }

	TargetT *operator -> () const
	{ return &target(); }

	bool operator == (const ReferenceBase<TargetT, ArgsT> &other) const
	{
		if (this->target() != other.target())
			return false;
		for (arity_t i = 0; i < this->target()->arity(); ++i) {
			if (*this->args()[i] != *other.args()[i])
				return false;
		}
		return true;
	}

	bool operator != (const ReferenceBase<TargetT, ArgsT> &other) const
	{ return !(*this == other); }

	const string &name() const
	{ return target_.lock()->name(); }

	arity_t arity() const
	{ return target_.lock()->arity(); }

	virtual bool bound() const override
	{ return !target_.expired(); }

	const vector<unique_ptr<ArgsT>> &args() const
	{ return args_; }

	vector<unique_ptr<ArgsT>> &args()
	{ return args_; }


	const ArgsT &arg_for_param(shared_ptr<const Variable> param) const
	{
		auto it = params_to_args_.find(param);
		if (it == params_to_args_.end())
			throw Bug(target()->str() + " has no parameter by the name " + param->str());

		return it->second;
	}


	virtual bool consistent() const override
	{
		if (!bound() || args().size() != target()->params().size())
			return false;

		// Compare target argument types with this reference's argument types
		auto it_rarg = args().begin();
		auto it_targ = target()->params().begin();
		for (; it_rarg < args().end() && it_targ < target()->params().end(); ++it_rarg, ++it_targ) {
			const Type &t_ref = (*it_rarg)->type();
			const Type &t_tgt = (*it_targ)->type();
			if (t_ref != t_tgt
				&& !(t_ref.is<SymbolType>() && t_tgt.is<StringType>())
				// TODO: Hack: Allow passing a symbol value to a string argument
				// This is needed because ReadyLog can't deal with strings.
			)
				return false;

			if ((*it_rarg)->is_ref() && !dynamic_cast<AbstractReference &>(**it_rarg).consistent())
				return false;
		}

		return true;
	}


	shared_ptr<TargetT> target()
	{ return target_.lock(); }

	shared_ptr<const TargetT> target() const
	{ return std::dynamic_pointer_cast<const TargetT>(target_.lock()); }


	virtual void attach_semantics(SemanticsFactory &f) override
	{
		for (unique_ptr<ArgsT> &expr : args_)
			expr->attach_semantics(f);
	}

	virtual string to_string(const string &pfx) const override
	{ return pfx + name() + '(' + concat_list(args(), ", ", "") + ')'; }

	virtual const Type &type() const override
	{ return this->target()->type(); }

	size_t hash() const
	{
		size_t rv = this->target()->hash();
		for (const unique_ptr<ArgsT> &c : this->args())
			boost::hash_combine(rv, c->hash());

		return rv;
	}


private:
	vector<unique_ptr<ArgsT>> args_;
	weak_ptr<TargetT> target_;

	std::unordered_map<
		shared_ptr<const Variable>,
		std::reference_wrapper<ArgsT>
	> params_to_args_;
};



template<class TargetT>
class Reference
: public ReferenceBase<TargetT, Expression>
, public LanguageElement<Reference<TargetT>>
{
public:
	using ReferenceBase<TargetT, Expression>::ReferenceBase;
	virtual ~Reference() override = default;

	virtual void attach_semantics(SemanticsFactory &f) override
	{
		if (!this->semantics_) {
			this->semantics_ = f.make_semantics(*this);
			ReferenceBase<TargetT, Expression>::attach_semantics(f);
		}
	}
};


template<class> class Grounding;

template<>
class Grounding<AbstractAction>
: public virtual AbstractReference
{};


class AbstractGrounding
: public virtual AbstractReference
{};


template<class TargetT>
class Grounding
: public std::conditional<
	std::is_base_of<AbstractAction, TargetT>::value,
	Grounding<AbstractAction>, // Have a specific superclass for all Action groundings
	                           // (used for the exogenous event queue)
	AbstractGrounding          // All others use some placeholder
  >::type
, public ReferenceBase<TargetT, Value>
{
public:
	using ReferenceBase<TargetT, Value>::ReferenceBase;

	Grounding(const Grounding<TargetT> &other)
	: ReferenceBase<TargetT, Value>(other.target(), copy(other.args()))
	{}

	virtual ~Grounding() override = default;


	virtual Scope &parent_scope() override
	{ return global_scope(); }

	virtual const Scope &parent_scope() const override
	{ return global_scope(); }


	virtual void attach_semantics(SemanticsFactory &f) override
	{
		if (!this->semantics_) {
			this->semantics_ = f.make_semantics(*this);
			ReferenceBase<TargetT, Value>::attach_semantics(f);
		}
	}


	struct Hash {
		std::size_t operator () (const shared_ptr<Grounding<TargetT>> &t) const
		{ return t->hash(); }
	};


	struct Equals {
		bool operator () (
			const shared_ptr<Grounding<TargetT>> &lhs,
			const shared_ptr<Grounding<TargetT>> &rhs
		) const
		{ return *lhs == *rhs; }
	};
};



template<>
class Reference<Variable>
: public Expression
, public AbstractReference
, public NoScopeOwner
, public LanguageElement<Reference<Variable>>
{
public:
	Reference(const shared_ptr<Variable> &target)
	: target_(target)
	{}

	Reference(Reference<Variable> &&other)
	: target_(std::move(other.target_))
	{}

	Reference(const string &var_name)
	: target_(global_scope().get_var(VarDefinitionMode::DENY, "", var_name))
	{}

	virtual ~Reference() override = default;

	const Variable &operator * () const
	{ return *target(); }

	const Variable *operator -> () const
	{ return target().get(); }

	const string &name() const
	{ return target()->name(); }

	virtual bool bound() const override
	{ return target_.get(); }

	shared_ptr<Variable> target()
	{ return target_; }

	shared_ptr<const Variable> target() const
	{ return std::dynamic_pointer_cast<const Variable>(target_); }

	bool operator == (const Reference<Variable> &other) const
	{ return *target() == *other.target(); }

	bool operator != (const Reference<Variable> &other) const
	{ return !(*this == other); }


	virtual void attach_semantics(SemanticsFactory &implementor) override
	{
		if (semantics_)
			return;
		semantics_ = implementor.make_semantics(*this);
	}


	virtual bool consistent() const override
	{ return bound(); }


	virtual string to_string(const string &pfx) const override
	{ return target()->to_string(pfx); }

	virtual const Type &type() const override
	{ return target()->type(); }

	size_t hash() const
	{ return target()->hash(); }


private:
	shared_ptr<Variable> target_;
};



} // namespace gologpp



namespace std {

template<class TargetT>
struct hash<gologpp::Reference<TargetT>> {
	size_t operator () (const gologpp::Reference<TargetT> &o) const
	{ return o.hash(); }
};


template<class TargetT>
struct hash<gologpp::unique_ptr<gologpp::Reference<TargetT>>> {
	size_t operator () (const gologpp::unique_ptr<gologpp::Reference<TargetT>> &o) const
	{ return o->hash(); }
};

template<class TargetT>
struct equal_to<gologpp::unique_ptr<gologpp::Reference<TargetT>>> {
	bool operator () (
		const gologpp::unique_ptr<gologpp::Reference<TargetT>> &lhs,
		const gologpp::unique_ptr<gologpp::Reference<TargetT>> &rhs
	) const {
		return *lhs == *rhs;
	}
};


}

#endif // GOLOGPP_REFERENCE_H_
