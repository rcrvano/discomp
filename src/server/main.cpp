// +--------------------------------------------------------------------+ 
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include <QCoreApplication>
#include <QString>
#include <QFile>
#include <QDir>

#ifdef Q_OS_UNIX
#include <sys/stat.h>	//for start as daemon
#include <sys/wait.h>	//for start as daemon
#include <signal.h>
#endif
#include <stdlib.h>
#include <time.h>


#include "config.h"
#include "server.h"
#include "rpc_server.h"
#include "lib/common.h"
#include "lib/log.h"


#ifdef Q_OS_UNIX
/*For start server as daemon*/
void start_daemon();

/*After receving SIGNAL for exit(ex. SIGTERM) will be executed this func*/
void handler_shutdown(int signum);
#endif

DiscompServer* discomp_server;
DiscompServerRPC* discomp_server_rpc;


int main(int argc, char *argv[])
{
	//parse command line arguments
	QHash<QString, QString> command_arguments;
	Config::parseCommandLineArguments(argc, argv, command_arguments );

	//read config for get path to the log directory
	if ( !Config::read(command_arguments.value("--config")) ) {
		printf("Could't read configuration\n");
		exit(4);
	}
	
	Log::logVerboseOnConsole = command_arguments.value("--verbose").toInt();
	if ( Config::variables["LogLevel"].toInt() >= 8 ) {
		Log::logVerboseOnConsole = 1;
	};
	//set log level
	//Log::currentLogLevel = Config::variables["LogLevel"].toInt();
	
	//clear log files
	if ( Config::variables["LogClear"].toInt() ) {
		Log::clearLogDir();
	} else {
		Log::addMessage( LOG_COMMON, "main", "------------------------------------------------------------");
	}

	Log::addMessage( LOG_COMMON, "main", QString("Starting Discomp Server (version: %1)").arg(VERSION) );


#ifdef Q_OS_UNIX
	//start as daemon	
	if ( command_arguments.value("--daemon") != "" ) {
		if ( command_arguments.value("--daemon") == "1" ) {
			Log::addMessage( LOG_DEBUG, "main", "Terminate parent process and start discomp server as daemon" );
			start_daemon();
		};
	} else if ( Config::variables["Daemon"].toInt() == 1 ) {
		Log::addMessage( LOG_DEBUG, "main", "Terminate parent process and start discomp server as daemon" );
		start_daemon();
	};

	//handle signals
	signal(SIGPIPE, SIG_IGN);
//	struct sigaction oact;
//	sigaction(SIGPIPE, NULL, &oact);
	signal(SIGHUP,  handler_shutdown);
	signal(SIGINT,  handler_shutdown);
	signal(SIGQUIT, handler_shutdown);
	signal(SIGTERM, handler_shutdown);
#endif

	
	//we in the new process. reread configuration
	if ( !Config::read(command_arguments.value("--config")) ) {
		exit(4);
	}
	// define temp directory
	if ( Config::variables["TempDirectory"] == "" ) {
		Config::variables["TempDirectory"] =  Config::variables["BaseDirectory"] + QDir::separator() + "tmp";
		QDir temp_dir;
		if ( !temp_dir.exists( Config::variables["TempDirectory"] ) ) { //create it
			temp_dir.mkpath ( Config::variables["TempDirectory"] );
			QFile::setPermissions ( Config::variables["TempDirectory"], QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
		};
	};


	//initializate event loop
	QCoreApplication app(argc, argv);

	
	//create discomp_server
	Log::addMessage( LOG_DEBUG, "main", "Initializating Discomp Server Class") ;
	discomp_server = new DiscompServer();
	
	Log::addMessage( LOG_DEBUG, "main", QString("Binding port %1").arg(Config::variables["ServerPort"]) );
	if ( !discomp_server->listen(QHostAddress::Any, Config::variables["ServerPort"].toInt()) ) {
//	if ( !discomp_server->listen(QHostAddress("192.168.50.1"), Config::variables["ServerPort"].toInt()) ) {
		Log::addMessage( LOG_ERROR,  "main", QString("Failed to bind to port %1").arg(Config::variables["ServerPort"]) );
		exit(3);
		return 3;
	};

	//create discomp_server_rpc
	Log::addMessage( LOG_COMMON, "main", "Initializating RPC Server") ;
	discomp_server_rpc = new DiscompServerRPC(NULL);
	discomp_server_rpc->setDiscompServer(discomp_server);
	
	Log::addMessage( LOG_DEBUG, "main", QString("Binding port %1").arg(Config::variables["ServerRPCPort"]) );
	if ( !discomp_server_rpc->listen(QHostAddress::Any, Config::variables["ServerRPCPort"].toInt()) ) {
		Log::addMessage( LOG_ERROR,  "main", QString("Failed to bind to port %1").arg(Config::variables["ServerRPCPort"]) );
		exit(3);
		return 3;
	};

	//auto start httpserver
	QProcess http_server;
	if ( Config::variables["HttpServerEnabled"].toInt() == 1 ) { 
		Log::addMessage( LOG_COMMON, "main", "Starting mongoose HTTP server" );
		QStringList arguments;
		arguments << "--config" << command_arguments.value("--config");
		http_server.start ( QDir::convertSeparators(Config::variables["BaseDirectory"] + "/bin/httpserver"), arguments );
	};


	Log::addMessage( LOG_COMMON, "main", "Discomp server started successfully" );


	int app_exec_status = app.exec();
	return app_exec_status;
}


#ifdef Q_OS_UNIX
/*For start server as daemon*/
void start_daemon()
{
	int	i;
	int	daemon_proc;	
	pid_t	pid;

	if ( (pid = fork()) != 0) { 
		//parent terminates
		exit(0);
	}

	// 41st child continues
	// become session leader
	setsid();

	signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) != 0) { 
		// 1st child terminates 
		exit(0);
	};

	// 42nd child continues 
	// for our err_XXX() functions 
	daemon_proc = 1;

	// change working directory 
	chdir("/");

	// clear our file mode creation mask
	umask(0);

	//MAXFD = 64
	for (i = 0; i < 64; i++)
		close(i);
}


/*After receving SIGNAL for exit(ex. SIGTERM) will be executed this func*/
void handler_shutdown(int signum)
{
	Log::addMessage(LOG_COMMON, "main", QString("Exit signal received: %1").arg(signum) );

	delete discomp_server;

	exit (0);
}

#endif


// vim: set fenc=utf-8 tabstop=8 :
