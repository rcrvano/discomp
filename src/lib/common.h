// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QHash>

#include "config.h"

#ifndef CONFIG_FILE
#define CONFIG_FILE ""
#endif
#ifndef PRIVATE_CONFIG_FILE
#define PRIVATE_CONFIG_FILE ""
#endif
namespace Config {
	extern QHash<QString,QString> variables; 
	
	int    read                      ( QString file_path );
	void   parseCommandLineArguments ( int argc, char* argv[], QHash<QString, QString> &command_arguments_hash );
	void   errorMsg                  ( const QString &msg );
};

#endif


// vim: set fenc=utf-8 tabstop=8 :
