#include "js_interpretator.h"

DiscompJSInterpretator::DiscompJSInterpretator() :
	QThread(){}

void DiscompJSInterpretator::run(){
    logMessage(LOG_COMMON, "hello i'm runned");
    QScriptValue result = this->script_engine->evaluate( this->js_code );
    if ( this->script_engine->hasUncaughtException() ) {
	    int line = this->script_engine->uncaughtExceptionLineNumber();
	    logMessage(LOG_ERROR, QString("Uncaught exception at line %1 : %2").arg(line).arg(result.toString()));
    }
}

void DiscompJSInterpretator::logMessage ( int log_level, QString message )
{
	Log::addMessage ( log_level, "DiscompProcessJS", message);
};
void DiscompJSInterpretator::setEngine(QScriptEngine *engine) {
    this->script_engine = engine;
}
void DiscompJSInterpretator::setCode(QString code) {
    this->js_code = code;
}
void DiscompJSInterpretator::setMutex(QMutex *unimutex) {
    this->unimutex = unimutex;
}
// TODO code dublication from process_plugin.cpp -
// evaluate script and if it has errors print them
/*
bool DiscompJSInterpretator::evaluateScript( const QString & script )
{
	QScriptValue result = process_script_engine->evaluate( script );
	if ( process_script_engine->hasUncaughtException() ) {
		int line = process_script_engine->uncaughtExceptionLineNumber();
		logMessage(LOG_ERROR, QString("Uncaught exception at line %1 : %2").arg(line).arg(result.toString()));
		return false;
	}
	return true;
}
*/
