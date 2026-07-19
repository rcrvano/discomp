// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef SERVERCOMMUNICATOR_H
#define SERVERCOMMUNICATOR_H

#include <QTcpServer>

#include "server.h"

class DiscompServerRPC : public QTcpServer
{
	Q_OBJECT
    
public:
	 DiscompServerRPC ( QObject *parent = 0 );
	~DiscompServerRPC ( );

	void setDiscompServer           ( DiscompServer* );
private:
	DiscompServer* discomp_server_ptr; //!< pointer to the current DiscompServer class. for collect information

	void logMessage            ( int log_level, const QString &message );

protected:
	void incomingConnection    ( int socketDescriptor );
};
 
#endif
// vim: set fenc=utf-8 tabstop=8 :
