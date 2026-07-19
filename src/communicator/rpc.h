// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <QString>
#include <QVector>
#include <QObject>
#include <QTcpSocket>
#include <QtXml>

class DiscompSocket;

class DiscompCommunicator : public QObject
{
	Q_OBJECT
public:
	DiscompCommunicator               ( QObject *parent = 0 );
	~DiscompCommunicator              ( );
	
	void showHelpUsage                ( );
	void parseCommandLine             ( int argc, char *argv[] );

	void doConnect                    ( const QString &serverIP, const QString &serverPort );
	void doDisconnect                 ( );

private:
	DiscompSocket *discomp_socket;
	QString        xml_command_for_send;
	
	//processing functions
	void showNodesList                ( const QDomNode &xml_node );
	void showNodeInfo                 ( const QDomNode &xml_node );
	void showProcessInfo              ( const QDomNode &xml_node );
	void showProcessesList            ( const QDomNode &xml_node );

	//some private functions
	void printFormatTable             ( QVector<QString> &columns, QVector <QStringList> &rows );
	void logMessage                   ( int log_level, const QString &message );

signals:
	void doDisconnect_signal          ( );
	void connectToServer_signal       ( const QString &, const QString & );
	void sendDataToRemoteHost_signal  ( const QString &xml );

private slots:
	void connectedToServer_slot       ( );
	void connectionClosed_slot        ( );
	void socketError_slot             ( int );

	void processAnswerFromServer_slot ( const QString &xml_data_str );
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
