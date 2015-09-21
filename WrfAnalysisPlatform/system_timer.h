#ifndef SYSTEM_TIMER_H_
#define SYSTEM_TIMER_H_

#include <QDebug>
#include <QTime>

class SystemTimer
{
public:
	static SystemTimer* GetInstance();
	static bool DeleteInstance();

	void BeginTimer();
	void EndTimer();
	void PrintTime();

protected:
	SystemTimer();
	~SystemTimer();

	static SystemTimer* instance;

private:
	QTime begin_time_;
	int interval_;
};

#endif