#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

#include <string>
#include <iostream>
#include <map>
#include <stdexcept>

namespace tce {

	class ConfigureFailure : public std::runtime_error {
	public:
		/**
		* Constructor.
		* @param reason String containing the description of the exception.
		*/
		ConfigureFailure(const std::string& reason);
	};

	class Properties : public std::map<std::string, std::string>
	{
	public:
		static Properties& Instance();
	public:
		Properties();
		virtual~Properties();

	public:
		virtual void load(const std::string& initFileName) throw (ConfigureFailure);
		virtual void load(std::istream& in);
		virtual void save(std::ostream& out);

		virtual float getFloat(const std::string& property, float defaultValue);
		virtual int getInt(const std::string& property, int defaultValue);
		virtual bool getBool(const std::string& property, bool defaultValue);
		virtual std::string getString(const std::string& property,
			const char* defaultValue);

	protected:
		virtual void _substituteVariables(std::string& value);
	};

}

#endif