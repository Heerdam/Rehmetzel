
#include "TimeLog.hpp"

using namespace Heerbann;

void Logger::log(std::string _id) {
	//logCache.emplace_back(LogEntry{ TIMESTAMP, _id });
	std::cout << _id << std::endl;
}

TimeStamp Timer::timeStamp() {
	auto start = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(start);
	
	return TimeStamp();
}

void Timer::start() {
	measures.push(TIMESTAMP);
}

double Timer::end() {
	std::chrono::duration<double> diff = TIMESTAMP.time - measures.top().time;
	measures.pop();
	return diff.count();
}

std::string TimeStamp::toString() {
	std::time_t t = std::chrono::system_clock::to_time_t(time);
	//return std::ctime(&t);
	return "";
}
