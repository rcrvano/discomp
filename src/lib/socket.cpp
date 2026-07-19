// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include <QDataStream>
#include <QtNetwork>

#include "socket.h"
#include "xml.h"
#include "log.h"
#include "common.h"
#include "file.h"
#include "config.h"
#include "md5.h"


/**
 * @class DiscompSocket
 * @brief Discomp protocol realization
 *
 * TCP Socket class implement client-server protocol for communicate with
 * discomp nodes or server if this class used on the client side.
 *
 * The protocol messages passes in the XML format.
 * Every message contain unique \a id which should be unambiguously understand 
 * on the receiving side. If unique \a id is not known, when emited signal to 
 * parent class to process received protocol message.
 * 
 * In this class realized file sending functions, which based on the FTP 
 * file sending algorithm. All files sending by 16Kb blocks, and for sending
 * file calculated MD5 sum. When file sending complete the sending and receiving
 * side check file MD5 for equal. 
 * This class support file sending queue and you can start send more than one file.
 * When file sending will be done emited signal fileSendingDone_signal
 *
 * @par Example for client side connection
 * @code
 * 	discomp_socket = new DiscompSocket(0);
 * 	connect(discomp_socket, SIGNAL ( connectedToServer_signal() )
 * 		this, SLOT ( connectedToServer_slot  () ) );
 * 	connect(discomp_socket, SIGNAL ( connectionClosed_signal () ),
 * 		this, SLOT ( connectionClosed_slot   () ) );
 * 	connect(discomp_socket, SIGNAL ( processRequestFromRemoteHost_signal (QString) )
 * 		this, SLOT ( processRequestFromServer_slot (QString) ));
 * 	connect(discomp_socket, SIGNAL ( processAnswerFromRemoteHost_signal  (QString) ),
 * 		this, SLOT ( processAnswerFromServer_slot (QString) ));
 * 	connect(discomp_socket, SIGNAL ( fileSendingDone_signal (QString, int, QString) ),
 * 		this, SLOT ( outputParameterSendFileDone_slot (QString, int, QString) ) );
 * @endcode
 *
 * @par Example for server incoming connection
 * @code
 *	//create TCP Socket
 *	nodeSocket = new DiscompSocket(socketDescriptor);
 *	if ( !nodeSocket->ok ) {
 *		logMessage(LOG_ERROR,"TCP socket creation failed");
 *	}
 *	
 *	connect ( nodeSocket, SIGNAL ( connectionClosed_signal()                        ),
 *		this,       SLOT   ( connectionClosed_slot()                          ) );
 *	connect ( nodeSocket, SIGNAL ( processRequestFromRemoteHost_signal (QString)    ),
 *		this,       SLOT   ( processRequestFromNode_slot         (QString)    ) );
 *	connect ( nodeSocket, SIGNAL ( processAnswerFromRemoteHost_signal  (QString)    ),
 *		this,       SLOT   ( processAnswerFromNode_slot          (QString)    ) );
 *
 *	nodeSocket->log_file = QString("node_%1_%2.%3").arg(this->id).arg(this->nodeIP).arg(LOG_FILE_EXT);
 *
 * 	exec(); //thread
 *
 *      //without threads you can use this forever loop
 *	while (nodeSocket->state() == QAbstractSocket::ConnectedState) {
 *		if (nodeSocket->bytesAvailable() < 1)
 *			if (!nodeSocket->waitForReadyRead(-1))
 *				break;
 *		if ( nodeSocket->bytesAvailable() >= 1 ) {
 *			emit nodeSocket->readDataFromRemoteHost();
 *	}
 * @endcode

 *
 * \todo Need to realize encryption protocol data with OpenSSL library
 *
 */






/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/**
 * Constructor. If it's a server side and this class created for incoming connection
 * then will be set socket descriptor. \n
 * Also in constructor connected all signals of QTcpSocket, and init private variables 
 */

DiscompSocket::DiscompSocket(int socketDescriptor, QObject *parent)
	: QTcpSocket ( parent )
{
#ifdef SOCKET_SERVER
	if (!this->setSocketDescriptor(socketDescriptor)) {
		emit socketError_slot(this->error() );
		return;
	}
#endif

	socketDescriptor = 0;//for hide gcc unused warning
	
	connect( this, SIGNAL  ( readyRead()                     ),
	         this, SLOT    ( readDataFromRemoteHost_slot()   ) );
#ifdef SOCKET_CLIENT
	connect( this, SIGNAL  ( connected()                     ),
	         this, SLOT    ( connectedToServer_slot()        ) );
#endif
	connect( this,  SIGNAL ( disconnected()                  ),
	         this,  SLOT   ( closeConnection_slot()          ) );
	connect( this,  SIGNAL ( error(QAbstractSocket::SocketError)              ),
	         this,  SLOT   ( socketError_slot(QAbstractSocket::SocketError)   ) );
	connect( this,  SIGNAL ( bytesWritten(qint64)            ),
	                SLOT   ( socketBytesWritten_slot(qint64) ) );



	ok               = true;//socket created successfully
	log_file         = "";
	block_size       = 0;
	protocol_send_msg_id      = 0;
	protocol_receive_msg_id   = 0;

	
	//file sending
	file_bytes_done  = 0;
	file_bytes_total = 0;
	file_call_write  = 0;
	file_data_dev    = 0;
	file_sending_busy= 0;
	file_current_name= "";
	file_incoming_dir= Config::variables["BaseDirectory"];


	bytes_available      = new QByteArray();
	bytes_available_read = 0;
	
	logMessage ( LOG_DEBUG, "Socket constructed successfully" );
};



