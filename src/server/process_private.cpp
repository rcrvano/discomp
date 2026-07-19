// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "process_private.h"


/**
 * @class DiscompProcess
 * @brief Calculating Process
 *
 * This is a base class for works with calculating proces
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

#include "process_plugin.h"

/*
 * Constructor
 * */
DiscompProcessPrivate::DiscompProcessPrivate(QObject *parent)
    : QObject( parent )
{
	logMessage( LOG_DEBUG, "Construct new process class Private");
}

/*
 * Destructor
 * */
DiscompProcessPrivate::~DiscompProcessPrivate()
{

};



/***********************************************************************/
/*---------------  Calculating process functions  ---------------------*/
/***********************************************************************/


/*
 * Initialization of the calculating process
 * */
QString DiscompProcessPrivate::getProcessType ( const QString &process_name )
{
	//get package name and scheme name
	if ( process_name.indexOf (".") == -1 ) {
		return "";
	};

	QStringList process_info = process_name.split(".");
	QString package_name = process_info.at(0);
	QString scheme_name  = process_info.at(1);
	if ( package_name == "" || scheme_name == "" ) {
		return "";
	};
	
	QString absolute_process_dir = "";
	//paths
	if ( Config::variables["PackagesDirectory"] != "" ) {
		absolute_process_dir = QDir::convertSeparators ( Config::variables["PackagesDirectory"] + "/" + QString(PACKAGES_DIR) + "/" + package_name + "/" + QString(PROCESSES_DIR) + "/"+ scheme_name );
	} else {
		absolute_process_dir = QDir::convertSeparators ( Config::variables["BaseDirectory"]     + "/"  + QString(PACKAGES_DIR) + "/" + package_name + "/" + QString(PROCESSES_DIR) + "/"+ scheme_name );
	}

	if ( QFile::exists( QDir::convertSeparators ( absolute_process_dir + "/" + QString("%1").arg(SCHEME_CONFIG_FILE) ) ) ) {
		return "xml";
	} else if ( QFile::exists( QDir::convertSeparators ( absolute_process_dir + "/" + QString("%1").arg(SCHEME_JS_CONFIG_FILE) ) ) ) {
		return "js";
	};

	return "";



};


// vim: set fenc=utf-8 tabstop=8 :
