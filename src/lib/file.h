// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef FILE_H
#define FILE_H

#include <QString>
#include <QMutex>
#include <QHash>
#include "config.h"

namespace File {
	int     read                            ( const QString &file_path,         QString &content          );
	int     save                            ( const QString &file_path,   const QString &content          );
	int     append                          ( const QString &file_path,   const QString &content          );
	int     touch                           ( const QString &file_path                                    );
	int     move                            ( const QString &file_path,   const QString &destfile         );
	QString getMD5                          ( const QString &file_path                                    );
	QString getDirMD5                       ( const QString &dir_path                                     ); 
	QString getStrMD5                       ( const QString &string                                       ); 
	int     checkFileName                   ( const QString &file_path                                    );
	QString createTempDirectory             (       QString prefix,             QString destination_dir="");
	QString createTempFile                  (       QString prefix,             QString destination_dir="");
	QString moveToTemporary                 ( const QString &target_file, const QString &prefix           );
	QString copyToTemporary                 ( const QString &target_file, const QString &prefix           );
	int     recursiveRemoveDirectory        ( const QString &target_path                                  );
	int     recursiveRemoveDirectoryContent ( const QString &target_path, int dont_remove_cvs_files=true  );
	int     recursiveCopyDirectory          ( const QString &source_path, const QString &destination_path );

	QVector<QString> getFilesList           ( const QString &target_dir                                   ); 
	bool    createArchive                   ( const QString &target_dir,   const QString &archive_path    );
	bool    extractArchive                  ( const QString &archive_path, const QString &destination_dir );
	
	void    logMessage                      ( int log_level,               const QString &message         ); 
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
