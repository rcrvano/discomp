// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Slastnoy Konstantin <qeadzc777@yandex.ru>                  |
// +--------------------------------------------------------------------+


#ifndef MODULE_JS_H
#define MODULE_JS_H

#include <QVector>
#include <QProcess>
#include <QTimer>
#include <QString>
#include <QtXml>
#include <QDateTime>
#include <QTime>
#include <QtScript>

#include "process_stage.h"
#include "process_js.h"
#include "lib/module_parameter.h"
#include "lib/log.h"

class DiscompModuleParameter;
class DiscompProcessJS;

class DiscompModuleJS : public DiscompProcessStageModule
{
	Q_OBJECT
public:
	DiscompModuleJS (QString name, DiscompProcessJS *parentProcess);
	DiscompModuleJS(DiscompProcessJS *parentProcess, DiscompModuleJS *parentModule, QString name, int list_num);
	~DiscompModuleJS ();

	QMap <QString,QString> events;      //!< Process stages interpreter events (onStart, onFinish, etc.)

	Q_INVOKABLE int isReady();
	Q_INVOKABLE int start();
	Q_INVOKABLE int startElement(int list_num);
	Q_INVOKABLE int stop();
	Q_INVOKABLE int waitForFinished(int interval = 1000);
	Q_INVOKABLE int waitForElementFinished(int element, int interval = 1000);
	Q_INVOKABLE int getStatus();
	Q_INVOKABLE int getElementStatus(int list_num);
	Q_INVOKABLE bool setEventListener(QString event, QString script, int list_num = -1);
	void processEvent ( const QString& event );

	void    logMessage   ( int log_level, const QString &message );
	QString log_file;
	DiscompProcessJS *parentProcess;
	bool areAllInputParametersExist();
private:
	QMap<int,DiscompModuleJS*> sub_modules; //only if this module has file type input param, but in parameters.xml this param is filelist type
	DiscompModuleJS* parentModule;
signals:
	void moduleStarted_signal();
};

#endif

// vim: set fenc=utf-8 tabstop=8 :
