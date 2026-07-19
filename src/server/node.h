// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+
// | Application      : Non-GUI Discomp Server                          |
// | File description : Connected Discomp Node class header             |
// | File $Id: node.h,v 1.14 2009/06/23 12:54:18 discomp Exp $           |
// +--------------------------------------------------------------------+


#ifndef DISCOMPNODE_H
#define DISCOMPNODE_H

#include <QDataStream>
#include <QTcpSocket>
#include <QtNetwork>
#include <QThread>
#include <QString>
#include <QVector>
#include <QtXml>
#include <QTime>

#include "lib/socket.h"
#include "lib/module.h"
#include "lib/module_parameter.h"
#include "lib/xml.h"
#include "lib/log.h"
#include "lib/common.h"
#include "lib/file.h"
#include "lib/xml_rrd.h"


class DiscompClips;
/*
class DiscompSocket;
class DiscompModule;
class DiscompModuleParameter;
*/

typedef struct {
	int     process_id;
	QString module_name;
	int     reason;
	QString msg;
} DiscompNodeFailedModule;

class DiscompNode : public QThread
{
	Q_OBJECT
		
public:
	DiscompNode  ( int socketDescriptor );
	~DiscompNode ( );
	
	void run  ( ); //thread

	
	QString  log_file;
	QString  addit_log_file;

	quint64  uptime_start;

	QString  ip;
	QString  name;
	QString  client_version;
	bool     authorized;
	int      id;
	bool     busy;
	bool     stoping_module_execution;
	int      process_id;
	QString  process_dir;
	QString  previous_module_name;
	int      previous_process_id;
	QString  current_module_exec;
	QVector<DiscompModule*> modules;
	int      process_list_element_num;
	int      cpu_num; //!< cpu number on the node side (it can be multiprocessor system)
	qint64   ping_msec;
	QString  sysinfo_xml;

	QVector<QString> upgrade_failed_modules;
	QString          upgrade_last_module;          
	
	QVector<DiscompNodeFailedModule> failedModules;

	void addFailedModule (int process_id, const QString &module_name, int reason, QString msg);
	void clearFailedModules ();
	int  isFailedModule  (int process_id, const QString &module_name);

	void prepareModule         ( const QString &module_name );
	void startModule           ( const QString &module_name );
	void stopModule            ( const QString &module_name );
	void clearNodeState        ( );
	
	//void disconnectFromHost  ( );
	int  getModuleNumByName    ( const QString &module_name );

	void updateModuleOnTheNode ( const QString &moduleName );

	void updateSysInfo         ( );

	void sendPingRequest ( );

	void setAdditionalLogFile  ( const QString &file_path );

	QString getMonitoringData ( const QString & interval );
private:
	bool        class_destructed;

	DiscompSocket* nodeSocket;
	int         socketDescriptor;


    	DiscompXMLRRD* m_rrd;
	
	QStringList files_for_send;
	QString     module_name_for_upgrade;

	QVector <QString> m_monitor_data;

//	DiscompClips *discomp_clips;


	//protocol functions
	void receiveAuthorizationRequest ( QDomNode xml_node );
	void receiveModulesListRequest   ( QDomNode xml_node );
	void receiveSysInfo              ( QDomNode xml_node );
	
	//module execution 
	void checkPrepareForStartModule  ( QDomNode xml_node );
	void allInputParametersSent      (                   );
	void checkInputParametersSending ( QDomNode xml_node );
	void checkModuleStarting         ( QDomNode xml_node );
	void moduleExecutionEnded        ( QDomNode xml_node );
	void checkModuleStoping          ( QDomNode xml_node );
	
	void extractFailedModuleArchive  ( const QString &module_name, const QString &filename);

	void setNodePing ( QDomNode root_node );


	void processMonitoringData ( const QDomNode & answer_node );

	void logMessage                  ( int log_level, const QString &message );

signals :
	void nodeConnected_signal                ( int node_id );
	void nodeDisconnected_signal             ( int node_id );

	void prepareModuleFailed_signal          ( int node_id, const QString &module_name, int reason, QString msg );
	void prepareModuleSuccess_signal         ( int node_id, const QString &module_name );
	
	void startModuleFailed_signal            ( int node_id, const QString &module_name, int reason, QString msg );
	void startModuleSuccess_signal           ( int node_id, const QString &module_name );

	void stopModuleSuccess_signal            ( int node_id, const QString &module_name );
	void stopModuleFailed_signal             ( int node_id, const QString &module_name, QString msg );

	void endModuleFailed_signal              ( int node_id, const QString &module_name, int status, QString msg );
	void endModuleSuccess_signal             ( int node_id, const QString &module_name );

	//for socket
	void doDisconnect_signal             ( );
	void connectToServer_signal          ( QString, QString );
	void sendDataToRemoteHost_signal     ( QString xml );
	void readDataFromRemoteHost_signal   ( );
	void sendFile_signal                 ( const QString& );
	void sendFile_signal                 ( const QStringList&, const QStringList& );
	void sendFile_signal                 ( const QString&, const QString& );
	void sendFile_signal                 ( const QString&, const QString&, const QString& );
	void clearFilesQueue_signal          ( );

	//monitoring
	void processMonitoringData_signal    ( int id, const QString &, const QString &xml_data_str );


private slots:
	void connectionClosed_slot ();

	void processRequestFromNode_slot     ( QString );
	void processAnswerFromNode_slot      ( QString );
	void inputParameterSendFileDone_slot ( QString, int, QString );

	void updateModuleArchiveUploaded_slot( QString, int, QString );

};

#endif
// vim: set fenc=utf-8 tabstop=8 :