/** 
 * Destructor 
 */
DiscompSocket::~DiscompSocket()
{
	//if file sending now, then close it
	if ( file_data_dev ) {
		logMessage(LOG_DEBUG, "Received signal for destructing socket, but file is sending now. Close file handle");
		file_data_dev->close();
		delete file_data_dev;
		file_data_dev = 0;
	};

	delete bytes_available;

	logMessage ( LOG_DEBUG, "Socket destructed" );
};








/***********************************************************************/
/*------------------------  TCP Socket slots  -------------------------*/
/***********************************************************************/


/**
 * This slot called when socket disconnected. \n
 * You can process this event by connecting to \a connectionClosed_signal
 * \sa connectionClosed_signal
 */
void DiscompSocket::closeConnection_slot()
{
	logMessage (LOG_COMMON, "Connection closed");
	if ( file_data_dev ) {
		file_data_dev->close();
		delete file_data_dev;
		file_data_dev   = 0;
	};
	protocol_receive_msg_id = 0;

	emit connectionClosed_signal( );
};


#ifdef SOCKET_CLIENT
/**
 * This slot used only on the client side for process 
 * event when client connected to server. \n
 * You can process this event by connecting to \a connectedToServer_signal
 * \sa connectedToServer_signal
 */
void DiscompSocket::connectedToServer_slot() 
{
	logMessage( LOG_COMMON, "Connected to server!" );
	emit connectedToServer_signal();
}
#endif


/**
 * Called when some data availablein the socket
 */
void DiscompSocket::readDataFromRemoteHost_slot()
{
	this->readDataFromRemoteHost();
}


/**
 * Called when block of data written to the socket
 */
void DiscompSocket::socketBytesWritten_slot(qint64 bytes_written)
{
	logMessage( LOG_COMMUNICATIONS, QString("Socket bytes written: %1").arg(bytes_written) );
	
	if (file_call_write) {
		sendBinaryDataToRemoteHost();
	};
}


/**
 * Called when some error ocurred in the socket (connection closed, server not found
 * or connection refused). \n
 * You can process this event by connecting to \a socketError_signal
 * \sa socketError_signal
 */
		    
void DiscompSocket::socketError_slot (QAbstractSocket::SocketError code)
{
	if ( file_data_dev ) {
		file_data_dev->close();
		delete file_data_dev;
		file_data_dev   = 0;
	};

	QString message;
	
	switch (code) {
		case QAbstractSocket::HostNotFoundError:
			logMessage ( LOG_ERROR, QString("Socket error! Server not found.") );
			break;
			
		case QAbstractSocket::ConnectionRefusedError:
			logMessage ( LOG_ERROR, QString("Socket error! Connection refused.") );
			break;
		
		case QAbstractSocket::RemoteHostClosedError:
			logMessage ( LOG_WARNING, QString("Socket error! The remote host closed the connection.") );
			break;
	
		default:
			logMessage ( LOG_WARNING, QString("Socket error! Code: %1. (reason you can see here http://doc.trolltech.com/qabstractsocket.html#SocketError-enum)").arg(code) );
			break;
	}

	protocol_receive_msg_id = 0;

	emit socketError_signal( (int)code );
};








/***********************************************************************/
/*--------------------- Base protocol functions -----------------------*/
/***********************************************************************/


/**
 * Send data to remote host. It's base function for client-server communctications. 
 * @par example
 * @code
 *	//protocol request
 *	QString xml = "";
 *	xml += "<request id='120' name='prepare module for start'>\n";
 *	xml += "        <module id='" + QString("%1").arg(mod_id) + "'/>\n";
 *	xml += "</request>";
 *	nodeSocket->sendDataToRemoteHost( xml );
 *
 *	//protocol answer
 *	nodeSocket->sendDataToRemoteHost("<answer id='100' status='1' msg='success'/>");
 * @endcode
 * Don't call this function directly from server side. call it via slot
 * sendDataToRemoteHost_slot
 * @param data - data for sending to remote host in the xml format (without xml header).
 * Data can be a protocol request or protocol answer
 * */
