// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QVector>
#include <QString>

#include "node.h"
#include "process_private.h"
#include "process.h"
#include "process_js.h"
#include "lib/xml.h"
#include "monitor.h"

class DiscompServer : public QTcpServer
{
	Q_OBJECT
    
public:
	DiscompServer(QObject *parent = 0);
	~DiscompServer();
    
	QVector <DiscompProcessPrivate*> processes;  //<! array with all processes
	QVector <DiscompNode*>    nodes;      //<! array with all connected nodes


	int  startProcess              ( const QString& process_name, const QString &user_name );
	int  startSingleProcess        ( const QString& process_dir,  const QString &user_name );
	void stopProcess               ( int            process_id   );
	void stopProcess               ( const QString& process_name );

	void connectNode               ( const QString& node_ip, int node_port );
	void disconnectNode            ( int            node_id                );

	int  getProcessIdByName        ( const QString& process_name );
	int  getProcessNumById         ( int            process_id   );

	void clearFailedModulesForNodes( );

private:
	int  process_counter;

	void checkProcessQueue         ( );

	void logMessage                ( int log_level, const QString& message );
	bool all_nodes_free;

	DiscompMonitor *m_discomp_monitor; //<!  object for receive monitoring data from nodes

protected:
	void incomingConnection        ( int socket_descriptor );

signals:
	void nodeConnected_signal      ( int node_id );
	void nodeDisconnected_signal   ( int node_id, const QString& name, const QString& ip );

public slots:
	void nodeConnected_slot        ( int node_id );
	void nodeDisconnected_slot     ( int node_id );

	void processFinished_slot      ( int process_id );
	void processFreeNode_slot      ( int process_id, int node_id, const QString &module_name );

	void prepareModuleSuccess_slot ( int node_id, const QString &module_name );
	void prepareModuleFailed_slot  ( int node_id, const QString &module_name, int reason, const QString& msg );
	
	void startModuleSuccess_slot   ( int node_id, const QString &module_name );
	void startModuleFailed_slot    ( int node_id, const QString &module_name, int reason, const QString& msg );

	void endModuleSuccess_slot     ( int node_id, const QString &module_name );
	void endModuleFailed_slot      ( int node_id, const QString &module_name, int status, const QString& msg );

	void stopModuleSuccess_slot    ( int node_id, const QString &module_name );
	void stopModuleFailed_slot     ( int node_id, const QString &module_name, const QString& msg );

	void stopModuleOnNode_slot ( int node_id, const QString &module_name );
	void newModulesAvailable_slot ();
};
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
