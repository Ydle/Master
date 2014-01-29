/*
 * Settings.cpp
 *
 *  Created on: Jan 11, 2014
 *      Author: denia
 */

#include "SettingsParser.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include <libconfig.h++>
#include <getopt.h>

using namespace std;


namespace ydle {

SettingsParser::SettingsParser() {
	this->g_configuration = new libconfig::Config();
	this->config_file = "";
}

SettingsParser::~SettingsParser() {
	delete g_configuration;
}

int SettingsParser::parseSettings(){
	return 0;
}

void SettingsParser::showConfig(){
	int port, tx_pin, rx_pin;
	string listen_address;

	if(this->g_configuration->lookupValue("master.port", port)
		 && this->g_configuration->lookupValue("master.address", listen_address)
		 && this->g_configuration->lookupValue("master.rx_pin", rx_pin)
		 && this->g_configuration->lookupValue("master.tx_pin", tx_pin)){
		std::cout << "Master config " << std::endl;
		std::cout << "Port : " << port << std::endl;
		std::cout << "Listen address : " << listen_address << std::endl;
	} else {
		std::cout << "Config incomplete";
	}
}

int SettingsParser::parseCommandLine(int argc, char** argv) {
	int c;
	while ((c = getopt (argc, argv, "c:?::h::")) != -1){
		std::cout << c << std::endl;
		switch (c)
		{
		case 'h':
			this->showUsage();
			return 0;
		case 'c':
			config_file = optarg;
			break;
		case '?':
			if (optopt == 'c')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
						"Unknown option character `\\x%x'.\n",
						optopt);
			return 0;
		default:
			break;
		}
	}
	if(config_file.length() <= 1){
		std::cout << "A parameter is missing, check your arguments" << std::endl;
		this->showUsage();
		return 0;
	}else{
		return 1;
	}
}
bool SettingsParser::configIsOk(){
	return false;
}

int SettingsParser::parseConfigFile(std::string config_file){
	try{
		this->g_configuration->readFile(config_file.c_str());
	}catch(libconfig::ParseException & pex){
		std::cout << "Error in config file : " << pex.what() << std::endl;
		std::cout << "at line: " << pex.getLine() << " " << pex.getError() << std::endl;
		return 0;
	}catch(std::exception & ex){
		std::cout << "Unable to read the config file : " << ex.what() << std::endl;
		return 0;
	}catch(...){
		std::cout << "Unknown exception" << std::endl;
		return 0;
	}
	return 1;
}

int SettingsParser::parseConfigFile(){
	return this->parseConfigFile(this->config_file);
}

void SettingsParser::showUsage(){
	std::cout << "Usage: ydlemaster -c [config file]" << std::endl;
	std::cout << "Example : ydlemaster -c ./ydle.conf" << std::endl;
	std::cout << "Options: "<< std::endl;
	std::cout << "Mandatory" << std::endl;
	std::cout << " -c : Path to config file" << std::endl;
	std::cout << " -h : Print this messsage" << std::endl;
}
/*
 * getConfig : Return a map<string, string> object with the configuration
 * En attendant mieux...
 * */
std::map<std::string, std::string> SettingsParser::getConfig(){
	std::map<std::string, std::string> conf;

	string port, tx_pin, rx_pin, ihm_address, ihm_port;
	string listen_address;

	string rest_level, stderr_level;
	if(this->g_configuration->lookupValue("master.port", port)
		 && this->g_configuration->lookupValue("master.address", listen_address)
		 && this->g_configuration->lookupValue("master.rx_pin", rx_pin)
		 && this->g_configuration->lookupValue("master.tx_pin", tx_pin)
		 && this->g_configuration->lookupValue("master.ihm_address", ihm_address)
		 && this->g_configuration->lookupValue("master.ihm_port", ihm_port)){

		conf.insert( std::pair<string, string>("port", port));
		conf.insert( std::pair<string, string>("address", listen_address));
		conf.insert( std::pair<string, string>("rx_pin", rx_pin));
		conf.insert( std::pair<string, string>("tx_pin", tx_pin));
		conf.insert( std::pair<string, string>("ihm_address", ihm_address));
		conf.insert( std::pair<string, string>("ihm_port", ihm_port));
	}else{
		std::cout << "Error...." << std::endl;
	}
	if(this->g_configuration->lookupValue("master.logger.rest.level", rest_level)){
		conf.insert( std::pair<string, string>("log_rest_level", rest_level));
	} else {
		conf.insert( std::pair<string, string>("log_rest_level", "1"));
	}

	if(this->g_configuration->lookupValue("master.logger.stderr.level", stderr_level)){
		conf.insert( std::pair<string, string>("log_stderr_level", stderr_level));
	} else {
		conf.insert( std::pair<string, string>("log_stderr_level", "1"));
	}

	return conf;
}

int SettingsParser::writeConfigFile(){
	return 1;
}

} /* namespace ydle */