void DiscompSocket::sendDataToRemoteHost( const QString &data )
{
	//waiting while previous block will be writed
	if ( !this->waitForBytesWritten  ( 60000 ) ) {
		//logMessage( LOG_COMMUNICATIONS, "[Send data to remote host] Error: block write failed" );
	}

	QByteArray block;

	if ( this->protocol_send_msg_id > 32768 ) {
		this->protocol_send_msg_id = 0;
	};

	//constructing XML message
	QString xml_data  = "<?xml version='1.0' encoding='UTF-8'?>\n"; 
		xml_data += "<protocol msg_id='"+QString("%1").arg(++this->protocol_send_msg_id)+"'>\n";
		xml_data += data + "\n";
		xml_data += "</protocol>";

	logMessage (LOG_COMMUNICATIONS_VERBOSE, "[Send data to remote host] Size: "+QString("%1").arg(xml_data.size()+1)+"; Data \n" + xml_data );

	block.insert(0,xml_data);

	//add request type
	block.push_front("S");

	//check block size
	if ( block.size() > 0xFFFF ) {
		logMessage ( LOG_ERROR, QString("Protocol data for sending to remote host is too long(%1b). Size should be less than 0xFFFF").arg(block.size()));
		return;
	}

	//add block size
	QByteArray block_size(4,'0');
	QByteArray tmp_byte_arr = QByteArray::number(block.size(), 16);
	block_size.replace(4-tmp_byte_arr.size(), tmp_byte_arr.size(), tmp_byte_arr );
	block.push_front( block_size );

	//write data to the socket
	this->write(block.data(), block.size());

	//wait while this block will be written to the socket
	if ( !this->waitForBytesWritten  ( 60000 ) ) {
		logMessage(LOG_COMMUNICATIONS, "[Send data to remote host] Error: waitForBytesWritten return false");
	};
};


/**
 * Send binary data(file) to remote host. 
 * File sending by blocks with size 16*1024 (how it realized in th FTP protocol)
 * file_data_dev, file_bytes_total should be defined before call this func
 * */
void DiscompSocket::sendBinaryDataToRemoteHost( )
{
	//check that file data dev is exists
	if (!file_data_dev) {
		logMessage(LOG_ERROR, "[Send binary data] Error: file_data_dev was deleted (1). Stop file sending" );
		return;
	}
	
	file_call_write = false;


	const qint64 block_size = 16*1024;
	char         buffer[16*1024];

	while ( !file_data_dev->atEnd() ) {
		while ( this->bytesToWrite() != 0 ) {
			if (!file_data_dev) {
				logMessage(LOG_ERROR, "[Send binary data] Error: file_data_dev was deleted (2). Stop File sending" );
				return;
			};
			
			if ( !this->waitForBytesWritten  ( 60000 ) ) {
				logMessage(LOG_COMMUNICATIONS, "[Send binary data] Error: waitForBytesWritten return false (1)");
			};
		};
		
		//read block from file
		qint64 read = file_data_dev->read(buffer, block_size);
		QByteArray block(buffer,read);
		
		file_bytes_done += read;

		//add type
		block.push_front("B");

		//add size of sending block
		QByteArray block_size(4,'0');
		QByteArray tmp_byte_arr = QByteArray::number(block.size(), 16);
		block_size.replace(4-tmp_byte_arr.size(), tmp_byte_arr.size(), tmp_byte_arr );
		block.push_front( block_size );

		logMessage( LOG_COMMUNICATIONS_VERBOSE, QString("[Send binary data] File sending. BlockSize:%1. FileBlockSize:%2. BytesTotal:%3. BytesDone:%4.").
		                                        arg(block.size()).arg(read).arg(file_bytes_total).arg(file_bytes_done));

		this->write( block.data(), block.size() );

		if ( !this->waitForBytesWritten  ( 60000 ) ) {
			logMessage(LOG_COMMUNICATIONS, "[Send binary data] Error: waitForBytesWritten return false (2)");
		};

		if (!file_data_dev) { // this can happen when a command is aborted
			logMessage(LOG_ERROR, "[Send binary data] Error: file_data_dev was deleted (3). Stop file sending" );
			return; 
		}

		emit fileSendingProgress_signal( file_current_name, file_bytes_done, file_bytes_total);
	}

	if (file_data_dev->atEnd()) {
		logMessage( LOG_COMMUNICATIONS, "[Send binary data]: All file blocks has been writed to the socket" );
		
		sendFileRemoteHostDone ();
		
		if (file_bytes_done == 0 && this->bytesToWrite() == 0)
			emit fileSendingProgress_signal( file_current_name, 0, file_bytes_total);
		
		file_data_dev->close();
		delete file_data_dev;
		file_data_dev   = 0;
		file_bytes_total = 0;
		file_bytes_done  = 0;
	} else {
		file_call_write = true;
	}
};


/**
 * Function for receive data from remote host.  
 * - It can be protocol data in the XML format ( request type: 'S' )
 *   Protocol data can be request or answer on the our request.
 *   After receiving data, executed processDataFromRemoteHost 
 * - Or binary data if remote host send file to us ( request type: 'B' )
 * 
 * This function called when new data available in the socket or manualy 
 * in the thread run function.
 *
 * \todo Check security problems here (memory overflow)
 * */
