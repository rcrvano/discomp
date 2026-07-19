// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2006 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef CLIENTBINDPORT_H
#define CLIENTBINDPORT_H

#include <QTcpServer>
class DiscompClient;

class DiscompClientBindPort : public QTcpServer
{
	Q_OBJECT
public:
	DiscompClientBindPort   ( QObject *parent = 0 );
	~DiscompClientBindPort  ( );

	QVector <DiscompClient*> *discomp_clients_list;
private:
	void logMessage         ( int log_level, const QString &message );

protected:
	void incomingConnection ( int socketDescriptor );
};
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
