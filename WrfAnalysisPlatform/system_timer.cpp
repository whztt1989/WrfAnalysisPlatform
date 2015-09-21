#include "system_timer.h"

SystemTimer* SystemTimer::instance = 0;

SystemTimer* SystemTimer::GetInstance(){
	if ( instance == 0 ){
		instance = new SystemTimer;
	}
	return instance;
}

bool SystemTimer::DeleteInstance(){
	if ( instance != 0 ){
		delete instance;
		instance = 0;
		return true;
	}
	return false;
}

SystemTimer::SystemTimer(){
	
}

SystemTimer::~SystemTimer(){

}

void SystemTimer::BeginTimer(){
	begin_time_ = QTime::currentTime();
}

void SystemTimer::EndTimer(){
	QTime end_time = QTime::currentTime();
	interval_ = begin_time_.msecsTo(end_time);
}

void SystemTimer::PrintTime(){
	qDebug() << "Time: " << interval_;
}