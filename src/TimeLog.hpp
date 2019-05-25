#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	struct TimeStamp {
		std::chrono::system_clock::time_point time;
		std::string toString();
	};

	enum LogLevel {
		ToConsole,
		Debug,
		Exceptions
	};

	class Logger {

		struct LogEntry {
			TimeStamp timestamp;
			std::string entry;
		};

		std::vector<LogEntry> logCache;

	public:

		void log(std::string);

	};

	class Timer {

		std::stack<TimeStamp> measures;

	public:

		TimeStamp timeStamp();

		void start();
		double end();

	};

}