void DiscompSocket::readDataFromRemoteHost()
{
	//read block size 
	if (block_size == 0) {
		if (this->bytesAvailable() < 4) {
			return;
		};

		QByteArray tmp_block_size;
		tmp_block_size.resize ( 4 );
		this->read( tmp_block_size.data(), tmp_block_size.size() );
		block_size = tmp_block_size.toLongLong( NULL, 16);

		bytes_available->resize ( block_size );
		bytes_available_read = 0;
	};
	
	QByteArray tmp_ba;
	qint64 read_bytes=0;
	if ( block_size > (this->bytesAvailable() + bytes_available_read ) ) {
		//read part of data
		tmp_ba.resize(this->bytesAvailable());
		read_bytes = this->read( tmp_ba.data(), this->bytesAvailable() );
	} else {
		//read last bytes
		tmp_ba.resize(block_size - bytes_available_read);
		read_bytes = this->read( tmp_ba.data(), block_size - bytes_available_read );
	};
	bytes_available->replace( bytes_available_read, read_bytes, tmp_ba);
	bytes_available_read += read_bytes;


        if ( bytes_available_read <= 0 ) { // a read following a readyRead() signal will never fail.
		return;
	}
	logMessage(LOG_COMMUNICATIONS_VERBOSE, QString("[Read data from remote host]: Block size:%1, Read bytes: %2").
						arg(block_size).arg(bytes_available_read) );

	//check that block received completely
	if (bytes_available_read < block_size) {
		return;
	};
	
	//get request type
	char request_type = bytes_available->at(0);


	//check that request type is a protocol data or binary
	if ( request_type != 'S' && request_type != 'B' ) {
		logMessage (LOG_ERROR, QString("[Read data from remote host]. CRITICAL ERROR. Unknown request type"));
	} else {
		logMessage (LOG_COMMUNICATIONS_VERBOSE, QString("[Read data from remote host]: Request type: %1").arg(request_type) );
		bytes_available->remove(0,1);
	};

	if ( request_type == 'S' ) {//protocol data
		QString received_data(bytes_available->data());

		logMessage (LOG_COMMUNICATIONS_VERBOSE, "[Read data from remote host] Data:\n" + received_data );

		//process message
		processDataFromRemoteHost( received_data );
	

	} else if ( request_type == 'B' )  {//binary data(file)

		file_bytes_done +=  bytes_available->size();

		logMessage( LOG_COMMUNICATIONS_VERBOSE, QString("[Read binary data from remote host] File receiving. BlockSize:%1. BytesTotal:%2. BytesDone:%3.").
		                                        arg(bytes_available->size()).arg(file_bytes_total).arg(file_bytes_done) );
		if ( !file_data_dev ) { 
			logMessage(LOG_ERROR, "[Read binary data from remote host] Error: file_data_dev was deleted (3). Stop File receiving" );
		} else {
			file_data_dev->write( bytes_available->data(), bytes_available->size() );
		};
		
		emit fileReceivingProgress_signal( file_current_name, file_bytes_done, file_bytes_total);
	};
	
	bytes_available->clear();
	bytes_available->resize ( 0 );

	block_size = 0;
	readDataFromRemoteHost();
};


/**
 * Proicessing received protocol message from remote host.
 * @param received_data - protocol message
 * */
void DiscompSocket::processDataFromRemoteHost( const QString &received_data )
{
	//parse XML protocol message
	QString errorStr;
	int errorLine;
	int errorColumn;


	
	QDomDocument doc;
	if ( !doc.setContent( received_data, true, &errorStr, &errorLine, &errorColumn ) ) {
		logMessage (LOG_ERROR, QString("[Process protocol message] Error: XML processing error ( line %1, column %2: %3 ). Message skiped.").
		                               arg(errorLine).arg(errorColumn).arg(errorStr) );
		block_size = 0; 
		readDataFromRemoteHost();
		return;
	};

	QDomElement rootNode = doc.documentElement();

	if ( rootNode.toElement().tagName() != "protocol" ) {
		logMessage (LOG_ERROR, "[Process protocol message] Error: Root node is not a protocol. Message skiped");
		block_size = 0; 
		readDataFromRemoteHost();
		return;
	};

	//check recevied message id. maybe it's a dublicate xml request (last)
	qint64 message_id = rootNode.toElement().attribute("msg_id").toLongLong();
	if ( (message_id <= this->protocol_receive_msg_id) && message_id > 1 ) {
		logMessage(LOG_COMMUNICATIONS, "[Process protocol message] Found a dublicated protocol message. Message skiped");
		block_size = 0; 
		readDataFromRemoteHost();
		return;
	};
	this->protocol_receive_msg_id = message_id;
		

	QDomNode xml_node = rootNode.firstChild();
	while ( !xml_node.isNull() && xml_node.nodeType() != QDomNode::ElementNode ) {
		xml_node = xml_node.nextSibling();
	}

	if (xml_node.toElement().tagName() == "request") {
		processRequestFromRemoteHost ( xml_node, received_data );
	} else if (xml_node.toElement().tagName() == "answer") {
		processAnswerFromRemoteHost ( xml_node, received_data );
	} else {
		logMessage(LOG_ERROR, "[Process protocol message] Error: Unknown root tag in received XML:" + rootNode.toElement().tagName() );
	};
};


/**
 * Main switch where we detect remote host request.
 * It's can be a file sending request or request which 
 * should be processed in the parent class. In this case emited
 * \a processRequestFromRemoteHost_signal.
 * */
