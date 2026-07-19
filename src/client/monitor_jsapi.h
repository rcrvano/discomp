// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef DISCOMPMONITORJSAPI_H
#define DISCOMPMONITORJSAPI_H

#include <QObject>
#include <QVector>

//class DiscompModuleParameter;
class DiscompMonitorJSAPI : public QObject
{
	Q_OBJECT
public:
	DiscompMonitorJSAPI  ( QObject *parent = 0);
	~DiscompMonitorJSAPI ( );


public slots:
	// DiscompAPI methods
	
	QString startProcess        ( const QString &cmd );

	void logMessage          ( const QString &message );

private:
	
	void logMessage ( int log_level, const QString& message );
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
