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
#include <QHash>
#include <iostream>

#include "rpc_client.h"
#include "lib/common.h"
#include "lib/file.h"
#include "lib/log.h"
#include "config.h"


DiscompRPC* discomp_rpc_client;

int main(int argc, char *argv[])
{
	//parse command line arguments
	QHash<QString, QString> command_arguments;
	Config::parseCommandLineArguments(argc, argv, command_arguments );

	//read configuration
	if ( !Config::read(command_arguments.value("--config")) ) {
		printf("Could't read configuration\n");
		printf("Exit code: 4\n");
		exit(4);
	}

	//set log level
	Log::logErrorsOnConsole = true;

	//create QCoreApp (for wait when connection to server will be established, etc)
	QCoreApplication app(argc, argv);

	//create discomp server rpc_client
	discomp_rpc_client = new DiscompRPC();
	discomp_rpc_client->parseCommandLine(argc, argv);

	//make connection to server and send request if connected
	QString host = (command_arguments.value("--host") != "") ? command_arguments.value("--host") : Config::variables["ServerIP"];
	QString port = (command_arguments.value("--port") != "") ? command_arguments.value("--port") : Config::variables["ServerPort"];

	discomp_rpc_client->doConnect( host, port );

	int app_exec_status = app.exec();
	return app_exec_status;
};



// vim: set fenc=utf-8 tabstop=8 :
