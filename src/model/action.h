#ifndef GOLOGPP_ACTION_H_
#define GOLOGPP_ACTION_H_

#include <vector>
#include <set>
#include <memory>
#include <algorithm>

#include <boost/optional.hpp>

#include "gologpp.h"

#include "error.h"
#include "formula.h"
#include "atoms.h"
#include "language.h"
#include "scope.h"
#include "reference.h"

namespace gologpp {


class AbstractEffectAxiom;


class AbstractAction
: public Global
, public ScopeOwner
, public virtual AbstractLanguageElement {
public:
	typedef Statement expression_t;

	AbstractAction(Scope *own_scope, const string &name, const vector<shared_ptr<AbstractVariable>> &args);

	AbstractAction(const AbstractAction &) = delete;
	AbstractAction(AbstractAction &&other) = default;

	virtual ~AbstractAction() override = default;

	const vector<unique_ptr<AbstractEffectAxiom>> &effects() const;
	void add_effect(AbstractEffectAxiom *effect);

	virtual void compile(AExecutionContext &ctx) override;
	virtual string to_string(const string &pfx) const override;

protected:
	vector<unique_ptr<AbstractEffectAxiom>> effects_;
};


class Action : public AbstractAction, public LanguageElement<Action> {
public:
	typedef AbstractAction abstract_t;

	Action(Scope *own_scope, const string &name, const vector<shared_ptr<AbstractVariable>> &args);

	Action(const Action &) = delete;
	Action(Action &&) = default;

	const BooleanExpression &precondition() const;

	void set_precondition(BooleanExpression *);

	void define(
		boost::optional<BooleanExpression *> precondition,
		boost::optional<vector<AbstractEffectAxiom *>> effects
	);

	virtual void implement(Implementor &) override;
	virtual string to_string(const string &pfx) const override;
	virtual Expression *ref(Scope &parent_scope, const vector<Expression *> &args) override;

protected:
	unique_ptr<BooleanExpression> precondition_;
};



class ExogAction : public AbstractAction, public LanguageElement<ExogAction> {
public:
	using AbstractAction::AbstractAction;
	ExogAction(const Action &) = delete;
	ExogAction(ExogAction &&) = default;
	virtual void implement(Implementor &) override;
	virtual string to_string(const string &pfx) const override;
};



class AbstractTransition : public virtual AbstractLanguageElement {
public:
	AbstractTransition(const shared_ptr<Action> &action, vector<unique_ptr<AbstractConstant>> &&args);

	const Action &action() const;
	const vector<unique_ptr<AbstractConstant>> &args() const;
	virtual string to_string(const string &pfx) const override;

protected:
	shared_ptr<Action> action_;
	vector<unique_ptr<AbstractConstant>> args_;
};



class Transition : public AbstractTransition, public LanguageElement<Transition> {
public:
	using AbstractTransition::AbstractTransition;
	Transition(const Transition &) = delete;
	Transition(Transition &&) = default;
	virtual void implement(Implementor &) override;
};



class ExogTransition : public AbstractTransition, public LanguageElement<ExogTransition> {
public:
	using AbstractTransition::AbstractTransition;
	ExogTransition(const ExogTransition &) = delete;
	ExogTransition(ExogTransition &&) = default;
	virtual void implement(Implementor &) override;
};



} // namespace gologpp



#endif /* GOLOGPP_ACTION_H_ */
