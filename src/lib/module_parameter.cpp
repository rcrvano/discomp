// +--------------------------------------------------------------------+
// | DISCOMP : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2009 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+



#include <QFile> 
#include <QDir> 
 
#ifdef Q_WS_WIN
	#include <windows.h>
#endif

#include "config.h" 
#include "xml.h" 
#include "log.h" 
#include "common.h" 
#include "file.h" 
#include "module_parameter.h"


/**
 * @class DiscompModuleParameter
 * @brief Discomp module parameters
 *
 * In this class realized all parameters types which used in 
 * the discomp process (file, filelist) and methods for works with 
 * this paramters (clear parameter from disk, get filelist size, etc.)
 *
 * @todo support of scalar & filescalar types
 */

/**
 * Constructor
 */
DiscompModuleParameter::DiscompModuleParameter(QObject *parent)
	: QObject(parent)
{
	filepath  = "";
	dir       = "";
	value     = "";
	type      = "";
	input     = false;
	list_size = -1;
	constant  = 0;
	md5       = "";
	constant_tmp_path = "";
};

/**
 * Destructor
 */
DiscompModuleParameter::~DiscompModuleParameter()
{
};


/**
 * Init parameter by xml defenition.
 * In example below two types of parameters.
 * @code 
 * Filelist type:
 * <param>
 *	<name>konus.out</name>
 *	<type>filelist</type>
 *	<filepattern>konus_%1.out</filepattern>
 * 	<comment>Result of konus calculating</comment>
 * </param>
 * File type:
 * <param>
 * 	<name>konus.in.descr</name>
 * 	<type>file</type>
 * 	<filename>konus.in.descr</filename>
 * 	<comment>Template of input file for konus</comment>
 * </param>
 * @endcode
 *
 * @param paramNode - the QDomNode which pointed to the &lt;param&gt; node
 * @return status of initiailization (true or false)
 */
int DiscompModuleParameter::Init ( const QDomNode &paramNode )
{
	this->name     = XML::getTextNodeByTag("name",     paramNode);
	this->type     = XML::getTextNodeByTag("type",     paramNode);
	this->comment  = XML::getTextNodeByTag("comment",  paramNode);
	this->constant = XML::getTextNodeByTag("constant", paramNode).toInt();

	if ( this->name == "" ) {
		emit logMessage_signal(LOG_ERROR, "    Parameter name is not specified");
		return false;
	};


	//file type
	this->filename = XML::getTextNodeByTag("filename" ,    paramNode);

	//filelist type
	this->filepattern = XML::getTextNodeByTag("filepattern",  paramNode);
	this->remote_filename = ""; //for element of list

	if ( this->type == "file" && this->filename == "" ) {
		emit logMessage_signal(LOG_ERROR, "    You should specify <filename> for parameter with type 'file'");
		return false;
	};
	if ( this->type == "filelist" && this->filepattern == "" ) {
		emit logMessage_signal(LOG_ERROR, "    You should specify <filepattern> for parameter with type 'filelist' (for ex. <filepattern>input%1</filepattern>)");
		return false;
	};



	/*
	//scalar and filescalar types. not supported yet
	if ( this->type == "scalar" ) {
		this->value = XML::getTextNodeByTag("value" ,    paramNode);
	} else if ( this->type == "filescalar" ) {
		//TODO: read file content to the value variable
	};
	*/

	emit logMessage_signal( LOG_COMMON, QString("    Parameter (name:%1)").arg( this->name ) );
	return true;
}


/**
 * Check defined parameter.
 * If parameter type is file, then checked if this file exists or not
 * if type is filelist, then checked filelist size, and it should be more than 0.
 * */
int DiscompModuleParameter::is_defined( bool hard )
{
	if ( this->type == "file" || this->type == "filescalar" ) {

		if ( QFile::exists( this->dir + QDir::separator() + this->filename) ) {
			return 1;
		} else {
			return 0;
		}
		
	} else if ( this->type == "scalar" ) {
		return ( this->value != "" );

	} else if ( this->type == "filelist" ) {
		return ( this->listsize(hard) > 0 ) ? true : false;
	}

	return false;
};

qint64 DiscompModuleParameter::filesize()
{
	if ( this->type == "file" ) {
		QFile file (this->dir + QDir::separator() + this->filename);
		quint64 filesize = file.size();
		return filesize;
	};
	return 0;
};

