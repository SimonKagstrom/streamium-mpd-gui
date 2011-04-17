#ifndef __TIMER_HH__
#define __TIMER_HH__

class TimeoutHandler;

class TimerController
{
public:
	TimerController(int ticks_per_ms);

	int arm(TimeoutHandler *which, int ms = 1);

	void disarm(TimeoutHandler *which);

	void tick();

	/* Singleton */
	static TimerController *controller;
	static void init(int ticks_per_ms);

private:
	int n_handlers;
	int m_ticks_per_ms;
	TimeoutHandler **handlers;
};

class TimeoutHandler
{
	friend class TimerController;
public:
	TimeoutHandler()
	{
		this->timeout = 0;
		this->timer_id = -1;
	}

	virtual ~TimeoutHandler();

	void tick()
	{
		this->timeout--;
		if (this->timeout == 0)
			this->timeoutCallback();
	}

	virtual void timeoutCallback() = 0;

protected:
	int timeout;
	int timer_id;
};

#endif /* __TIMER_HH__ */
