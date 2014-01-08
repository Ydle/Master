/**
 * Ydle - v0.5 (http://www.ydle.fr)
 * Home automotion project lowcost and DIY, all questions and information on http://www.ydle.fr
 *
 * @package Master
 * @license CC by SA
 **/

#ifdef RASPBERRY
#include <wiringPi.h>
#else
#include <unistd.h>
#endif

#include "master.h"

#include <cstring>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <map>


#include "IhmCommunicationThread.h"
#include "logging.h"
#include "webServer.h"

#include "NodeRequestHandler.h"

#define DEFAULT_HTTP_PORT 8888
#define DEFAULT_IHM_ADDRESS "192.168.1.9"

using namespace std;

// global pin used for emit
int g_pinTx = 1;

// global pin used for receive
int g_pinRx = 0;

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
	ydle::InitLogging(ydle::YDLE_LOG_DEBUG, ydle::YDLE_LOG_STDERR);

	if (setuid(0)) {
		perror("setuid");
		return 1;
	}

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	scheduler_realtime();

	//log("Program start");
	YDLE_DEBUG << "Program start";
	//End the program if the wiringPi library is not found
	if (wiringPiSetup() == -1) {
		YDLE_FATAL << ("Lib Wiring PI not found...");
		return -1;
	}

	g_ProtocolRF = new protocolRF(g_pinRx, g_pinTx);

	// comment if you don't want debug
	g_ProtocolRF->debugMode();

	WebServer::HTTPServer::HTTPServerOptions options;
	options.port = DEFAULT_HTTP_PORT;

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

		ydle::IhmCommunicationThread com(DEFAULT_IHM_ADDRESS, &ListCmd, &listcmd_mutex);
		com.start();

		// TODO: (Denia) Temporaire
		while (1) {
			sleep(1000);
		}
	} catch (exception const& e) {
		YDLE_FATAL << "Something failed.  " << e.what() << endl;
	}

	YDLE_INFO << "program end"; // end execution .

	scheduler_standard();
}
