// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include <QTextStream>
#include <QByteArray>
#include <QVector>
#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QMap>
#include <iostream>


#include "file.h"
#include "common.h"
#include "config.h"
#include "log.h"
#include "xml.h"
#include "md5.h"

QMutex  file_mutex;
/**
 * @brief Files manipulations
 *
 * This namespace contain functions for works with files. 
 * Save file content, get file content, get md5sum, etc. 
 * Save and get content based on the QFile and get/return QString type. 
 * Don't use save() and read() if you want to works with bigest files.
 */
namespace File 
{

	/**
	 * Read file and
	 * @param file_path     - path to the geting file
	 * @param *content      - string to which will be contain file content
	 * @return The reading status (bool)
	 * @sa save
	 */
	int read ( const QString &file_path, QString &content )
	{
		//open file
		QFile file (file_path);
		if (!file.open(QIODevice::ReadOnly)) {
			return false;
		};

		content = "";

		//read file content
		QTextStream in(&file);
		content = in.readAll();
//		while ( !in.atEnd() ) {
//			content += in.readLine();;
//		};
	
		file.close();
		return true;
	};

	/**
	 * Save content to the specified file. File will be rewrited.
	 * @param file_path    - path to the saving file
	 * @param content      - string which should be saved
	 * @return The saving status (bool)
	 * @sa read
	 */
	int save ( const QString &file_path, const QString &content ) 
	{
		file_mutex.lock();
		//open file
		QFile file (file_path);
		if (!file.open(QIODevice::WriteOnly)) {
			file_mutex.unlock();
			return false;
		};
	    
		//save file
		QTextStream out(&file);
		out << content;
	
		file.close();
		file_mutex.unlock();
	
		return true;
	};


	/**
	 * Append content to the specified file.
	 * @param file_path    - path to the saving file
	 * @param content      - string which should be saved
	 * @return The saving status (bool)
	 * @sa read
	 */
	int append ( const QString &file_path, const QString &content ) 
	{
		file_mutex.lock();
		//open file
		QFile file (file_path);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
			file_mutex.unlock();
			return false;
		};
	    
		//append to file
		QTextStream out(&file);
		out << content;
	
		file.close();
		file_mutex.unlock();
	
