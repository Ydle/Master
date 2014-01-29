/**
 * Ydle - v0.5 (http://www.ydle.fr)
 * Home automotion project lowcost and DIY, all questions and information on http://www.ydle.fr
 *
 * @package Master
 * @license CC by SA
 **/


#include <wiringPi.h>

#include "master.h"

#include <cstring>
#include <sstream>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <map>

#include "SettingsParser.h"
#include "IhmCommunicationThread.h"
#include "logging.h"
#include "restLoggerDestination.h"
#include "webServer.h"
#include "NodeRequestHandler.h"

using namespace std;
protocolRF *g_ProtocolRF;

// ----------------------------------------------------------------------------
/**
 Routine: LongToString()
 Inputs: Long

 Outputs: string


 */
// ----------------------------------------------------------------------------
std::string LongToString(long value) {
	string szresult;
	stringstream szstream;

	szstream << value;
	szresult = szstream.str();
	return szresult;
}

// ----------------------------------------------------------------------------
/**
 Routine: scheduler_realtime()
 Inputs:


 Outputs:

 switch to 'real time'
 */
// ----------------------------------------------------------------------------
void scheduler_realtime() {
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &p) == -1) {
		perror("Failed to switch to realtime scheduler.");
	}
}

// ----------------------------------------------------------------------------
/**
 Routine: scheduler_standard()
 Inputs:


 Outputs:

 switch to normal
 */
// ----------------------------------------------------------------------------
void scheduler_standard() {
	struct sched_param p;
	p.__sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &p) == -1) {
		perror("Failed to switch to normal scheduler.");
	}
}

/* This is the critical section object (statically allocated). */
static pthread_mutex_t listcmd_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t g_mutexSynchro = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // Mutex used to prevent reading RF when we Sending something
static list<protocolRF::Frame_t> ListCmd;

// ----------------------------------------------------------------------------
/**
 Routine: AddToListCmd()
 Inputs: protocolRF::Frame_t cmd


 Outputs:

 Add a trame for IHM
 */
// ----------------------------------------------------------------------------
void AddToListCmd(protocolRF::Frame_t cmd) {
	/* Enter the critical section */

	protocolRF::Frame_t tmp;
	memcpy(&tmp, &cmd, sizeof(cmd));

	pthread_mutex_lock(&listcmd_mutex);
	ListCmd.push_back(tmp);
	/*Leave the critical section */
	pthread_mutex_unlock(&listcmd_mutex);
}

void exit_handler(int s) {
	YDLE_INFO << "Caught signal: " << s;
	scheduler_standard();
	exit(1);
}


// ----------------------------------------------------------------------------
/**
 Routine: main()
 Inputs:


 Outputs:

 Main entry program
 */
// ----------------------------------------------------------------------------
int main(int argc, char** argv) {
	// Init logging system
	ydle::StdErrorLogDestination *stderr_log =
			new ydle::StdErrorLogDestination(ydle::YDLE_LOG_DEBUG);

	ydle::InitLogging(ydle::YDLE_LOG_DEBUG, stderr_log);

	// Parse command line and config file
	ydle::SettingsParser* s;

	s = new ydle::SettingsParser();

	if(s->parseCommandLine(argc, argv) == 0){
		return 0;
	}

	if(s->parseConfigFile() == 0){
		return 0;
	}

	map<string, string> master_config;
	master_config = s->getConfig();
	if(master_config.size() == 0){
		return 0;
	}

	if (setuid(0)) {
		perror("setuid");
		return 1;
	}

	// Config read ok, adjust the log level according to the user configuration
	stderr_log->setLevel(atoi(master_config["log_stderr_level"].c_str()));

	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	std::stringstream ihm;
	ihm << master_config["ihm_address"] << ":" << master_config["ihm_port"];
	ydle::restLoggerDestination *restLog =
			new ydle::restLoggerDestination(
					(ydle::log_level)atoi(master_config["log_rest_level"].c_str()),
					ihm.str()
			);
	restLog->Init();

	ydle::InitLogging(ydle::YDLE_LOG_DEBUG, restLog);

	//log("Program start");
	YDLE_DEBUG << "Program start";
	//End the program if the wiringPi library is not found
	if (wiringPiSetup() == -1) {
		YDLE_FATAL << "Error : Library wiringPi not found" << std::endl;
		return -1;
	}
	int rx_pin = atoi(master_config["rx_pin"].c_str());
	int tx_pin = atoi(master_config["tx_pin"].c_str());

	g_ProtocolRF = new protocolRF(rx_pin, tx_pin);

	// comment if you don't want debug
	g_ProtocolRF->debugMode();

	WebServer::HTTPServer::HTTPServerOptions options;
	int port = atoi(master_config["port"].c_str());
	options.port = port;

	// Launch webserver
	WebServer::HTTPServer *server;
	server = new WebServer::HTTPServer(options);

	pthread_t threadListenRF;

	// Start listen thread
	if (pthread_create(&threadListenRF, NULL, protocolRF::listenSignal,
			g_ProtocolRF) < 0) {
		YDLE_FATAL << "Can't start ListenRF thread";
	}

	try {
		WebServer::NodeRequestHandler* n = new WebServer::NodeRequestHandler(
				g_ProtocolRF);
		server->RegisterHandler("/node", n);
		server->Init();
		server->Run();

		std::stringstream temp;
		temp << master_config["ihm_address"] << ":" << master_config["ihm_port"];
		ydle::IhmCommunicationThread com(temp.str(), &ListCmd, &listcmd_mutex);
		com.start();

		// TODO: (Denia) Temporaire
		while (1) {
			sleep(1000);
		}
	} catch (exception const& e) {
		YDLE_FATAL << "Something failed.  " << e.what() << endl;
	}

	YDLE_INFO << "program end"; // end execution .
}
