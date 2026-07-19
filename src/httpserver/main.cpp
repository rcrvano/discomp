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

#include "../server/config.h"
#include "httpserver_mongoose.h"
#include "lib/common.h"
#include "lib/log.h"

static struct mg_context *httpserver_mongoose_ctx; 

int main(int argc, char *argv[])
{
	//parse command line arguments
	QHash<QString, QString> command_arguments;
	Config::parseCommandLineArguments(argc, argv, command_arguments );

	//read config for get path to the log directory
	if ( !Config::read(command_arguments.value("--config")) ) {
		qDebug("Error: Could't read configuration\n");
		return 4;
	}

	//initializate event loop
	QCoreApplication app(argc, argv);
	
	qDebug (QString("Starting mongoose httpserver on the port %1").arg(Config::variables["HttpServerPort"]).toStdString().c_str());
	
	
	if ((httpserver_mongoose_ctx = mg_start()) == NULL) {
		(void) printf("%s\n", "Cannot initialize Mongoose context");
		qDebug ( "Error: Cannot initialize Mongoose HTTP server context ");
	}
	mg_set_option(httpserver_mongoose_ctx, "root", Config::variables["HttpDocumentRoot"].toStdString().c_str() );
	mg_set_option(httpserver_mongoose_ctx, "access_log", QString(Log::getLogDirPath() + QDir::separator() + QString("httpserver_access.%1").arg(LOG_FILE_EXT)).toStdString().c_str() );
	mg_set_option(httpserver_mongoose_ctx, "error_log", QString(Log::getLogDirPath() + QDir::separator() + QString("httpserver_error.%1").arg(LOG_FILE_EXT)).toStdString().c_str() );
	mg_set_option(httpserver_mongoose_ctx, "cgi_ext", "php");
	mg_set_option(httpserver_mongoose_ctx, "cgi_interp", QString ( Config::variables["HttpPhpPath"]  ).toStdString().c_str() );
	mg_set_option(httpserver_mongoose_ctx, "cgi_interp_arg2", QString ( "-c" ).toStdString().c_str() );
	mg_set_option(httpserver_mongoose_ctx, "cgi_interp_arg3", QString ( Config::variables["HttpPhpIniPath"]  ).toStdString().c_str() );
	if (mg_get_option(httpserver_mongoose_ctx, "ports") == NULL &&
	    mg_set_option(httpserver_mongoose_ctx, "ports", Config::variables["HttpServerPort"].toStdString().c_str() ) != 1) {
		qDebug( QString("Error: Failed to bind to port %1").arg(Config::variables["HttpServerPort"]).toStdString().c_str() );
	};


	int app_exec_status = app.exec();
	return app_exec_status;
}
