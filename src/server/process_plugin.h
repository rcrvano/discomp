// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef PROCESSPLUGIN_H
#define PROCESSPLUGIN_H

#include <QtXml>
#include <QtScript>
//#include <qsinterpreter.h>
//#include <qswrapperfactory.h>

#include "process_plugin_api.h"

class DiscompProcess;

class DiscompProcessPlugin : public QObject
{
	Q_OBJECT
public:
	DiscompProcessPlugin  (DiscompProcess *parent = 0);
	~DiscompProcessPlugin ( );

	int Init ( const QDomNode &pluginNode );

	// plugin calls
	void processModuleEvent      ( int stage_num, int module_num, const QString& event, int reason=0 ); 
	void processStageEvent       ( int stage_num,                 const QString& event, int reason=0 );
	void setDiscompAPIEnviroment ( int stage_num, int module_num, const QString& event, int reason=0 );

private:
	QScriptEngine* script_engine;         //<! ECMA Script Engine

	bool evaluateScript( const QString & script );

	DiscompProcessPluginAPI* discomp_api; //<! DiscompAPI for use in the qsa scripts

	DiscompProcess *discomp_process;      //<! pointer on the DiscompProcess of this plugin

	bool initialized;                     //<! qsa interpreter initalized correctly or not
	
	void logMessage ( int log_level, const QString &message );

//public slots:
//	void error( const QString &msg, const QString &, int line );

};


// Wrappers for QSA Interperter
/*
class DiscompProcessPluginWrappers : public QSWrapperFactory
{
public:
	DiscompProcessPluginWrappers() {
		// Tell the factory what which classes we provide wrappers for.
		registerWrapper("DiscompProcessPluginAPI");
	}
	QObject *create(const QString &name, void *ptr) {
		if (name == "DiscompProcessPluginAPI")
			return new DiscompProcessPluginAPIWrapper((DiscompProcessPluginAPI *)ptr);
		return 0;
	}
};
*/
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
