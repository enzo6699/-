#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/HierarchyMaintainer.hh>

#if defined(WIN32)
//#pragma comment( lib, "ws2_32.lib") 
#endif


//#define tce_logger(name, code, message, ...) tce::Loger::name(code, __FILE__, __LINE__, messagem, ...)

namespace tce {
	class Loger
	{
	public:
		Loger();
		virtual~Loger();

		void configure(std::string initFileName);
	public:

		/**
		*
		*
		* @param code error code.
		* @param file __FILE__ error file.
		* @param line __LINE__ error line.
		* @param stringFormat error msg.
		**/
		static void debug(int code, const char* file, int line, const char* stringFormat, ...) throw();
		static void debug(int code, const char* file, int line, const std::string message) throw();

		/**
		*
		*
		* @param code error code.
		* @param file __FILE__ error file.
		* @param line __LINE__ error line.
		* @param stringFormat error msg.
		**/
		static void info(int code, const char* file, int line, const char* stringFormat, ...) throw();
		static void info(int code, const char* file, int line, const std::string message) throw();

		/**
		*
		*
		* @param code error code.
		* @param file __FILE__ error file.
		* @param line __LINE__ error line.
		* @param stringFormat error msg.
		**/
		static void notice(int code, const char* file, int line, const char* stringFormat, ...) throw();
		static void notice(int code, const char* file, int line, const std::string message) throw();

		/**
		* 
		*
		* @param code error code.
		* @param file __FILE__ error file.
		* @param line __LINE__ error line.
		* @param stringFormat error msg.
		**/
		static void error(int code, const char* file, int line, const char* stringFormat, ...) throw();
		static void error(int code, const char* file, int line, const std::string message) throw();

		/**
		*
		*
		* @param code error code.
		* @param file __FILE__ error file.
		* @param line __LINE__ error line.
		* @param stringFormat error msg.
		**/
		static void crit(int code, const char* file, int line, const char* stringFormat, ...) throw();
		static void crit(int code, const char* file, int line, const std::string message) throw();

		/**
		*
		*
		* @param code error code.
		* @param file __FILE__ error file.
		* @param line __LINE__ error line.
		* @param stringFormat error msg.
		**/
		static void alert(int code, const char* file, int line, const char* stringFormat, ...) throw();
		static void alert(int code, const char* file, int line, const std::string message) throw();

		/**
		*
		*
		* @param code error code.
		* @param file __FILE__ error file.
		* @param line __LINE__ error line.
		* @param stringFormat error msg.
		**/
		static void fatal(int code, const char* file, int line, const char* stringFormat, ...) throw();
		static void fatal(int code, const char* file, int line, const std::string message) throw();
	};
}

#endif