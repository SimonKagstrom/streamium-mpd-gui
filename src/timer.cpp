#include "gui.hh"

#include "timer.hh"
#include "utils.hh"

void TimeoutHandler::tick()
{
	this->timeout--;
	if (this->timeout == 0) {
		this->timeoutCallback();
		Gui::gui->m_needs_redraw = true;
	}
}


TimeoutHandler::~TimeoutHandler()
{
	/* If we haven't timed out yet, disarm us */
	TimerController::controller->disarm(this);
}


TimerController::TimerController(int ticks_per_ms)
{
	this->n_handlers = 0;
	this->handlers = NULL;
	this->m_ticks_per_ms = ticks_per_ms;
}

int TimerController::arm(TimeoutHandler *which, int ms)
{
	int i;

	/* Set the timeout */
	which->timeout = ms / this->m_ticks_per_ms;

	if (which->timeout == 0)
		which->timeout = 1;

	/* Re-register? */
	for (i = 0; i < this->n_handlers; i++)
		if (this->handlers[i] == which)
			return i;

	/* Empty slot? */
	for (i = 0; i < this->n_handlers; i++)
		if (this->handlers[i] == NULL)
			break;

	if (i == this->n_handlers)
	{
		this->n_handlers++;
		this->handlers = (TimeoutHandler**)xrealloc(this->handlers,
				this->n_handlers * sizeof(TimeoutHandler*));
	}
	this->handlers[i] = which;
	which->timer_id = i;

	return i;
}

void TimerController::disarm(TimeoutHandler *which)
{
	/*Trying to disarm something which is not armed! */
	if (which->timer_id < 0)
		return;

	panic_if(which->timer_id >= this->n_handlers,
			"timer_id %d is too out of bounds (max %d)\n",
			which->timer_id, this->n_handlers);

	this->handlers[which->timer_id] = NULL;
	which->timer_id = -1;
}

void TimerController::tick()
{
	for (int i = 0; i < this->n_handlers; i++)
	{
		TimeoutHandler *cur = this->handlers[i];

		if (!cur)
			continue;

		if (cur->timeout == 0)
			this->disarm(cur);
		cur->tick();
	}
}



TimerController *TimerController::controller;
void TimerController::init(int ticks_per_ms)
{
	TimerController::controller = new TimerController(ticks_per_ms);
}