void DiscompSocket::processRequestFromRemoteHost( const QDomNode &xml_node, const QString &xml_data_str )
{
	int requestTypeId = xml_node.toElement().attribute("id").toInt();

	logMessage (LOG_COMMUNICATIONS, QString("[Processing request] Request id: %1").arg(requestTypeId) );

	switch ( requestTypeId ) {
		case 10: //remote host want to send a file 
			receiveFileRequestFromRemoteHost( xml_node );
			break;

		case 11: //remote host finished file sending
			receiveFileFromRemoteHostDone ( xml_node );
			break;
	
		default :
			logMessage (LOG_COMMUNICATIONS, "[Processing request] Unknown request type. emit signal to process this request in parent class" );
			emit processRequestFromRemoteHost_signal( xml_data_str );
			break;
	};
};


/**
 * Main switch where we detect answer from remote host.
 * It's can be answer that remote host ready to receive a file,
 * or request which should be processed in the parent class. 
 * In this case emited \a processAnswerFromRemoteHost_signal
 * */
void DiscompSocket::processAnswerFromRemoteHost( const QDomNode &xml_node, const QString &xml_data_str )
{
	int requestTypeId = xml_node.toElement().attribute("id").toInt();

	logMessage ( LOG_COMMUNICATIONS, QString("[Processing answer] Answer id: %1").arg(requestTypeId) );
	
	switch ( requestTypeId ) {
		case 10: //file transfer
			sendFileRemoteHostReady ( xml_node );
			break;

		case 11: //remote host send info about file receiving
			sendFileRemoteHostDoneCheck ( xml_node );
			break;

		default :
			logMessage (LOG_COMMUNICATIONS,  "[Processing answer] Unknown answer type. emit signal to process answer on the request in parent class" );
			emit processAnswerFromRemoteHost_signal( xml_data_str );
			break;
	};
};








/***********************************************************************/
/*--------------------- File sending protocol -------------------------*/
/***********************************************************************/


/**
 * Add file to the file sending queue
 * @param file_path - path to file which should be sent to remote host
 */
void DiscompSocket::sendFile( const QString &file_path )
{
	logMessage(LOG_DEBUG, "sendFile() 1 ");
	logMessage( LOG_COMMUNICATIONS, QString("[File queue]: Add file to the filesending queue: %1.").arg(file_path) );

	QFileInfo fileInfo(file_path);
	prepareForSendFileToRemoteHost( file_path, "", fileInfo.fileName() );
};

/**
 * Add file to the file sending queue
 * @param file_path - path to file which should be sent to remote host
 */
void DiscompSocket::sendFile( const QStringList &files_paths, const QStringList &files_names )
{
	logMessage(LOG_DEBUG, "sendFile() 0 ");
	logMessage( LOG_COMMUNICATIONS, QString("[File queue]: Add filelist to the filesending queue. List size is: ").arg(files_paths.size()) );

	for ( int i=0; i<files_paths.size(); i++ ) {
		QStringList tmp_list;
		tmp_list << files_paths.at(i) << "" << files_names.at(i) ;
		file_sending_queue.append( tmp_list );
	};
	checkFileSendingQueue();
};



/**
 * Add file to the file sending queue
 * @param file_path - path to file which should be sent to remote host
 * @param destination_dir - directory on the remote host where should be save file
 */
void DiscompSocket::sendFile( const QString &file_path, const QString &destination_dir )
{
	logMessage(LOG_DEBUG, "sendFile() 2 ");
	logMessage( LOG_COMMUNICATIONS, QString("[File queue]: Add file to the filesending queue: %1. Destination dir is: %2")
					.arg(file_path).arg(destination_dir) );

	QFileInfo fileInfo(file_path);
	prepareForSendFileToRemoteHost( file_path, destination_dir, fileInfo.fileName() );
};


/**
 * Add file to the file sending queue with different filename on the remote host
 * @param file_path - path to file which should be sent to remote host
 * @param destination_dir - directory on the remote host where should be save file
 * @param destination_file_name - file name on the remote host
 */
void DiscompSocket::sendFile( const QString &file_path, const QString &destination_dir, const QString &destination_file_name )
{
	logMessage(LOG_DEBUG, "sendFile() 3 ");

	logMessage( LOG_COMMUNICATIONS, QString("[File queue]: Add file to the filesending queue: %1. Destination dir is: %2. Destination filename: %3")
					.arg(file_path).arg(destination_dir).arg(destination_file_name) );

	prepareForSendFileToRemoteHost( file_path, destination_dir, destination_file_name );
}


/**
 * Send multilple files to the remote host
 * @par Example
 * @code
 *	QVector<QStringList> files_list_for_send;
 *	QStringList file_for_send;
 *	file_for_send << currParam->filepath << "modules/" + modules[module_num]->name << currParam->filename;
 *	files_list_for_send.append(file_for_send);
 *	nodeSocket->sendFiles(files_list_for_send);
 * @endcode
 * @param files_list_for_send - a array with files for send
 */