		return true;
	};


	/**
	 * Touch file
	 * @param file_path    - path to the saving file
	 * @return The touch status (bool)
	 * @sa read
	 */
	int touch ( const QString &file_path )
	{
		//open file
		QFile file (file_path);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
			return false;
		};
	    
		file.close();
	
		return true;
	};


	/**
	 * Move file (with mutex)
	 * @param target_file - target file path
	 * @param destination_file - destination file path
	 * @return boolean status
	 */
	int move ( const QString& target_file, const QString &destination_file) {
		file_mutex.lock();
		
		bool flag = true;
		
		QFile::remove ( destination_file );

		if ( !QFile::rename ( target_file, destination_file ) )  {
			flag = false;
		};

		file_mutex.unlock();

		return flag;
	};

	/**
	 * Move file to the temporary destination
	 * @param target_file - target file path
	 * @param prefix - file prefix in the destination directory
	 * @return temporary file path
	 */
	QString moveToTemporary ( const QString& target_file,  const QString& prefix ) {
		int counter = 0;
		while ( true ) {
			QString temp_file_path = Config::variables["TempDirectory"] + QDir::separator() + prefix + QString("%1").arg(++counter);
			if ( QFile::rename ( target_file, temp_file_path ) ) {
				return temp_file_path;
			};
			if ( counter > 30000 ) {
				return NULL;
			};
		};
	};

	/**
	 * Copy file to the temporary destination
	 * @param target_file - target file path
	 * @param prefix - file prefix in the destination directory
	 * @return temporary file path
	 */
	QString copyToTemporary ( const QString& target_file,  const QString& prefix ) {
		int counter = 0;
		while ( true ) {
			QString temp_file_path = Config::variables["TempDirectory"] + QDir::separator() + prefix + QString("%1").arg(++counter);
			if ( !QFile::exists ( temp_file_path ) &&  QFile::copy ( target_file, temp_file_path ) ) {
				return temp_file_path;
			};
			if ( counter > 30000 ) {
				return NULL;
			};
		};
	};


	/** 
	 * Recursive copy directory. The destination directory shouldn't exists
	 * \sa recursiveRemoveDirectory
	 * @param source_path - dir which should be copied
	 * @param destination_path - destination directory
	 * @return boolean status (true or false)
	 */
	int recursiveCopyDirectory ( const QString &source_path, const QString &destination_path ) {
		QDir destination_dir ( destination_path );
		QDir source_dir ( source_path );

		//recreate destination directory
		//if ( !File::recursiveRemoveDirectoryContent ( destination_path ) ) {
		//	return false;
		//};
		if ( !destination_dir.exists() ) {
			if ( !destination_dir.mkpath(destination_path) ) {
				return false;
			};
			QFile::setPermissions ( destination_path, QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
		};
		QFile dest_dir( destination_path );
		QFile::setPermissions( destination_path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadUser | QFile::ExeUser | QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther );

		
		//copy list of files from source_dir
		QFileInfoList source_dir_list = source_dir.entryInfoList();
		for (int i = 0; i < source_dir_list.size(); ++i) {
			QFileInfo fileInfo = source_dir_list.at(i);
			if ( fileInfo.fileName() == "." || fileInfo.fileName() == ".." || fileInfo.fileName() == "CVS" ) {
				continue;
			};
			
			QString source_file = source_path + QDir::separator() + fileInfo.fileName();
			QString destination_file = destination_path + QDir::separator() + fileInfo.fileName();
			
			if ( fileInfo.isDir() ) {
				File::logMessage ( LOG_DEBUG, "DIRECTORY" +source_file + "->"  + destination_file );
				if ( !File::recursiveCopyDirectory ( source_file, destination_file ) ) {
					return false;
				};
			} else {
				File::logMessage ( LOG_DEBUG, source_file + "->"  + destination_file );
//TODO!!!!!!!!!! REMOVE THIS
//#ifdef Q_OS_LINUX
//QProcess cmd;
//cmd.start(QString("cp -r %1 %2").arg(source_file).arg(destination_file));
//cmd.waitForFinished();
//#else
				QFile sourceFile( source_file );
				QFile destFile( destination_file );
				bool success = true;
				success &= sourceFile.open( QFile::ReadOnly );
				success &= destFile.open( QFile::WriteOnly | QFile::Truncate );
				success &= destFile.write( sourceFile.readAll() ) >= 0;
				sourceFile.close();
				destFile.close();
				destFile.setPermissions( sourceFile.permissions () );
				if  ( !success ) {
					return false;
				}



//				if ( !QFile::copy( source_file, destination_file ) ) {
//					return false;
//				};
//#endif
			};
		};
		return true;
	};

	/** 
	 * Recursive remove directory. The target_path will be removed completely
	 * \sa recursiveCopyDirectoryContent
	 * @param target_path - path to dir which should be removed
	 * @return boolean status (true or false)
	 */
	int recursiveRemoveDirectory ( const QString &target_path ) {
		if ( !File::recursiveRemoveDirectoryContent(target_path, true) ) {
			return false;
		};
		//remove dir
		QDir target_dir(target_path);
		target_dir.rmdir(target_path);

		return true;
	}


	/** 
	 * Recursive remove directory content. You should note that
	 * target_path isn't removed. Removed only all child files and 
	 * directories
	 * \sa recursiveCopyDirectory
	 * @param target_path - path to dir which content should be removed
	 * @param dont_remove_cvs_files - boolean parameter which indicate skip CVS directories (used for clear log dirs)
	 * @return boolean status (true or false)
	 */
	int recursiveRemoveDirectoryContent ( const QString &target_path, int dont_remove_cvs_files ) {
		QDir target_dir ( target_path );

		if ( !target_dir.exists()  ) {
			return true;
		};
		
		//recursive remove content
		QFileInfoList target_dir_list = target_dir.entryInfoList();
		for (int i = 0; i < target_dir_list.size(); ++i) {
			QFileInfo fileInfo = target_dir_list.at(i);
			if ( fileInfo.fileName() == "." || fileInfo.fileName() == ".." ) {
				continue;
			};

			if ( dont_remove_cvs_files && fileInfo.fileName() == "CVS" ) {
				continue;
			};

			QString target_file = target_path + QDir::separator() + fileInfo.fileName();
			
			if ( fileInfo.isDir() ) {
				if ( !File::recursiveRemoveDirectoryContent ( target_file ) ) {
					return false;
				};
				target_dir.rmdir(fileInfo.fileName());
			} else {
				if ( !QFile::remove ( target_file ) ) {
					return false;
				};
			};
		};
		return true;
	};

	/**
	 * This function create temporary directory and return creted dir name
	 * directory
	 * @param prefix - file prefix
	 * @return temporary directory name
	 */
	QString createTempDirectory ( QString prefix, QString destination_dir ) {
		QDir temp_dir;
		int counter = 0;

		if ( destination_dir == "" ) {
			destination_dir = Config::variables["TempDirectory"];
		};
		if ( !temp_dir.exists(destination_dir) ) { //create it
			temp_dir.mkpath ( destination_dir );
			QFile::setPermissions ( destination_dir, QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
		};
		
		//search first not exist directory
		while ( true ) {
			QString temp_dir_path = destination_dir + QDir::separator() + prefix + QString("%1").arg(++counter);

			if ( !temp_dir.exists(temp_dir_path) ) {
				if ( temp_dir.mkpath ( temp_dir_path ) ) {
					
					QFile::setPermissions ( temp_dir_path, QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
					File::logMessage ( LOG_DEBUG, temp_dir_path );
					return temp_dir_path;
				};
			};

			if ( counter > 32768 ) { 
				File::logMessage ( LOG_WARNING, "Couldn't create temporary dir in the directory: " + destination_dir );
				return "";
			};
		};
	};

	/**
	* This function create temporary file and return creted file name
	* directory
	* @param prefix - file prefix
	* @return temporary directory name
	*/
	QString createTempFile ( QString prefix, QString destination_dir ) 
	{
		if ( destination_dir == "" ) {
			destination_dir = Config::variables["TempDirectory"];
		};
		if ( !QFile::exists(destination_dir) ) {
			File::logMessage( LOG_WARNING, "Couldn't create temporary file. Directory " + destination_dir + " doesn't exists") ;
			return "";
		};
		

		int counter = 0;
		while ( true ) {
			QString temp_file_path = destination_dir + QDir::separator() + prefix + QString("%1").arg(++counter);
			if ( !QFile::exists ( temp_file_path ) && File::touch(temp_file_path) ) { 
				return temp_file_path;
			};
			if ( counter > 30000 ) {
				File::logMessage ( LOG_WARNING, "Couldn't create temporary dir in the directory: " + destination_dir );
				return NULL;
			};
		};
	
	}


	/**
	 * Function calculate MD5 file sum.
	 * @param file_path - path to the file 
	 * @return MD5 hash string. See openssl.h for hash length
	 */
	QString getMD5 ( const QString &file_path )
	{
		const qint64 block_size = 16*1024;
		char         buffer[16*1024];
		QIODevice*   file_data_dev;
		MD5::Context md5sum_ctx;

		//init openssl MD5 ctx
		MD5::Init(&md5sum_ctx);

		//open file
		file_data_dev = new QFile(file_path);
		if ( !file_data_dev->open(QIODevice::ReadOnly) ) {
			delete file_data_dev;
			return "";
		};

		//read file by 16*1024 blocks and calculate block sum
		while ( !file_data_dev->atEnd() ) {
			qint64 read = file_data_dev->read(buffer, block_size);

			QByteArray block(buffer,read);

			MD5::Update(&md5sum_ctx, (unsigned char*)block.data(), block.size() );

			if (!file_data_dev) {
				delete file_data_dev;
				return "";
			};
		};

		//close file
		file_data_dev->close();
		delete file_data_dev;

		//convert md5sum_ctx to the char
		char md5sum_bin[32];
		char md5sum_hex[32];
		MD5::Final(&md5sum_ctx, (unsigned char*)md5sum_bin);
		MD5::CvtHex ((unsigned char*)md5sum_bin,(unsigned char*)md5sum_hex);

		return QString(md5sum_hex);
	};

	/**
	 * Function recursive calculate MD5 sum for each file in the directory.
	 * @param dir_path - path to the directory 
	 * @return MD5 hash string. See openssl.h for hash length
	 */
	QString getDirMD5 ( const QString &dir_path ) 
	{
		QVector <QString> files_list = File::getFilesList(dir_path);
		if ( !files_list.size() ) {
			return "";
		};
		
		QString full_md5_string = "";
		for (int i = 0; i < files_list.size(); ++i) {
			full_md5_string += QString(files_list.at(i)).replace(QDir::separator(),"/") + ":" + File::getMD5 ( dir_path + QDir::separator() + files_list.at(i) ) + ";";
		};
		//uncomment for debug
		return full_md5_string;

		return File::getStrMD5(full_md5_string);
	};


	/**
	 * Function calculate MD5 sum for string
	 * @param string - input string
	 * @return MD5 hash string. See openssl.h for hash length
	 */
	QString getStrMD5 ( const QString &full_md5_string ) 
	{
		// calculate result MD5 for string
		MD5::Context md5sum_ctx;
		MD5::Init(&md5sum_ctx);
		QByteArray block; 
		block.append(full_md5_string);
		MD5::Update(&md5sum_ctx, (unsigned char*)block.data(), block.size() );

		//convert md5sum_ctx to the char
		char md5sum_bin[32];
		char md5sum_hex[32];
		MD5::Final(&md5sum_ctx, (unsigned char*)md5sum_bin);
		MD5::CvtHex ((unsigned char*)md5sum_bin,(unsigned char*)md5sum_hex);

		return QString(md5sum_hex);
	};



	/**
	 * Create Discomp archive from directory filelist. 
	 * archive have primitive structure which included xml defenition about all items,
	 * and after xml_structure to the file added data for all files
	 * @param target_dir - target directory
	 * @param archive_path - destination archive path
	 * @return status of creating
	 * \sa extractArchive
	 */
	bool createArchive ( const QString &target_dir, const QString &archive_path )
	{
		File::logMessage(LOG_COMMON, "Creating file archive for dir: " + target_dir + ". Archive file: " + archive_path );
		if ( target_dir == "" || archive_path == "" ) {
			File::logMessage (LOG_ERROR, "Some of archiving parameter is empty");
			return false;
		};

		QVector <QString> files_list = File::getFilesList(target_dir);

		if ( files_list.size() == 0 ) {
			File::logMessage ( LOG_ERROR, "[archive create] Filelist is empty");
			return false;
		};

		//first generating archive files structure
		QString files_structure_xml = "<?xml version='1.0' encoding='UTF-8'?>\n";
		files_structure_xml += "<fileslist>\n";

		for (int i = 0; i < files_list.size(); ++i) {
			QString file_name = files_list.at(i);
			QFileInfo file_info ( target_dir + QDir::separator() + file_name );
			
			//relative file dir
			QString file_dir = file_name.left( file_name.length() - file_info.fileName().length() );

			files_structure_xml += "\t<item type='file' name='" + file_name + "' size='" + QString("%1").arg(file_info.size()) + "' exe='" + QString("%1").arg(file_info.isExecutable()) + "' dir='" + file_dir + "'/>\n"; 
			File::logMessage ( LOG_COMMON, "[archive create] File: " + file_name + ", size:" + QString("%1").arg(file_info.size()) );
		};
		files_structure_xml += "</fileslist>\n";

		logMessage ( LOG_DEBUG, files_structure_xml );

		//create archive file
		QFile archive_file (archive_path);
		if (!archive_file.open(QIODevice::WriteOnly)) {
			File::logMessage ( LOG_ERROR, "[archive create] Couldn't open file for writing: " + archive_path );
			return false;
		};

		//save archive files structure to the destination archive path
		QByteArray files_structure_data; 
		files_structure_data.append(files_structure_xml);
		
		//first 4 char should contan size of the files_structure_data
		QByteArray structure_size(4,'0');
		QByteArray tmp_byte_arr = QByteArray::number(files_structure_data.size(), 16);
		structure_size.replace(4-tmp_byte_arr.size(), tmp_byte_arr.size(), tmp_byte_arr );

		archive_file.write( structure_size.data(), structure_size.size() );
		archive_file.write( (const char*)files_structure_data.data(), files_structure_data.size() );
		

		//now add files data to the archive file
		for (int i = 0; i < files_list.size(); ++i) {
			QFileInfo file_info ( target_dir + QDir::separator() + files_list.at(i) );
			if ( file_info.isDir() ) {
				continue;
			};

			QFile target_file ( target_dir + QDir::separator() + files_list.at(i) );

			if ( !target_file.open(QIODevice::ReadOnly) ) {
				File::logMessage ( LOG_ERROR, "[archive create] Couldn't open file for reading: " + target_dir + QDir::separator() + files_list.at(i) );
				return false;
			}

			const qint64 block_size = 16*1024;
			char         buffer[16*1024];

			// read block from target file by 16Kb blocks and write it to the archive file
			while ( !target_file.atEnd() ) {
				qint64 read = target_file.read(buffer, block_size);
				QByteArray block(buffer,read);

				archive_file.write( block.data(), read );
			};
		};
		archive_file.close();
		
		return true;
	};

	/**
	 * Extract Discomp archive from file to the specified directory. 
	 * @param archive_path - target archive path
	 * @param destination_dir - destination directory path
	 * @return status of extracting
	 * \sa createArchive
	 */
	bool extractArchive ( const QString &archive_path, const QString &destination_dir )
	{ 
		File::logMessage(LOG_COMMON, "Extract archive: " + archive_path + " to the dir: " + destination_dir );
		if ( destination_dir == "" || archive_path == "" ) {
			File::logMessage (LOG_ERROR, "Some of archiving parameter is empty");
			return false;
		};

		QFileInfo archive_file_info ( archive_path );
		if ( archive_file_info.size() < 5 ) {
			File::logMessage ( LOG_ERROR, "[archive extract] archive file size less than 4");
			return 0;
		};

		QFile archive_file (archive_path);
		if (!archive_file.open(QIODevice::ReadOnly)) {
			File::logMessage ( LOG_ERROR, "[archive extract] Couldn't open archive file : " + archive_path );
			return false;
		};

		//read xml structure size	
		QByteArray tmp_block_size;
		tmp_block_size.resize ( 4 );
		archive_file.read( tmp_block_size.data(), tmp_block_size.size() );
		qint64 structure_size = tmp_block_size.toLongLong( NULL, 16);

		//read xml files structure data
		tmp_block_size.resize ( structure_size );
		archive_file.read( tmp_block_size.data(), tmp_block_size.size() );
		QString files_structure_xml = QString(tmp_block_size);

		logMessage ( LOG_DEBUG, files_structure_xml );

		//parse xml structure
		QString errorStr;
		int errorLine;
		int errorColumn;
		QDomDocument doc;
		if ( !doc.setContent( files_structure_xml, true, &errorStr, &errorLine, &errorColumn ) ) {
			File::logMessage ( LOG_ERROR, QString("[archive extract] XML processing error ( line %1, column %2: %3 )").arg(errorLine).arg(errorColumn).arg(errorStr) );
			return 0;
		};
		QDomElement rootNode = doc.documentElement();
		if (rootNode.tagName() != "fileslist") {
			File::logMessage ( LOG_ERROR, "[archive extract] Error parsing filelist structure defenition. Couldn't find root <filelist> node" );
			return 0;
		};
		
		//processing all structure items
		QDomNode data_node = rootNode.firstChild();
		while ( !data_node.isNull() ) {
			if ( data_node.toElement().tagName() == "item" ) {
				QString file_name   = data_node.toElement().attribute("name");
				qint64  file_size   = data_node.toElement().attribute("size").toLongLong();
				QString file_dir    = data_node.toElement().attribute("dir");
				int     file_exec   = data_node.toElement().attribute("exe").toInt();

				QString destination_file_path = destination_dir + QDir::separator() + QDir::convertSeparators(file_name);
				QString destination_file_dir  = destination_dir + QDir::separator() + QDir::convertSeparators(file_dir);

				// check if dir is exist
				QDir tmp_dir ( destination_file_dir );
				if ( !tmp_dir.exists() ) {
					if ( !tmp_dir.mkpath(destination_file_dir) ) {
						File::logMessage ( LOG_ERROR, "[archive extract] Couldn't create directory" + destination_file_dir );
						return false;
					};
					QFile::setPermissions ( destination_file_dir, QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
				};
				
				
				// extract file
				File::logMessage(LOG_COMMON,  "[archive extract] File: " + destination_file_path );

				file_mutex.lock();
				QFile destination_file ( destination_file_path );
				if ( !destination_file.open(QIODevice::WriteOnly) ) {
					File::logMessage ( LOG_ERROR, "[archive extract] Couldn't open file for writing" + destination_file_path );
					return false;
				};

				// read data from archive file by 16Kb blocks and write to the destination file
				qint64 processed_file_size = 0;
				qint64 block_size = 16*1024;
				char   buffer[16*1024];
				while ( processed_file_size < file_size ) {
					if ( file_size - processed_file_size < block_size ) {
						block_size = file_size - processed_file_size;
						if ( block_size < 1 ) {
							break;
						};
					};
					qint64 read = archive_file.read(buffer, block_size);
					QByteArray block(buffer,read);

					destination_file.write( block.data(), read );

					processed_file_size += block_size;
				};

				//set permissions
				if ( file_exec == 1 ) {
					destination_file.setPermissions ( QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
				};


				destination_file.close();

				file_mutex.unlock();

				//check file size from structure defenition and on the disk
				QFileInfo destination_file_info ( destination_file_path );
				if ( destination_file_info.size() != file_size ) {
					File::logMessage ( LOG_ERROR, "[archive extract] File size in the archive strcture and on the disk are differ. File: " + destination_file_path );
					return false;
				};

			};
			data_node = data_node.nextSibling();
		};

		return true;
	};


	/**
	 * Recursive get files list
	 * @param dir_path - target directory path
	 * @return vector with relative files paths
	 */
	QVector<QString> getFilesList ( const QString &dir_path ) 
	{
		QDir target_dir ( dir_path );
		
		QVector<QString> files_list;

		if ( !target_dir.exists()  ) {
			return files_list;
		};
		
		
		//recursive calculate md5 sum
		QFileInfoList target_dir_list = target_dir.entryInfoList();
		for (int i = 0; i < target_dir_list.size(); ++i) {
			QFileInfo fileInfo = target_dir_list.at(i);
			if ( fileInfo.fileName() == "." || fileInfo.fileName() == ".." ) {
				continue;
			};

			if ( fileInfo.fileName() == "CVS" ) {
				continue;
			};

			QString target_file = dir_path + QDir::separator() + fileInfo.fileName();
			
			if ( fileInfo.isDir() ) {
				//files_list.append( fileInfo.fileName() );
				QVector<QString> dir_files_list = File::getFilesList ( target_file );
				for (int i = 0; i < dir_files_list.size(); ++i) {
				 	files_list.append( fileInfo.fileName() + QDir::separator() + dir_files_list.at(i));
				};
			} else {
				files_list.append(fileInfo.fileName());
			};
		};
		return files_list;
	};

	/**
	 * Security chech file name that it's not contained forbidden characters
	 * like "../", ";", "," etc.
	 * @param file_path - file path
	 * @return boolean status good or not
	 */
	int checkFileName ( const QString &file_path )
	{
		if ( 	file_path.contains("../") || 
			file_path.contains("/..") || 
			file_path.contains("|") || 
			file_path.contains(">") || 
			file_path.contains("<") || 
			file_path.contains("*") || 
			file_path.contains("~") || 
			file_path.contains("&") || 
			file_path.contains("?") || 
			file_path.contains("#") || 
			file_path.contains("\"") || 
			file_path.contains(";") || 
			file_path.contains("^") || 
			file_path.contains("$") || 
			file_path.contains(",") ) 
		{
			return false;
		};
		return true;
	};




	/**
	 * Add log message
	 * @param log_level - log level from log.h
	 * @param message - log message
	 */
	void logMessage ( int log_level, const QString &message ) 
	{
		Log::addMessage (log_level, "File",  message);
	};
};


// vim: set fenc=utf-8 tabstop=8 :
