#ifndef JS_INTERPRETATOR_H
#define JS_INTERPRETATOR_H

#include <QThread>
#include <QScriptEngine>
#include <QMutex>
#include <lib/log.h>

class DiscompJSInterpretator : public QThread
{
    Q_OBJECT
public:
    DiscompJSInterpretator();
    void setEngine(QScriptEngine *engine);
    void setCode(QString code);
    void setMutex(QMutex *unimutex);
    void run();
    void logMessage( int log_level, QString message );
private:
    QScriptEngine* script_engine;
    QString js_code;
    QMutex *unimutex;

};

#endif // JS_INTERPRETATOR_H
