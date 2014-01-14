/*
 * Settings.h

 *
 *  Created on: Jan 11, 2014
 *      Author: denia
 */
#include <map>
#include <libconfig.h++>

#ifndef SETTINGS_H_
#define SETTINGS_H_

namespace ydle {

class SettingsParser {
private:
	libconfig::Config * g_configuration;
	std::string config_file;
public:
	SettingsParser();
	int parseCommandLine(int argc, char **argv);
	int parseConfigFile(std::string config_file);
	int parseConfigFile();
	int writeConfigFile();
	int parseSettings();
	bool configIsOk();
	void showUsage();
	void showConfig();
	virtual ~SettingsParser();
	std::map<std::string, std::string> getConfig();
};

} /* namespace ydle */

#endif /* SETTINGS_H_ */
