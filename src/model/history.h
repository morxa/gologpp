#ifndef GOLOGPP_HISTORY_H_
#define GOLOGPP_HISTORY_H_

#include "language.h"
#include "scope.h"
#include "gologpp.h"


namespace gologpp {


class HistorySemantics : public AbstractSemantics {
public:
	HistorySemantics(History &history);
	virtual ~HistorySemantics();

	virtual shared_ptr<Transition> get_last_transition() = 0;
	virtual void append_exog(shared_ptr<Grounding<AbstractAction>>) = 0;
	virtual void append_sensing_result(shared_ptr<Activity>) = 0;

protected:
	History &history_;
};


class History : public LanguageElement<History>, public NoScopeOwner {
public:
	History() = default;
	History(History &&);

	virtual Scope &parent_scope() override;
	virtual const Scope &parent_scope() const override;

	DEFINE_ATTACH_SEMANTICS

	HistorySemantics &abstract_impl()
	{ return static_cast<HistorySemantics &>(*semantics_); }

	virtual string to_string(const string &pfx) const override;
};



} // namespace gologpp


#endif // GOLOGPP_HISTORY_H_