/**
 * Clear parameter.
 * If it's a file or filelist then file will be removed.
 * */
void DiscompModuleParameter::clear()
{
	if ( this->type == "file" || this->type == "filescalar" ) {
		QFile::remove( this->dir + QDir::separator() + this->filename );

	} else if ( this->type == "filescalar" ) {
		this->value = "";
		QFile::remove( this->dir + QDir::separator() + this->filename );

	} else if ( this->type == "scalar" ) {
		this->value = "";
		
	} else if ( this->type == "filelist" ) {

		//open dir
		QDir files_dir(this->dir);
		//create filter (like: file[0-9]*)
		QStringList files_filter;
		files_filter << QString(this->filepattern).arg("[0-9]*");
		//get files list
		QStringList files_list = files_dir.entryList( files_filter, QDir::Files );

		for (int i = 0; i < files_list.size(); i++) {
			QFile::remove( this->dir + QDir::separator() + files_list.at(i) );
		};
	};
}


/**
 * Touch parameter file
 */
void DiscompModuleParameter::touch()
{
	if ( !File::touch ( this->dir + QDir::separator() + this->filename ) ) {
		emit logMessage_signal (LOG_WARNING, "Couldn't touch file: " + this->dir + QDir::separator() + this->filename);
	};
}

/**
 * Get list size for param tipe:filelist
 * The numeration of the files shuld start from 1
 * @param hard - completely rebuild list size value
 * */
int DiscompModuleParameter::listsize( bool hard ) 
{
	if ( this->type != "filelist" ) {
		return 0;
	};

	if ( !hard && this->list_size != -1 && this->list_size != 0 ) {
		return this->list_size;
	};
	
	if ( this->dir == "" ) {
		return 0;
	};

	//open dir
	QDir files_dir(this->dir);

	//create filter (like: file[0-9]*)
	QStringList files_filter;
	files_filter << QString(this->filepattern).arg("[0-9]*");

	//get files list
	QStringList files_list = files_dir.entryList( files_filter, QDir::Files );
	this->list_size = files_list.size();

	return this->list_size;
};



/**
 * Get parameter value for type: file 
 */
QString DiscompModuleParameter::getFileTypeValue(int blocked)
{
	if ( this->type == "file" || this->type == "filescalar" ) {
		QString data;
		while ( !QFile::exists ( this->dir + QDir::separator() + this->filename ) ) {
		    if ( !blocked ){
			return "";
		    }
                    emit logMessage_signal(LOG_COMMON, "getFileTypeValue: waiting");
#ifdef Q_WS_WIN
                    Sleep(1000);
#else
                    usleep(1000);
#endif
		}
		File::read ( this->dir + QDir::separator() + this->filename, data );
		return data;
	};
	return "";
};

/**
 * Set parameter value for type: file 
 */
bool DiscompModuleParameter::setFileTypeValue( const QString & param_value )
{
	if ( this->type == "file" || this->type == "filescalar" ) {
		return File::save ( this->dir + QDir::separator() + this->filename, param_value );
	};
	return 0;
};



/**
 * Get parameter value for type: filelist
 */
QString DiscompModuleParameter::getListTypeValue( int list_num, int blocked )
{
	if ( this->type == "filelist" ) {
		QString file_path = this->dir + QDir::separator() + QString(this->filepattern).arg(list_num);
		while ( !QFile::exists ( file_path ) ) {
		    if ( !blocked ){
			return "";
		    }
                    emit logMessage_signal(LOG_COMMON, "getListTypeValue: waiting");
#ifdef Q_WS_WIN
                    Sleep(1000);
#else
                    usleep(1000);
#endif
		}
		QString data;
		File::read ( file_path, data );
		return data;
	};
	return "";
};

/**
 * Set parameter value for type: filelist
 */
bool DiscompModuleParameter::setListTypeValue( int list_num, const QString &param_value )
{
	if ( this->type == "filelist" ) {
		QString file_path = this->dir + QDir::separator() + QString(this->filepattern).arg(list_num);
//		if ( QFile::exists ( file_path ) ) {
			return File::save ( file_path, param_value );
//		} else {
//			emit logMessage ( LOG_WARNING, "Couldn't set parameter value. Reason file not exists: " +  file_path );
//		};
	};
	return 0;
};



// vim: set fenc=utf-8 tabstop=8 :