void DiscompSocket::sendFiles( const QVector<QStringList> &files_list_for_send )
{
	logMessage(LOG_DEBUG, "sendFiles() 4 ");
	for (int i = 0; i < files_list_for_send.size(); ++i) {
		if ( files_list_for_send.at(0).size() < 3 ) {
			logMessage(LOG_ERROR, "[File queue]: Could't add file to queue. It's not well formed");
			return;
		};
		logMessage(LOG_COMMUNICATIONS, QString("[File queue]: Add file to the filesending queue:%1. Destination dir is: %2. Destination filename: %3")
					.arg(files_list_for_send.at(i).at(0)).arg(files_list_for_send.at(i).at(1)).arg(files_list_for_send.at(i).at(2)) );
		this->file_sending_queue.append( files_list_for_send.at(i) );
	};

	checkFileSendingQueue();
}


/**
 * Clear filesending queue
 */
void DiscompSocket::clearFilesQueue()
{
	logMessage(LOG_DEBUG, "[File queue]: Clear");
	while ( file_sending_queue.size() > 0 ) {
		file_sending_queue.pop_front();
	};
}

void DiscompSocket::stopFileSending ()
{
	logMessage(LOG_DEBUG, "[File queue]: Stop file sending");
	if ( file_data_dev ) {
		file_data_dev->close();
		delete file_data_dev;
		file_data_dev = 0;
	};
};


/**
 * Check file sending queue. If it's not empty first file from it should be
 * send to remote host. THis function called after adding new file to queue
 * or when file sending done.
 * */
void DiscompSocket::checkFileSendingQueue()
{
	logMessage(LOG_COMMUNICATIONS, "[File queue]: Check file sending queue");

	if ( file_sending_busy ) {
		logMessage (LOG_COMMUNICATIONS, "[File queue]: File already sending.");
		return;
	};


	if ( file_sending_queue.size() > 0 ) {
		logMessage(LOG_COMMUNICATIONS, "[File queue]: Found task for sending file to remote host");
		if ( file_sending_queue.at(0).size() < 3 ) {
			logMessage(LOG_ERROR, "[File queue]: Error: queue is corrupted");
			file_sending_queue.clear();
			return;
		};
		QString file_path            = file_sending_queue.at(0).at(0);
		QString destination_dir      = file_sending_queue.at(0).at(1);
		QString destination_filename = file_sending_queue.at(0).at(2);
		
		file_sending_queue.pop_front();
		prepareForSendFileToRemoteHost ( file_path, destination_dir, destination_filename );
	} else {
		logMessage (LOG_COMMUNICATIONS, "[File queue]: File sending queue is empty");
	};
};


/**
 * Return true if file sending queue is empty
 */
int DiscompSocket::getFileSendingQueueSize ( )
{
	return file_sending_queue.size();

}


/**
 * Function for prepare file sending to remote host. \n
 * (sending-server side)
 * @param file_path - path to file on local mashine (which should be sent)
 * @param destination_dir - directory on the remote host (where this file should be saved)
 *   this parameter should have only relative path from dir where client is installed.
 * @param destination_file_name - file name on the remote host
 * */
void DiscompSocket::prepareForSendFileToRemoteHost( const QString &file_path, const QString &destination_dir, const QString &destination_file_name )
{
	logMessage(LOG_DEBUG, "prepareForSendFileToRemoteHost () 5");
	//First of all we open file handle. get file info and send request 
	//to remote host that "we want to send a file to you". 
	
	//if file already sending add it to queue, and exit
	if ( file_sending_busy ) {
		logMessage(LOG_COMMUNICATIONS, "[File sending] File already sending. add this request to queue");
		QStringList tmp_list;
		tmp_list << file_path << destination_dir << destination_file_name;
		file_sending_queue.append( tmp_list );
		return;
	};

	logMessage( LOG_COMMUNICATIONS, QString("[File sending]: Preparing file sending: %1. Destination dir is: %2. Destination filename: %3")
					.arg(file_path).arg(destination_dir).arg(destination_file_name) );
	
	file_data_dev = new QFile(file_path);

	if ( !file_data_dev->open(QIODevice::ReadOnly) ) {
		logMessage(LOG_ERROR, "[File sending]: Could't open file for reading: " +  file_path);
		file_sending_busy = false;
		file_current_name = "";
		delete file_data_dev;
		file_data_dev = 0;
		checkFileSendingQueue();
		return;
	};

	if (file_data_dev->isSequential()) {
		file_bytes_total = 0;
	} else {
		file_bytes_total = file_data_dev->size();
	};
	file_bytes_done = 0;
	
	logMessage(LOG_COMMUNICATIONS, QString("[File sending] File size: %1b").arg(file_bytes_total) );

	file_sending_busy = true;
	file_current_name = file_path;

	//send to remote host request for sending file
        QString xml = "";
	xml += "<request id='10' name='file_sending' msg='file sending request'>\n";
	xml += "        <file name='" + destination_file_name + "' dir='"+destination_dir+"' size='" + QString("%1").arg(file_bytes_total) + "'/>\n";
	xml += "</request>";

	sendDataToRemoteHost( xml );
};


