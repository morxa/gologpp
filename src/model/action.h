#ifndef GOLOGPP_ACTION_H_
#define GOLOGPP_ACTION_H_

#include <vector>
#include <set>
#include <memory>
#include <algorithm>

#include "gologpp.h"

#include "error.h"
#include "formula.h"
#include "atoms.h"
#include "language.h"
#include "scope.h"

namespace gologpp {


class AbstractEffectAxiom;


class AbstractAction : public Statement, public Global, public virtual AbstractLanguageElement {
public:
	AbstractAction(Scope *own_scope, const string &name, const vector<shared_ptr<AbstractVariable>> &args);

	AbstractAction(const AbstractAction &) = delete;
	AbstractAction(AbstractAction &&other) = default;

	virtual ~AbstractAction() override = default;

	const vector<unique_ptr<AbstractEffectAxiom>> &effects() const;
	void add_effect(AbstractEffectAxiom *effect);

	Scope &scope();
	const Scope &scope() const;

protected:
	unique_ptr<Scope> scope_;

	vector<unique_ptr<AbstractEffectAxiom>> effects_;
};


class Action : public AbstractAction, public LanguageElement<Action> {
public:
	using AbstractAction::AbstractAction;
	Action(const Action &) = delete;
	Action(Action &&) = default;

	const BooleanExpression &precondition() const;

	void set_precondition(BooleanExpression *);

	virtual void implement(Implementor &) override;

protected:
	unique_ptr<BooleanExpression> precondition_;
};




class ExogAction : public AbstractAction, public LanguageElement<ExogAction> {
public:
	using AbstractAction::AbstractAction;
	ExogAction(const Action &) = delete;
	ExogAction(ExogAction &&) = default;
	virtual void implement(Implementor &) override;
};


class AbstractTransition : public virtual AbstractLanguageElement {
public:
	AbstractTransition(shared_ptr<Action> action, vector<unique_ptr<AbstractConstant>> args);

	const Action &action() const;
	const vector<unique_ptr<AbstractConstant>> &args() const;

protected:
	shared_ptr<Action> action_;
	vector<unique_ptr<AbstractConstant>> args_;
};


class Transition : public AbstractTransition, public LanguageElement<Transition> {
public:
	using AbstractTransition::AbstractTransition;
	Transition(const Transition &) = delete;
	Transition(Transition &&) = delete;
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