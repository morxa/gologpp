#include "execution.h"
#include "fluent.h"
#include "action.h"
#include "procedural.h"
#include "history.h"
#include "platform_backend.h"
#include "transition.h"

#include <iostream>


namespace gologpp {


AExecutionContext::AExecutionContext(unique_ptr<SemanticsFactory> &&semantics, unique_ptr<PlatformBackend> &&platform_backend)
: platform_backend_(move(platform_backend))
, semantics_(std::move(semantics))
{
	if (!platform_backend_)
		platform_backend_ = std::make_unique<DummyBackend>();
	platform_backend_->set_context(this);
	Clock::clock_source = platform_backend_.get();
}


shared_ptr<Grounding<AbstractAction>> AExecutionContext::exog_queue_pop()
{
	std::lock_guard<std::mutex> { exog_mutex_ };
	shared_ptr<Grounding<AbstractAction>> rv = std::move(exog_queue_.front());
	exog_queue_.pop();
	return rv;
}


shared_ptr<Grounding<AbstractAction>> AExecutionContext::exog_queue_poll()
{
	std::unique_lock<std::mutex> queue_empty_lock { queue_empty_mutex_ };
	queue_empty_condition_.wait(queue_empty_lock, [&] { return !exog_queue_.empty(); });
	return exog_queue_pop();
}


void AExecutionContext::exog_queue_push(shared_ptr<Grounding<AbstractAction>> exog)
{
	std::lock_guard<std::mutex> { exog_mutex_ };
	exog_queue_.push(std::move(exog));
	{
		std::lock_guard<std::mutex> { queue_empty_mutex_ };
		queue_empty_condition_.notify_one();
	}
}

bool AExecutionContext::exog_empty()
{
	std::lock_guard<std::mutex> l(exog_mutex_);
	return exog_queue_.empty();
}

SemanticsFactory &AExecutionContext::semantics_factory()
{ return *semantics_; }

unique_ptr<PlatformBackend> &AExecutionContext::backend()
{ return platform_backend_;}



ExecutionContext::ExecutionContext(unique_ptr<SemanticsFactory> &&semantics, unique_ptr<PlatformBackend> &&exec_backend)
: AExecutionContext(std::move(semantics), std::move(exec_backend))
{}

ExecutionContext::~ExecutionContext()
{}

Clock::time_point ExecutionContext::context_time() const
{ return context_time_; }

History ExecutionContext::run(Block &&program)
{
	History history;
	history.attach_semantics(semantics_factory());

	global_scope().implement_globals(semantics_factory(), *this);

	program.attach_semantics(semantics_factory());
	compile(program);

	while (!final(program, history)) {
		context_time_ = backend()->time();
		while (!exog_empty()) {
			shared_ptr<Grounding<AbstractAction>> exog = exog_queue_pop();
			std::cout << ">>> Exogenous event: " << exog << std::endl;
			exog->attach_semantics(semantics_factory());
			history.abstract_impl().append_exog(exog);
		}

		if (trans(program, history)) {
			shared_ptr<Transition> trans = history.abstract_impl().get_last_transition();
			if (trans) {
				std::cout << "<<< trans: " << trans->str() << std::endl;
				if (trans->hook() == Transition::Hook::STOP)
					backend()->preempt_activity(trans);
				else if (trans->hook() == Transition::Hook::START)
					backend()->start_activity(trans);
				else if (trans->hook() == Transition::Hook::FINISH && trans->target()->senses())
					history.abstract_impl().append_sensing_result(backend()->end_activity(trans));
				else
					backend()->end_activity(trans);
			}
		}
		else {
			std::cout << "=== No transition possible: Waiting for exogenous events..." << std::endl;
			shared_ptr<Grounding<AbstractAction>> exog = exog_queue_poll();
			std::cout << ">>> Exogenous event: " << exog << std::endl;
			exog->attach_semantics(semantics_factory());
			history.abstract_impl().append_exog(exog);
		}
	}

	return history;
}




}
