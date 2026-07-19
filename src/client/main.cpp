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

#ifdef Q_OS_LINUX
#include <sys/stat.h>	//for start as daemon
#include <sys/wait.h>	//for start as daemon
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "config.h"
#include "client.h"
#include "monitor.h"
#include "clips_api.h"
#include "client_bind_port.h"
#include "lib/common.h"
#include "lib/file.h"
#include "lib/log.h"
#include "database.h"


#include <QFile>
#include <QDir>
#include <QHash>
#include <QHostInfo>


#ifdef Q_OS_UNIX
/*For start  as daemon*/
void start_daemon();

/*After receving SIGNAL for exit(ex. SIGTERM) will be executed this func*/
void handler_shutdown(int signum);
#endif
void checkConfigVariables ();


void myMessageHandler(QtMsgType type, const char *msg);


QVector <DiscompClient*> discomp_clients_list;
DiscompClientBindPort   *discomp_client_bind_port;

DiscompMonitor* discomp_monitor;
DiscompClips* discomp_clips;


int main(int argc, char *argv[])
{
	//parse command line arguments
	QHash<QString, QString> command_arguments;
	Config::parseCommandLineArguments(argc, argv, command_arguments );

	//read configuration
	if ( !Config::read(command_arguments.value("--config")) ) {
		printf("Could't read configuration\n");
		exit(4);
	}

	Log::addHostnamePrefix = Config::variables.value("LogHostnamePrefix").toInt();
        qInstallMsgHandler(myMessageHandler);

#ifdef Q_OS_UNIX
	//start as daemon	
	if ( Config::variables.value("Daemon").toInt() == 1 ) {
		Log::addMessage( LOG_DEBUG, "main", "Terminate parent process and start discomp client as daemon" );
		start_daemon();
	};

	//handle signals
	signal(SIGHUP,  handler_shutdown);
	signal(SIGINT,  handler_shutdown);
	signal(SIGQUIT, handler_shutdown);
	signal(SIGTERM, handler_shutdown);
#endif

	
	//we in the new process. reread configuration
	if ( !Config::read(command_arguments.value("--config")) ) {
		printf("Could't read configuration\n");
		exit(4);
	}
	Log::addHostnamePrefix = Config::variables.value("LogHostnamePrefix").toInt();
	checkConfigVariables();
	
	Log::addMessage( LOG_COMMON, "main", QString("Starting Discomp Client (version: %1)").arg(VERSION) );

	
	QCoreApplication app(argc, argv);

	//------------------- Bind port on the client side ---------------------/
	if ( Config::variables.value("ClientPort").toInt() ) {
		discomp_client_bind_port = new DiscompClientBindPort;
		if ( !discomp_client_bind_port->listen(QHostAddress::Any, Config::variables.value("ClientPort").toInt()) ) {
			Log::addMessage( LOG_ERROR, "main",  QString("Failed to bind to port %1").arg(Config::variables.value("ClientPort")) );
			exit(6);
		};
		discomp_client_bind_port->discomp_clients_list = &discomp_clients_list;
	};

        
	//------------------- Create Discomp clients ---------------------------/
	int cpu_count = Config::variables.value("CPUCount").toInt() ? Config::variables.value("CPUCount").toInt() : 1;
	if ( cpu_count > MAX_CPU_COUNT ) { 
		Log::addMessage ( LOG_WARNING, "main", "CPU count cannot be more than " + QString("%1").arg(MAX_CPU_COUNT) );
		cpu_count = MAX_CPU_COUNT;
	}
	if ( cpu_count < 1 ) {
		Log::addMessage ( LOG_WARNING, "main", "CPU count cannot be less than 1");
		cpu_count = 1;
	};
	Log::addMessage ( LOG_COMMON, "main", QString("CPU count: %1").arg(cpu_count) );
	for ( int i=0; i<cpu_count; i++ ) {
		Log::addMessage ( LOG_COMMON, "main", QString("Starting client for CPU: %1").arg(i) );

		//create discomp client
		DiscompClient *discomp_client = new DiscompClient( );
		discomp_client->log_file = ( cpu_count > 1 ) ? QString("%1_%2.%3").arg(LOG_FILE).arg(i).arg(LOG_FILE_EXT) : "";

		//hostname + processor num
		discomp_client->setClientName ( QString("%1:%2").arg(QHostInfo::localHostName()).arg(i) );
		discomp_client->setServer     ( Config::variables.value("ServerAddr"), Config::variables.value("ServerPort") );
		discomp_client->setCPUNum     ( i );
		discomp_client->setReconnectInterval ( Config::variables.value("ReconnectInterval").toInt() );
		//discomp_client->InitModules();

		//if we should connect to server after start
		discomp_client->setConnectAfterStart ( true );

		//star thread
		discomp_client->start( );

		discomp_clients_list.append(discomp_client);
	};

        //------------------- Initializing system montitor ---------------------/
        discomp_monitor = new DiscompMonitor(discomp_clients_list.at(0));
        //Log::addMessage ( LOG_COMMON, "main", "Interval:: " + Config::variables.value("MonitorInterval") );
        //Log::addMessage ( LOG_COMMON, "main", QString("Interval int:: ").arg(Config::variables.value("MonitorInterval").toInt()) );
	int interval = Config::variables.value("MonitorInterval").toInt();
	if ( interval < 1 ) { interval = 60; };
        discomp_monitor->startMonitoring( Config::variables.value("MonitorInterval").toInt() );

/*
	//----------------- Initializing database connection -------------------/
        // get path to the sqlite file from the Config and init db connection
        Database::init( Config::variables.value("DatabasePath") );
        Database::connect ( );


*/
        //------------------- Initializing clips -------------------------------/
//        discomp_clips = new DiscompClips();
//        discomp_clips->processClipsFile( );


	//----------------------- Initializate event loop ----------------------/
	int app_exec_status = app.exec();
	return app_exec_status;
};

