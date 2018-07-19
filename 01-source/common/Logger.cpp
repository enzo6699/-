#include "Logger.h"
#include <iostream>
#include "./../utils/StringUtil.h"


namespace tce {

	Loger::Loger()
	{
	}

	Loger::~Loger()
	{
		//log4cpp::Category::shutdownForced();
	}

	void Loger::configure(std::string initFileName)
	{
		try
		{
			log4cpp::PropertyConfigurator::configure(initFileName);
			std::cout << "start logger" << std::endl;
		}
		catch (log4cpp::ConfigureFailure ex)
		{
			std::cout << "start logger failed " << ex.what() << std::endl;
		}
	}

	void Loger::debug(int code, const char* file, int line, const char* stringFormat, ...) throw()
	{
		va_list va;
		va_start(va, stringFormat);
		std::string message = StringUtil::vform(stringFormat, va);
		va_end(va);

		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().debug(format.c_str(), code, file, line);
	}

	void Loger::debug(int code, const char* file, int line, const std::string message) throw()
	{
		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().debug(format.c_str(), code, file, line);
	}

	void Loger::info(int code, const char* file, int line, const char* stringFormat, ...) throw()
	{
		va_list va;
		va_start(va, stringFormat);
		std::string message = StringUtil::vform(stringFormat, va);
		va_end(va);

		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().info(format.c_str(), code, file, line);
	}

	void Loger::info(int code, const char* file, int line, const std::string message) throw()
	{
		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().info(format.c_str(), code, file, line);
	}

	void Loger::notice(int code, const char* file, int line, const char* stringFormat, ...) throw()
	{
		va_list va;
		va_start(va, stringFormat);
		std::string message = StringUtil::vform(stringFormat, va);
		va_end(va);

		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().notice(format.c_str(), code, file, line);
	}

	void Loger::notice(int code, const char* file, int line, const std::string message) throw()
	{
		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().notice(format.c_str(), code, file, line);
	}

	void Loger::error(int code, const char* file, int line, const char* stringFormat, ...) throw()
	{
		va_list va;
		va_start(va, stringFormat);
		std::string message = StringUtil::vform(stringFormat, va);
		va_end(va);

		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().error(format.c_str(), code, file, line);
	}

	void Loger::error(int code, const char* file, int line, const std::string message) throw()
	{
		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().error(format.c_str(), code, file, line);
	}

	void Loger::crit(int code, const char* file, int line, const char* stringFormat, ...) throw()
	{
		va_list va;
		va_start(va, stringFormat);
		std::string message = StringUtil::vform(stringFormat, va);
		va_end(va);

		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().crit(format.c_str(), code, file, line);
	}

	void Loger::crit(int code, const char* file, int line, const std::string message) throw()
	{
		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().crit(format.c_str(), code, file, line);
	}

	void Loger::alert(int code, const char* file, int line, const char* stringFormat, ...) throw()
	{
		va_list va;
		va_start(va, stringFormat);
		std::string message = StringUtil::vform(stringFormat, va);
		va_end(va);

		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().alert(format.c_str(), code, file, line);
	}

	void Loger::alert(int code, const char* file, int line, const std::string message) throw()
	{
		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().alert(format.c_str(), code, file, line);
	}

	void Loger::fatal(int code, const char* file, int line, const char* stringFormat, ...) throw()
	{
		va_list va;
		va_start(va, stringFormat);
		std::string message = StringUtil::vform(stringFormat, va);
		va_end(va);

		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().fatal(format.c_str(), code, file, line);
	}

	void Loger::fatal(int code, const char* file, int line, const std::string message) throw()
	{
		std::string format = "[%d] [" + message + "] [%s] [%d]";
		log4cpp::Category::getRoot().fatal(format.c_str(), code, file, line);
	}
}