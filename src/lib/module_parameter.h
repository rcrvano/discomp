// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#include <QString>
#include <QObject>
#include <QVector>
#include <QtXml>


#ifndef MODULEPARAMETER_H
#define MODULEPARAMETER_H

#define LIST_SIZE_LIMIT 65536

class DiscompModuleParameter : public QObject
{
	Q_OBJECT

public:

	DiscompModuleParameter  ( QObject *parent = 0 );
	~DiscompModuleParameter ( );

	int   Init              ( const QDomNode &paramNode );

	QString  dir;
	bool     clear_before_destroy;
	QString  constant_tmp_path;
	bool     input;

	QString  name;
	QString  type;
	QString  comment;

	int      constant;

	QString  value;

	QString  md5; //!< var for store paramateter md5. used in the "Send module temporarary output parameters to server"

	/*file*/
	QString  filename;
	QString  filepath;
	
	/*filelist*/
	QString  filepattern;
	QString  remote_filename;
	int      list_size;
	int      listsize   ( bool hard = false );
	qint64   filesize   ( );


	int     is_defined        ( bool hard = false );
	void    clear             ( );
	void    touch             ( );
	QString getFileTypeValue  ( int blocked = 0 );
	bool    setFileTypeValue  ( const QString & param_value );
	QString getListTypeValue  ( int list_num, int blocked = 0 );
	bool    setListTypeValue  ( int list_num, const QString &param_value );

	void    checkListParameterSequence ( );

signals :
	void logMessage_signal ( int log_level, const QString &message );

};

#endif
// vim: set fenc=utf-8 tabstop=8 :
