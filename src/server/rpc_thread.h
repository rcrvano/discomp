// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef SERVERCOMMUNICATORTHREAD_H
#define SERVERCOMMUNICATORTHREAD_H

#include <QThread>
#include <QString>
#include <QtXml>

class DiscompSocket;
class DiscompServer;

class DiscompServerRPCThread : public QThread
{
	Q_OBJECT
		
public:
	DiscompServerRPCThread  ( int socket_descriptor );
	~DiscompServerRPCThread ( );
	void setDiscompServer            ( DiscompServer * );
	
	void run ( ); //thread


private:
	DiscompServer* discomp_server_ptr; //!< pointer to the DiscompServer class
	DiscompSocket* host_socket;        //!< host socket (DiscompSocket)
	int            socket_descriptor;  //!< descriptor of incoming connection

	QString        login;
	QString        user_type;
	
	//works with nodes
	void connectNode           ( const QDomNode & xml_node );
	void disconnectNode        ( const QDomNode & xml_node );
	void getNodeInfo           ( const QDomNode & xml_node );
	void getConnectedNodesList (                           );
	void installModuleOnNode   ( const QDomNode & xml_node );
	void updateNodeSysInfo     ( const QDomNode & xml_node );
	void deleteFailedModule    ( const QDomNode &xml_node );

	//works with processes
	void startProcess          ( const QDomNode & xml_node );
	void startSingleProcess    ( const QDomNode & xml_node );
	void stopProcess           ( const QDomNode & xml_node );
	void getProcessInfo        ( const QDomNode & xml_node );
	void getProcessesList      (                           );
	void clearFailedModulesForNodes ( );

	//works with users
	void getUsersList          (  );
	void createNewUser         ( const QDomNode & xml_node );
	void changeUserPassword    ( const QDomNode &xml_node, const QString &login );
	
	void logMessage            ( int log_level, const QString & message );

	//works with monitoring

	void getNodeMonitoringData( const QDomNode &xml_node );

signals :
	//for socket
	void doDisconnect_signal           ( );
	void sendDataToRemoteHost_signal   ( const QString & xml );

private slots:
	void connectionClosed_slot         ( );
	void processRequestFromHost_slot   ( const QString & );
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