/** 
 * Process file sending request.
 * Here we receive a file info which has been sent from prepareForSendFileToRemoteHost,
 * check all and prepare enviroment for receiving new file.\n
 * (sending-client side)
 **/
void DiscompSocket::receiveFileRequestFromRemoteHost( const QDomNode &xml_node )
{
	logMessage(LOG_DEBUG, "receiveFileRequestFromRemoteHost () 6");
	//On the client side we open file_data_dev for file writing, 
	//and if all is ok we send to remote host
	//answer that we are ready for receive file data
	
	//init file_data_dev
	QDomNode fileNode = XML::getChildNodeByTag( "file", xml_node );
	QString fileName  = fileNode.toElement().attribute("name");
	QString filePath  = fileNode.toElement().attribute("dir");
	file_bytes_total  = fileNode.toElement().attribute("size").toLongLong();
	file_bytes_done   = 0;
	file_sending_busy = true; //now we cannot send a file to remote host, while receving not completed

	file_current_name = filePath + QDir::separator() + fileName;

	if ( file_current_name.contains(QRegExp("^/")) ) {
		logMessage ( LOG_WARNING, "File path contains absolute dir. Trying to remove it" );
		file_current_name = file_current_name.replace(this->file_incoming_dir, "" );
	};

	//security checks
	if ( !File::checkFileName (file_current_name) ) {
		logMessage(LOG_ERROR, QString("[File receiving] Error: File name contained forbidden characters: %1").arg(file_current_name));
		sendDataToRemoteHost( "<answer id='10' status='0' name='file_sending' msg='File or dir contain forbidden characters'/>" );
		delete file_data_dev;
		file_data_dev    = 0;
		file_bytes_total = 0;
		file_sending_busy= false;
		emit fileReceivingDone_signal ( file_current_name, false, "File or dir contain forbidden characters" );
		checkFileSendingQueue();
		return;
	};

	logMessage(LOG_COMMUNICATIONS, QString("[File receiving]: Remote host sending file to us: %1").
					arg( this->file_incoming_dir + QDir::separator() + file_current_name) );

	file_data_dev = new QFile( this->file_incoming_dir + QDir::separator() + file_current_name );
	if ( !file_data_dev->open(QIODevice::WriteOnly) ) {
		logMessage(LOG_ERROR, "[File receiving] Error: Could't open file for write: " + file_current_name);
		sendDataToRemoteHost( "<answer id='10' status='0' name='file_sending' msg='Could not open file for writing : "+(this->file_incoming_dir + QDir::separator() + file_current_name)+"'/>" );
		delete file_data_dev;
		file_data_dev    = 0;
		file_bytes_total = 0;
		file_sending_busy= false;
		emit fileReceivingDone_signal ( file_current_name, false, "Could not open file for writing: " + this->file_incoming_dir + QDir::separator() + file_current_name  );
		checkFileSendingQueue();
	} else {
		sendDataToRemoteHost( "<answer id='10' status='1' name='file_sending' msg='ready for receive file data'/>" );
	};
};


/**
 * Process answer from remote host. If he is ready we 
 * start sending file data to remote host\n
 * (sending-server side)
 * */
void DiscompSocket::sendFileRemoteHostReady( const QDomNode &xml_node ) 
{
	logMessage(LOG_DEBUG, "sendFileRemoteHostReady () 7");

	if ( xml_node.toElement().attribute("status").toInt() != 1 ) {
		logMessage(LOG_ERROR, QString("[File sending] Error: Could't send file: %1 to temote host. Reason: %2").
					arg(file_current_name).arg(xml_node.toElement().attribute("msg")) );
		delete file_data_dev;
		file_data_dev     = 0;
		file_bytes_done   = 0;
		file_bytes_total  = 0;
		file_sending_busy = false;
		emit fileSendingDone_signal ( file_current_name, false, xml_node.toElement().attribute("msg") );
		checkFileSendingQueue();
		return;
	};

	sendBinaryDataToRemoteHost();
};


/**
 * All file blocks sent to remote host.
 * Now sent request for checking MD5 file sum\n
 * (sending-server side)
 * */
void DiscompSocket::sendFileRemoteHostDone ()
{
	logMessage(LOG_DEBUG, "sendFileRemoteHostDone () 8");
	sendDataToRemoteHost( QString("<request id='11' name='file_sending_check' md5='%1' msg='file sending done'/>\n").arg(File::getMD5(file_current_name)) );
};


/**
 * Processing request for checking MD5 file sum\n
 * (sending-client side)
 * */
