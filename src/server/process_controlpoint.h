// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef PROCESSCONTROLPOINT_H
#define PROCESSCONTROLPOINT_H

#include <QString>
#include <QObject>

class DiscompProcess;

class DiscompProcessControlPoint : public QObject
{
	Q_OBJECT
		
public:
	DiscompProcessControlPoint  (QObject *parent = 0);
	~DiscompProcessControlPoint ();
	
	DiscompProcess* discomp_process_ptr;  //!< pointer to the parent DiscompProcess class
	QString         log_file;             //!< to which log file should be writed log messages
	QString         controlpoint_file;    //!< absoulte path to the controlpoints file
	QString         process_plan_file;    //!< absolute path to the process.xml file
	QString         process_plan_md5_sum; //!< MD5 file sum for process.xml

	int isExists();
	int Start();
	int Delete();
	int Resume();
	int Create();

private:
	void resetModulesState ( );
	void logMessage        ( int log_level, const QString &message );
};
 
#endif

// vim: set fenc=utf-8 tabstop=8 :
