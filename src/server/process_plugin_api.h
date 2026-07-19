// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef PROCESSPLUGINAPI_H
#define PROCESSPLUGINAPI_H

#include <QObject>
#include <QVector>

//class DiscompModuleParameter;
class DiscompProcess;
class DiscompProcessPluginAPI : public QObject
{
	Q_OBJECT
public:
	DiscompProcessPluginAPI  (DiscompProcess *parent = 0);
	~DiscompProcessPluginAPI ( );


public slots:
	// DiscompAPI methods
	void     stopModule          ( const QString &module_name );
	void     stopListModule      ( const QString &module_name, int list_num );

	void     restartModule       ( const QString &module_name );
	void     restartListModule   ( const QString &module_name, int list_num );

	QString  getFileParameterValue ( const QString &parameter_name );
	bool     setFileParameterValue ( const QString &parameter_name, const QString & value ); 
	int      getListParameterSize  ( const QString &parameter_name );
	QString  getListParameterValue ( const QString &parameter_name, int list_num );
	bool     setListParameterValue ( const QString &parameter_name, int list_num, const QString & value ); 

	bool     gotoStage ( int stage_num );

	// Access to the some variables
	int      getCurrentStage ( );
	QString  getProcessDir ( );
	
	void     logMessage          ( const QString &message );

private:
	DiscompProcess* discomp_process; //<! pointer on the DiscompProcess 

	void logMessage ( int log_level, const QString& message );
};

/*
// Wrapper for QSA interpreter
class DiscompProcessPluginAPIWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY( int      currentStageNum   READ  getCurrentStage )
    Q_PROPERTY( QString  processDirectory  READ  getProcessDir   )

public:
	DiscompProcessPluginAPIWrapper(DiscompProcessPluginAPI *ptr) : discomp_api(ptr) { }
	
	int      getCurrentStage ( )                   { return discomp_api->getCurrentStage(); }
	QString  getProcessDir   ( )                   { return discomp_api->getProcessDir(); }
public slots:

	void     stopModule        ( const QString &module_name )               { discomp_api->stopModule(module_id); }
	void     stopListModule    ( const QString &module_name, int list_num ) { discomp_api->stopListModule(module_id, list_num); };

	void     restartModule     ( const QString &module_name )               { discomp_api->restartModule(module_id); }
	void     restartListModule ( const QString &module_name, int list_num ) { discomp_api->restartListModule(module_id, list_num); }

	void     logMessage      ( const QString &msg ) { discomp_api->logMessage(msg); }

private:
	DiscompProcessPluginAPI *discomp_api;
};
*/

#endif
// vim: set fenc=utf-8 tabstop=8 :