/**
 * Check all config variables and set correct TempDirectory,
 * ModulesDirectory, LogDirectory, etc.
 */
void checkConfigVariables ()
{
	if ( Config::variables["ReconnectInterval"] == "") {
		Config::variables["ReconnectInterval"] = "60";
	};

	//check and clear temp directory
	if ( Config::variables["TempDirectory"] == "" ) {
		printf("TempDirectory isn't specified in the config file");
		Log::addMessage( LOG_ERROR, "main", "TempDirectory isn't specified in the config file" );
		exit(5);
	};
	
	if ( !QFile::exists(Config::variables["TempDirectory"]) ) {
		QDir dir;
		if ( !dir.mkpath ( Config::variables["TempDirectory"] ) ) {
			printf( QString("Couldn't create TempDirectory: %1\n").arg(Config::variables["TempDirectory"]).toStdString().c_str() );
			exit(5);
		};

	} else if ( !File::recursiveRemoveDirectoryContent(Config::variables["TempDirectory"] + QDir::separator() ) ) {
		printf(QString("Couldn't clear temp directory: " + Config::variables["TempDirectory"] + "\n").toStdString().c_str() );
		Log::addMessage( LOG_ERROR, "main", "Couldn't clear temp directory: " + Config::variables["TempDirectory"] );
//		exit(5);
	};
	QFile::setPermissions ( Config::variables["TempDirectory"], QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
//	Config::variables["TempDirectory"] = File::createTempDirectory ( "client_", Config::variables["TempDirectory"] );


	//check and clear log directory
	if ( Config::variables["LogDirectory"] == "" ) {
		Config::variables["LogDirectory"] = Config::variables["BaseDirectory"] + QDir::separator() + "log";
	};


	if ( !QFile::exists(Config::variables["LogDirectory"]) ) {
		QDir dir;
		if ( !dir.mkpath ( Config::variables["LogDirectory"] ) ) {
			printf( QString("Couldn't create LogDirectory: %1").arg(Config::variables["LogDirectory"]).toStdString().c_str() );
			exit(5);
		};
	} else if ( !File::recursiveRemoveDirectoryContent(Config::variables["LogDirectory"] + QDir::separator() ) ) {
		printf(QString("Couldn't clear log directory: " + Config::variables["LogDirectory"]).toStdString().c_str());
		Log::addMessage( LOG_ERROR, "main", "Couldn't clear log directory: " + Config::variables["LogDirectory"] );
		exit(5);
	};
	QFile::setPermissions ( Config::variables["LogDirectory"], QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );





	//check and clear monitor directory
	if ( Config::variables["MonitorDataDirectory"] == "" ) {
		Config::variables["MonitorDataDirectory"] = Config::variables["BaseDirectory"] + QDir::separator() + "monitor";
	};

	if ( !QFile::exists(Config::variables["MonitorDataDirectory"]) ) {
		QDir dir;
		if ( !dir.mkpath ( Config::variables["MonitorDataDirectory"] ) ) {
			printf( QString("Couldn't create MonitorDataDirectory: %1").arg(Config::variables["MonitorDataDirectory"]).toStdString().c_str() );
			exit(5);
		};
	}
	QFile::setPermissions ( Config::variables["MonitorDataDirectory"], QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );



//	Config::variables["MonitorDataDirectory"] = File::createTempDirectory ( "client_", Config::variables["MonitorDataDirectory"] );
};

/**
 * Show help usage
 * \todo create good Help usage
 */
void showUsageHelp()
{
	printf ( "Discomp client usage help:\n" );
	printf ( "discomp_client.exe --connect\n" );
};


void myMessageHandler(QtMsgType type, const char *msg)
{
        int log_level = LOG_COMMON;

        switch (type) {
        case QtDebugMsg:
                log_level = LOG_DEBUG;
                break;
        case QtWarningMsg:
                log_level = LOG_WARNING;
                break;
        case QtCriticalMsg:
                log_level = LOG_ERROR;
                break;
        case QtFatalMsg:
                log_level = LOG_ERROR;
                break;
        default:
                log_level = LOG_COMMON;
                break;
        }

        Log::addMessage(log_level, "STDOUT", msg);
}



#ifdef Q_OS_UNIX
/*For start as daemon*/
void start_daemon()
{
	int	i;
	int	daemon_proc;	
	pid_t	pid;

	if ( (pid = fork()) != 0)
		exit(0);			/* parent terminates */

	/* 41st child continues */
	setsid();				/* become session leader */

	signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) != 0)
		exit(0);			/* 1st child terminates */

	/* 42nd child continues */
	daemon_proc = 1;			/* for our err_XXX() functions */

	chdir("/");				/* change working directory */

	umask(0);				/* clear our file mode creation mask */

	//MAXFD = 64
	for (i = 0; i < 64; i++)
		close(i);
}


/*After receving SIGNAL for exit(ex. SIGTERM) will be executed this func*/
void handler_shutdown(int signum)
{
	Log::addMessage(LOG_COMMON, "main", QString("Exit signal received: %1").arg(signum) );
	Log::addMessage(LOG_DEBUG,  "main", QString("Signal num:%1").arg(signum) );

	//destruct all clients threads
	for ( int i=0; i < discomp_clients_list.size(); i++ ) {
		DiscompClient *discomp_client = discomp_clients_list[i];
		discomp_client->exit(0);
		delete discomp_client;
	};
	//TODO
	//delete discomp_client;

	delete discomp_client_bind_port;

	exit (0);
}

#endif

// vim: set fenc=utf-8 tabstop=8 :
// vim: set fenc=utf-8 tabstop=8 :
