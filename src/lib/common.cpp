// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include <QTextStream>
#include <QByteArray>
#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QMap>
#include <iostream>
#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "log.h"
#include "md5.h"


/**
 * @brief Functions for works with config files and command line arguments.
 *
 * This namespace provide methods for wotks with config files: extract values, etc.
 * parseCommandLineArgumets get all arguments which user specify n the command line and 
 * generate QHash from them.
 */
namespace Config 
{
	QHash<QString,QString> variables; //!< hash with all config variables. which can be accessed like variables["BaseDirectory"]

	/**
	 * Read config file and fill variables hash
	 * @param file_path - path to the config file
	 * @return boolean status - success or not. 
	 */
	int read ( QString file_path ) 
	{
		//first check is defined config file specified as command argument
		if ( !QFile::exists(file_path) ) { //if specified file not exists
			if ( file_path != "" ) {
				errorMsg(QString("Config file specified as argument doesn't exists:%1").arg(file_path));
				return false;
			};
			
			file_path = QString(PRIVATE_CONFIG_FILE).replace("~", QDir::homePath()); //private filepath
			if ( !QFile::exists(file_path) ) { //if private config not exists. use global
				file_path = CONFIG_FILE;
			};
		} 
		
		//open file
		QFile file (file_path);
		if (!file.open(QIODevice::ReadOnly)) {
			errorMsg("Could't open config file " + file_path + " for reading");
			return false;
		};

		//read file
		QTextStream in(&file);
		while ( !in.atEnd() ) {
			
			QString     conf_line      = in.readLine();
			
			//skip commented lines
			if ( conf_line.contains(QRegExp("^#")) ) {
				continue;
			};

			//get var_name, var_value
			QStringList conf_line_list = conf_line.split(QRegExp("\\s*=\\s*"));
			if ( conf_line_list.size() < 2 ) {
				continue;
			};
			QString var_name  = conf_line_list[0].trimmed();
			QString var_value = conf_line_list[1].trimmed().remove('\'').remove('"');
#ifdef Q_OS_WIN
			var_value.replace("{USERNAME}",getenv("USERNAME"));
#else
			var_value.replace("{USERNAME}",getenv("USER"));
#endif
			variables[var_name] = var_value;
		};

		//close file
		file.close();

		return true;
	};

	/**
	 * Display error message if some errors occur when processing config file. 
	 * Message will be displayed in the popup window for windows and will be writed
	 * to the stderror on the windows.
	 * @param msg - error message.
	 *
	 * @todo diplay dialog window for win32
	 */
	void errorMsg ( const QString &msg ) {
		std::cerr << msg.toStdString() << std::endl;
	};

	/**
	 * Get all command line arguments, and put them to the command_arguments_hash
	 * @param argc - count of arg parameters
	 * @param argv - values of arg parameters
	 * @param command_arguments_hash - hash which will contain command line arguments
	 */
	void parseCommandLineArguments ( int argc, char* argv[], QHash<QString, QString> &command_arguments_hash )
	{
		for ( int arg_num=1; arg_num<argc; arg_num++ ) {
			QString argument_str (argv[arg_num]);

			if ( argument_str.startsWith("--") ) { //it's a option name like --node
				
				QString argument_value_str = "1";

				if ( arg_num+1 < argc ) {
					argument_value_str = QString (argv[arg_num + 1]); //get next argument
				};

				if ( argument_value_str.startsWith("-") ) {
					command_arguments_hash[argument_str] = "";
				} else {
					command_arguments_hash[argument_str] = argument_value_str;
					arg_num++;
				};

			} else if ( argument_str.startsWith("-") ) {  //it's a small option name like -n or -npv (multiple options)
				//not supported yet	
				for (int j=1; j<argument_str.length(); j++) {
					command_arguments_hash[QString("-%1").arg(argument_str.data()[j])] = "";
				};
			} else { //it's a option value like node IP
				//error?
			};
		};

	};
};

// vim: set fenc=utf-8 tabstop=8 :
