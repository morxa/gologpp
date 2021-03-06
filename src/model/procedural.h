/**
 * @file procedural.h Header for all classes that represent procedural code.
 */

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include <vector>
#include <memory>
#include <algorithm>

#include "gologpp.h"

#include "language.h"
#include "expressions.h"
#include "error.h"
#include "global.h"
#include "scope.h"
#include "action.h"
#include "reference.h"
#include "fluent.h"

namespace gologpp {


/**
 * @brief A scoped block of procedural code.
 */
class Block : public Expression, public ScopeOwner, public LanguageElement<Block, VoidType> {
public:
	Block(Scope *own_scope, const vector<Expression *> &elements);
	virtual void attach_semantics(SemanticsFactory &) override;

	const vector<SafeExprOwner<VoidType>> &elements() const;

	virtual string to_string(const string &pfx) const override;

private:
	vector<SafeExprOwner<VoidType>> elements_;
};



/**
 * @brief Nondeterministic choice from a set of @ref Statement.
 */
class Choose : public Expression, public ScopeOwner, public LanguageElement<Choose, VoidType> {
public:
	Choose(Scope *own_scope, const vector<Expression *> &alternatives);
	void attach_semantics(SemanticsFactory &) override;

	const vector<SafeExprOwner<VoidType>> &alternatives() const;

	virtual string to_string(const string &pfx) const override;

private:
	vector<SafeExprOwner<VoidType>> alternatives_;
};



/**
 * @brief Classical if-then-else.
 */
class Conditional : public Expression, public NoScopeOwner, public LanguageElement<Conditional, VoidType> {
public:
	Conditional(
		Expression *condition,
		Expression *block_true,
		Expression *block_false
	);

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*condition_, *block_true_, *block_false_)

	const Expression &condition() const;
	const Expression &block_true() const;
	const Expression &block_false() const;

	virtual string to_string(const string &pfx) const override;

protected:
	SafeExprOwner<BoolType> condition_;
	SafeExprOwner<VoidType> block_true_;
	SafeExprOwner<VoidType> block_false_;
};



/**
 * @brief Execute a set of statements in parallel.
 */
class Concurrent : public Expression, public ScopeOwner, public LanguageElement<Concurrent, VoidType> {
public:
	Concurrent(Scope *own_scope, const vector<Expression *> &procs);
	void attach_semantics(SemanticsFactory &) override;

	const vector<SafeExprOwner<VoidType>> &procs() const;

	virtual string to_string(const string &pfx) const override;

private:
	vector<SafeExprOwner<VoidType>> procs_;
};



class AbstractAssignment
: public Expression
, public NoScopeOwner
, public virtual AbstractLanguageElement {
public:
	virtual const Expression &lhs() const = 0;
	virtual const Expression &rhs() const = 0;
};



/**
 * @class Assignment
 * @tparam LhsT The type of the left hand side expression.
 * @brief Type-safe assignment.
 *
 * The type of the right hand side must have the same Expression type as the left hand side.
 * @see Assignment::Assignment.
 */
template<class LhsT>
class Assignment
: public AbstractAssignment
, public LanguageElement<Assignment<LhsT>, VoidType> {
public:
	static_assert(!std::is_base_of<Function, LhsT>::value, "Cannot assign to a function");

	Assignment(LhsT *lhs, Expression *rhs)
	: lhs_(lhs), rhs_(rhs)
	{
		if (lhs_->type().template is<VoidType>())
			throw TypeError("Cannot assign to a void expression");

		ensure_type_equality(*lhs, *rhs);
		lhs_->set_parent(this);
		rhs_->set_parent(this);
	}

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*lhs_, *rhs_)

	const LhsT &lhs() const override
	{ return *lhs_; }

	const Expression &rhs() const override
	{ return *rhs_; }

	virtual string to_string(const string &pfx) const override
	{ return lhs().to_string(pfx) + " = " + rhs().to_string("") + ';'; }

private:
    unique_ptr<LhsT> lhs_;
    unique_ptr<Expression> rhs_;
};



/**
 * @class Pick
 *
 * @brief Nondeterministically pick a variable assignment.
 */
class Pick : public Expression, public ScopeOwner, public LanguageElement<Pick, VoidType> {
public:
	Pick(
		Scope *own_scope,
		const shared_ptr<Variable> &variable,
		const boost::optional<std::vector<Value *>> &domain,
		Expression *statement
	);

	const vector<unique_ptr<Value>> &domain() const;
	const Variable &variable() const;
	const Expression &statement() const;

	virtual void attach_semantics(SemanticsFactory &f) override;

	virtual string to_string(const string &pfx) const override;

private:
	vector<unique_ptr<Value>> domain_;
	shared_ptr<Variable> variable_;
	SafeExprOwner<VoidType> statement_;
};



/**
 * @brief Plan and execute.
 *
 * Resolve all nondeterinisms within a statement so that all its tests succeed and all its actions
 * become executable.
 */
class Search : public Expression, public NoScopeOwner, public LanguageElement<Search, VoidType> {
public:
	Search(Expression *statement);
	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*statement_)

	const Expression &statement() const;

	virtual string to_string(const string &pfx) const override;

protected:
	unique_ptr<Expression> statement_;
};



/**
 * @brief Plan with a reward function up to a search horizon, then execute.
 *
 * Search for a "best" executable path given a reward function, but only up to a
 * certain maximum number of actions (the horizon). Then execute the found action sequence.
 */
