// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QVector>
#include <QString>
#include <QtXml>

#include "config.h"

class DiscompSocket : public QTcpSocket
{
	Q_OBJECT
public:
		
	DiscompSocket  (int sockDescr, QObject * parent = 0);
	~DiscompSocket ( );

	QString  log_file;       //!< log file to which will be saved log data
	QString  addit_log_file; //!< log file to which will be saved log data (copy)
	bool     ok;             //!< status of the socket creation

	void sendDataToRemoteHost    ( const QString& data );

	void clearFilesQueue         ( );
	int  getFileSendingQueueSize ( );
	void stopFileSending         ( );
	void setIncomingFilesDir     ( const QString &dir_path );

signals :
	
	void connectionClosed_signal             ( );
	void socketError_signal                  ( int code );
	void processRequestFromRemoteHost_signal ( const QString &xml_data );
	void processAnswerFromRemoteHost_signal  ( const QString &xml_data );

	void fileSendingProgress_signal          ( const QString &file_name, long file_bytes_done, long file_bytes_total);
	void fileReceivingProgress_signal        ( const QString &file_name, long file_bytes_done, long file_bytes_total);
	void fileSendingDone_signal              ( const QString &file_name, int  status, QString reason = "" );
	void fileReceivingDone_signal            ( const QString &file_name, int  status, QString reason = "" );

#ifdef SOCKET_CLIENT
	void connectedToServer_signal();
#endif
private slots:
	void sendDataToRemoteHost_slot       ( const QString & data );
	void readDataFromRemoteHost_slot     ( );

	void sendFile_slot                   ( const QString & );
	void sendFile_slot                   ( const QString &, const QString & );
	void sendFile_slot                   ( const QStringList &files_path, const QStringList &files_names );
	void sendFile_slot                   ( const QString &, const QString &, const QString & );
	void clearFilesQueue_slot            ( );

	void doDisconnect_slot               ( );	
#ifdef SOCKET_CLIENT
	void connectToServer_slot            ( const QString &serverIP, const QString &serverPort );
	void connectedToServer_slot          ( );
#endif

	void         closeConnection_slot    ( );
	void         socketBytesWritten_slot ( qint64 );
	virtual void socketError_slot        ( QAbstractSocket::SocketError );
private:

	QByteArray*   bytes_available;           //!< available data in the socket
	qint64        bytes_available_read;      //!< already read data size
	qint64        block_size;                //!< size of the sending/receiving block
	
	qint64        protocol_send_msg_id;      //!< counter for sending messages
	qint64        protocol_receive_msg_id;   //!< counter for receiving messages

	qint64        file_bytes_done;	         //!< count of the file bytes sent
	qint64        file_bytes_total;          //!< total bytes of the file
	bool          file_call_write;           //!< should be called sendBinaryDataToRemoteHost again
	QIODevice*    file_data_dev;             //!< file handle
	bool          file_sending_busy;         //!< file is already sending. The new file for sent should be append to the file sending queue
	QString       file_current_name;         //!< name of the current sending file
	
	QVector<QStringList> file_sending_queue; //!< file sending queue
	QString       file_incoming_dir;         //!< incoming dir for new(received) files

	
	void readDataFromRemoteHost          ( );
	
	void processDataFromRemoteHost       ( const QString &received_data                      );
	void processRequestFromRemoteHost    ( const QDomNode &xml_node, const QString &xml_data );
	void processAnswerFromRemoteHost     ( const QDomNode &xml_node, const QString &xml_data );

	void sendBinaryDataToRemoteHost      ( );
	

	void sendFile                        ( const QString &file_path );
	void sendFile                        ( const QStringList &files_path, const QStringList &files_names );
	void sendFile                        ( const QString &file_path, const QString &destination_dir );
	void sendFile                        ( const QString &file_path, const QString &destination_dir, const QString &destination_file_name );
	void sendFiles                       ( const QVector<QStringList> &files_list_for_send );

	void checkFileSendingQueue           ( );

	void prepareForSendFileToRemoteHost  ( const QString &file_path, const QString &destination_dir, const QString &destination_file_name );
	void sendFileRemoteHostReady         ( const QDomNode &xml_node );
	void receiveFileRequestFromRemoteHost( const QDomNode &xml_node );
	
	void sendFileRemoteHostDone          ( );
	void receiveFileFromRemoteHostDone   ( const QDomNode &xml_node );
	void sendFileRemoteHostDoneCheck     ( const QDomNode &xml_node );

	
	void logMessage                      ( int log_level, const QString &message );
};

#endif

// vim: set fenc=utf-8 tabstop=8 :
