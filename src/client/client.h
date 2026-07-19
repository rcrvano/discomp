// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QString>
#include <QVector>
#include <QObject>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QtXml>

#include "sysinfo.h"

class DiscompModule;
class DiscompSocket;

class DiscompClient : public QThread
{
	Q_OBJECT
public:
	 DiscompClient ( );
	~DiscompClient ( );
	
	QString serverIP;
	QString serverPort;
	QString clientName;

	QString log_file;

	void InitModules   ( );

	void setServer     ( const QString &IP, const QString &port );
	void setClientName ( const QString &name );

	void setCPUNum            ( int number );
	void setReconnectInterval ( int interval );
	void setConnectAfterStart ( bool flag );

	void doConnect     ( );
	void doDisconnect  ( );

	void sendDataToRemoteHost(const QString& xml);

	int  getConnectionStatus();

protected:
	void run           ( );

private:
	DiscompSocket*           discomp_socket;
	QVector<DiscompModule*>  modules;

	int                      cpu_num;
	int                      reconnect_interval;
	bool                     connect_after_start;
	
	QString                  process_dir_on_server;
	QStringList              files_for_send;
	QString                  current_module_exec;
	int                      connection_status;
	QString                  tmp_protocol_msg; //<! used in the function for sending filed module to server
	QTimer*                  output_synctoserver_timer;
	DiscompSysInfo*          sysinfo;
	

	//module initialization
	void loadModuleByName             ( const QString &module_name );
	void loadModulesFromDir           ( const QString &dir_path    );

	//connect to server
	void sendAuthorizationRequest     ( );
	void checkAuthorization           ( const QDomNode &data_node );
	void sendModulesListToServer      ( );
	void checkSendModulesListToServer ( const QDomNode &xml_node );
	void updateModuleFromArchive      ( const QDomNode &rootNode );
	void updateSysInfo                ( );

	//module execution
	void prepareForStartModule        ( const QDomNode &rootNode );
	void receivedInputParameters      ( const QDomNode &xml_node );
	void startModule                  ( const QString &module_name );
	//void receivedRequestOnStartModule ( QDomNode xml_node );
	void receivedRequestOnStopModule  ( const QDomNode & xml_node );
	void outputParameterIsNotExists   ( const QString &module_name, const QString &param_name );
	void allOutputParametersSent      ( );
	int  clearModuleTempEnviroment    ( const QString &module_name );
	void sendFailedModuleTempDirToServer ( const QString &module_name, const QString &protocol_msg );

	int  getModuleNumByName           ( const QString &module_name );
	void logMessage                   ( int log_level, const QString &message );

signals:
	//for parent of this
	void connectionStatusChanged      ( int );

	//for tcpsocket
	void doDisconnect_signal          ( );
	void connectToServer_signal       ( const QString &, const QString & );
	void sendDataToRemoteHost_signal  ( const QString & xml );
	void sendFile_signal              ( const QString & );
	void sendFile_signal              ( const QString &, const QString & );
	void sendFile_signal              ( const QString &, const QString &, const QString & );

private slots:
	void connectedToServer_slot                   ( );
	void connectionClosed_slot                    ( );
	void socketError_slot                         ( int code );

	void processAnswerFromServer_slot             ( const QString &xml_data_str );
	void processRequestFromServer_slot            ( const QString &xml_data_str );
	
	void syncModuleOutputParametersToServer_slot  (  );
	void sendModuleOutputParametersToServer_slot  ( const QString &module_name, int status );
	void outputParameterSendFileDone_slot         ( const QString &filename, int status, const QString &reason );
	void sendFailedModuleTempDirToServerDone_slot ( const QString &filename, int status, const QString &reason );

	void sendModulesListToServer_slot();
	void connectToServer_slot();
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