class Solve : public Expression, public NoScopeOwner, public LanguageElement<Solve, VoidType> {
public:
	Solve(
		Expression *horizon,
		Reference<Function> *reward,
		Expression *statement
	);

	const Expression &statement() const;
	const Expression &horizon() const;
	const Reference<Function> &reward() const;
	virtual void attach_semantics(SemanticsFactory &implementor) override;
	virtual string to_string(const string &pfx) const override;

private:
	SafeExprOwner<VoidType> statement_;
	SafeExprOwner<NumberType> horizon_;
	unique_ptr<Reference<Function>> reward_;
};



/**
 * @brief Test for a boolean condition. Fail the program if the condition evaluates to false.
 */
class Test : public Expression, public NoScopeOwner, public LanguageElement<Test, VoidType> {
public:
	Test(Expression *expression);
	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*expression_)

	const Expression &expression() const;

	virtual string to_string(const string &pfx) const override;

protected:
	SafeExprOwner<BoolType> expression_;
};



/**
 * @brief Classical while loop.
 */
class While : public Expression, public NoScopeOwner, public LanguageElement<While, VoidType> {
public:
	While(Expression *expression, Expression *stmt);
	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*expression_, *statement_)

	const Expression &expression() const;
	const Expression &statement() const;

	virtual string to_string(const string &pfx) const override;

protected:
	SafeExprOwner<BoolType> expression_;
	SafeExprOwner<VoidType> statement_;
};



/**
 * @brief Return a value from a function.
 */
class Return : public Expression, public NoScopeOwner, public LanguageElement<Return, VoidType> {
public:
	Return(Expression *expr);
	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*expr_)

	const Expression &expression() const;
	virtual string to_string(const string &pfx) const override;

private:
	unique_ptr<Expression> expr_;
};



/**
 * @brief A function, also called a subroutine.
 * A function that returns a @ref Expression is also called a @ref Procedure.
 */
class Function
: public Global
, public ScopeOwner
, public LanguageElement<Function>
{
public:
	Function(
		Scope *own_scope,
		const string &type_name,
		const string &name,
		const vector<shared_ptr<Variable>> &params
	);

	Function(
		Scope *own_scope,
		const string &type_name,
		const string &name,
		const boost::optional<vector<shared_ptr<Variable>>> &params
	);

	const Expression &definition() const;
	void define(Expression *definition);
	virtual void compile(AExecutionContext &ctx) override;

	Reference<Function> *make_ref(const vector<Expression *> &params);
	virtual Expression *ref(const vector<Expression *> &params) override;

	virtual string to_string(const string &pfx) const override;

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(scope(), *definition_)

private:
	SafeExprOwner<VoidType> definition_;
	vector<shared_ptr<Variable>> params_;
};



class DurativeCall
: public Expression
, public NoScopeOwner
, public LanguageElement<DurativeCall, VoidType>
{
public:
	enum Hook {
		START, FINISH, FAIL, STOP
	};

	DurativeCall(Hook hook, Reference<Action> *action);
	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*action_)

	Hook hook() const;
	const Reference<Action> &action() const;
	virtual string to_string(const string &pfx) const override;

private:
	const Hook hook_;
	const unique_ptr<Reference<Action>> action_;
};


string to_string(DurativeCall::Hook);



class FieldAccess
: public Expression
, public NoScopeOwner
, public LanguageElement<FieldAccess>
{
public:
	FieldAccess(Expression *subject, const string &field_name);
	const Expression &subject() const;
	const string &field_name() const;

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*subject_)

	virtual const Type &type() const override;

	string to_string(const string &pfx) const override;

private:
	SafeExprOwner<CompoundType> subject_;
	const string field_name_;
};



class ListAccess
: public Expression
, public NoScopeOwner
, public LanguageElement<ListAccess>
{
public:
	ListAccess(Expression *subject, Expression *index);
	const Expression &subject() const;
	const Expression &index() const;

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*subject_, *index_)

	virtual const Type &type() const override;

	string to_string(const string &pfx) const override;

private:
	SafeExprOwner<ListType> subject_;
	SafeExprOwner<NumberType> index_;
};



class ListLength
: public Expression
, public NoScopeOwner
, public LanguageElement<ListLength, NumberType>
{
public:
	ListLength(Expression *subject);
	const Expression &subject() const;

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*subject_)

	string to_string(const string &pfx) const override;

private:
	SafeExprOwner<ListType> subject_;
};



enum ListOpEnd {
	FRONT, BACK
};



class ListPop
: public Expression
, public NoScopeOwner
, public LanguageElement<ListPop, VoidType>
{
public:
	ListPop(Expression *list, ListOpEnd which_end);
	const Expression &list() const;
	ListOpEnd which_end() const;

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*list_)

	string to_string(const string &pfx) const override;

private:
	SafeExprOwner<ListType> list_;
	ListOpEnd which_end_;
};



class ListPush
: public Expression
, public NoScopeOwner
, public LanguageElement<ListPush, VoidType>
{
public:
	ListPush(Expression *list, ListOpEnd which_end, Expression *what);
	const Expression &list() const;
	ListOpEnd which_end() const;
	const Expression &what() const;

	DEFINE_ATTACH_SEMANTICS_WITH_MEMBERS(*list_, *what_)

	string to_string(const string &pfx) const override;

private:
	SafeExprOwner<ListType> list_;
	ListOpEnd which_end_;
	unique_ptr<Expression> what_;
};



} // namespace gologpp



#endif // PROCEDURE_H


