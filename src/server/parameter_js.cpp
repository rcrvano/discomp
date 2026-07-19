#include "parameter_js.h"

DiscompParameterJS::DiscompParameterJS(QString name, DiscompProcessJS *process)
{
    for (int i = 0; i < process->parameters.size(); i++) {
	if (process->parameters[i]->name == name) {
	    this->parameter = process->parameters[i];
	}
    }
    this->log_file = "";
    this->parentProcess = process;
}

QString DiscompParameterJS::getValue()
{
    QString result = "";
    if (this->parameter->type != "file") {
        logMessage(LOG_ERROR, "getValue() you should use getElementValue(int i) instead");
    }
    result = this->parameter->getFileTypeValue(0);
    return result;
}

QString DiscompParameterJS::getValueBlocked()
{
	if (parentProcess->EDP) {
		logMessage(LOG_COMMON, "getValueBlocked(): you should not use this function in EDP mode");
	}
    QString result = "";
    if (this->parameter->type != "file") {
        logMessage(LOG_ERROR, "getValue() you should use getElementValueBlocked(int i) instead");
    }
    result = this->parameter->getFileTypeValue(1);
    return result;
}

QString DiscompParameterJS::getElementValue(int listnum)
{
    QString result = "";
    if (this->parameter->type == "file") {
            logMessage(LOG_ERROR, "getElementValue(): you should getValue() instead");
    }
    result = this->parameter->getListTypeValue(listnum, 0);
    return result;
}

QString DiscompParameterJS::getElementValueBlocked(int listnum)
{
	if (parentProcess->EDP) {
		logMessage(LOG_COMMON, "getElementValueBlocked(): you should not use this function in EDP mode");
	}
    QString result = "";
    if (this->parameter->type == "file") {
            logMessage(LOG_ERROR, "getElementValue(): you should getValue() instead");
    }
    result = this->parameter->getListTypeValue(listnum, 1);
    return result;
}

int DiscompParameterJS::setValue(QString value){
    int result;
    if (this->parameter->type != "file") {
        logMessage(LOG_ERROR, "setValue(): you should use setElementValue(i, value) instead");
    }
    result = this->parameter->setFileTypeValue(value);
    return result;
}

int DiscompParameterJS::setElementValue(int listnum, QString value){
    int result;
    if (this->parameter->type != "file") {
        logMessage(LOG_ERROR, "setValue(): you should use setValue(value) instead");
    }
    result = this->parameter->setListTypeValue(listnum, value);
    return result;
}

int DiscompParameterJS::length()
{
    return this->parameter->listsize(true);
}

void DiscompParameterJS::logMessage ( int log_level, const QString &message )
{
	if ( this->log_file != "" ) {
		Log::addMessage ( this->log_file, log_level, "DiscompModuleJS", message);
	} else {
		Log::addMessage ( log_level, "DiscompModuleJS", message);
	};
};
