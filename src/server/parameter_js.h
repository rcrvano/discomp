#ifndef PARAMETER_JS_H
#define PARAMETER_JS_H

#include <QString>
#include <QObject>
#include <lib/module_parameter.h>
#include "process_js.h"
#include <lib/log.h>


class DiscompProcessJS;
class DiscompParameterJS: public QObject
{
	Q_OBJECT
public:
    DiscompParameterJS(QString name, DiscompProcessJS *process);
    DiscompModuleParameter *parameter;
    DiscompProcessJS *parentProcess;

    Q_INVOKABLE QString getValue();
    Q_INVOKABLE QString getValueBlocked();
    Q_INVOKABLE QString getElementValue(int listnum);
    Q_INVOKABLE QString getElementValueBlocked(int listnum);

    Q_INVOKABLE int setValue(QString value);
    Q_INVOKABLE int setElementValue(int listnum, QString value);

    Q_INVOKABLE int length();

    void logMessage ( int log_level, const QString &message );
    QString log_file;
};

#endif // PARAMETER_JS_H