void DiscompSocket::receiveFileFromRemoteHostDone ( const QDomNode &xml_node )
{
	logMessage(LOG_DEBUG, "receiveFileFromRemoteHostDone () 9");

	if ( file_data_dev ) {
		file_data_dev->close();
	};
	delete file_data_dev;
	file_data_dev    = 0;
	file_bytes_done  = 0;
	file_bytes_total = 0;
	file_sending_busy= false;

	//TODO!!!!!!!!!!!! in this case cliet can write to every dir on server!!!. need to check and block this possibility
	//maybe allow to write only in the specific directoried ???
	//
	QString file_md5 = File::getMD5(this->file_incoming_dir + QDir::separator() + file_current_name);

	if ( xml_node.toElement().attribute("md5") != QString(file_md5) ) {
		QFile::remove(this->file_incoming_dir + QDir::separator() + file_current_name);
		logMessage(LOG_ERROR, QString("[File receiving] Error: MD5 sum is differ. File: %1, remote MD5:%2, my MD5:%3")
					.arg(this->file_incoming_dir + QDir::separator() + file_current_name)
					.arg(xml_node.toElement().attribute("md5")).arg(file_md5) );

		sendDataToRemoteHost( "<answer id='11' status='0' name='file_sending_check' msg='md5 file sums is differ'/>\n" );

		emit fileReceivingDone_signal ( file_current_name, false, "md5 file sums is differ" );
	} else {
		logMessage(LOG_COMMON, QString("Received new file: %1").arg(this->file_incoming_dir + QDir::separator() + file_current_name) );

		sendDataToRemoteHost( "<answer id='11' status='1' name='file_sending_check' msg='file sending done success'/>\n" );
		
		//emmit signal that done
		emit fileReceivingDone_signal ( file_current_name, true, "" );
	}
	
	file_current_name = "";
	
	checkFileSendingQueue();
}


/**
 * Checking answer from sedning-client about MD5 file checking.\n
 * (sending-server side)
 * */
void DiscompSocket::sendFileRemoteHostDoneCheck ( const QDomNode &xml_node )
{
	logMessage(LOG_DEBUG, "sendFileRemoteHostDoneCheck () 10");

	if ( file_data_dev ) {
		file_data_dev->close();
		delete file_data_dev;
	};
	file_data_dev    = 0;
	file_bytes_done  = 0;
	file_bytes_total = 0;
	file_sending_busy= false;

	if ( xml_node.toElement().attribute("status").toInt() != 1 ) {
		logMessage(LOG_ERROR, "[File sending] Failed. Reason:" + xml_node.toElement().attribute("msg") );
		emit fileSendingDone_signal ( file_current_name, false, xml_node.toElement().attribute("msg") );
		return;
	};

	logMessage (LOG_COMMON, QString("[File sending] Complete. File:%1").arg(file_current_name) );

	emit fileSendingDone_signal ( file_current_name, true, "" );
	file_current_name = "";

	checkFileSendingQueue();
}






/***********************************************************************/
/*------------------------ Public slots -------------------------------*/
/***********************************************************************/
#ifdef SOCKET_CLIENT
/**
 * Connect to remote server
 * @param serverIP - IP of the remote server
 * @param serverPort - Port on the remote server
 */
void DiscompSocket::connectToServer_slot( const QString &serverIP, const QString &serverPort )
{
	logMessage ( LOG_COMMON, QString("Trying to connect to server:%1 port:%2").arg(serverIP).arg(serverPort) );
	
	this->connectToHost( serverIP, serverPort.toInt() );

	block_size = 0;
}
#endif

/**
 * Close socket connection
 */
void DiscompSocket::doDisconnect_slot( )
{
	this->close();
};

/**
 * Set directory for incoming files
 */
void DiscompSocket::setIncomingFilesDir ( const QString &dir_path )
{
	logMessage ( LOG_DEBUG, "Files incoming dir set to :" + dir_path );
	this->file_incoming_dir = QString(dir_path);
}

/***********************************************************************/
/*------------------------ Slots for public use -----------------------*/
/***********************************************************************/
/**
 * Send data to remote host (slot function)
 */
void DiscompSocket::sendDataToRemoteHost_slot (const QString &data ) {
	this->sendDataToRemoteHost (data);
};
void DiscompSocket::sendFile_slot  ( const QString &file_path ) {
	this->sendFile ( file_path );
};
void DiscompSocket::sendFile_slot  ( const QStringList &file_path, const QStringList& files_names ) {
	this->sendFile ( file_path, files_names);
};

void DiscompSocket::sendFile_slot  ( const QString &file_path, const QString &destination_dir ) {
	this->sendFile ( file_path, destination_dir );
};
void DiscompSocket::sendFile_slot  ( const QString &file_path, const QString &destination_dir, const QString &destination_file_name ) {
	this->sendFile ( file_path, destination_dir, destination_file_name );
};
void DiscompSocket::clearFilesQueue_slot () { 
	this->clearFilesQueue(); 
};




/***********************************************************************/
/*------------------------- Private methods ---------------------------*/
/***********************************************************************/
/**
 * Add log message
 * @param log_level - log level from log.h
 * @param message - log message
 */
void DiscompSocket::logMessage ( int log_level, const QString &message ) 
{
	if ( this->log_file != "" ) {
		Log::addMessage ( this->log_file, log_level, "DiscompSocket", message);
	} else {
		Log::addMessage ( log_level, "DiscompSocket", message);
	};

	if ( this->addit_log_file != "" ) {
		Log::addMessage ( this->addit_log_file, log_level, "DiscompSocket", message);
	};
};


// vim: set fenc=utf-8 tabstop=8 :
