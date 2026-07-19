// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2011 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Authors: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>     |
// +--------------------------------------------------------------------+

#include <QDateTime>
#include <QDebug>
#include <QtXml>
#include <QStringList>
#include "xml_rrd.h"


#include "lib/common.h"
#include "lib/file.h"
#include "lib/log.h"
#include "config.h"



/**
 * @brief Class for for system monitoring
 *
 * This class provide methods for wotks with ...
 * ...
 * ...
 */










/**
  * Constructor
  */
DiscompXMLRRD::DiscompXMLRRD() 
{
	m_rra_cycle_step = 0;
	m_subdir_path="";
}

/**
  * Destructor
  */
DiscompXMLRRD::~DiscompXMLRRD()
{
}


void DiscompXMLRRD::Init (  )
{
    m_rra_cycle_step = 0;
    Log::addMessage(LOG_COMMON, "monitor", "Init monitor Round Robin Archives");

    QStringList rras_config = Config::variables.value("MonitorRRAs").split(" ");
    for (int i=0; i<rras_config.size(); i++){
        if ( rras_config.at(i).size() > 2 ) {
		QStringList rra_conf = rras_config.at(i).split(":");
		QPair<int,int> conf;
		conf.first = rra_conf.at(0).toInt();
		conf.second = rra_conf.at(1).toInt();

		this->m_rra_conf.push_back( conf );

		QVector <QDomDocument> *vector = new QVector<QDomDocument>();
		this->m_rra_data.push_back( vector );
        };
    };

}


/**
 * Start thread
 */
void DiscompXMLRRD::setData ( const QString& stat_xml )
{
	//init base agregation object by first element
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument stat_doc;
	if ( !stat_doc.setContent( stat_xml, true, &errorStr, &errorLine, &errorColumn ) ) {
		Log::addMessage (LOG_ERROR, "monitor rrd", QString("Monitor stat Error: XML processing error ( line %1, column %2: %3 ). Message skiped.").
		                               arg(errorLine).arg(errorColumn).arg(errorStr) );
		return;
	};
	if ( stat_doc.documentElement().toElement().tagName() != "stat" ) {
		Log::addMessage (LOG_ERROR, "monitor rrd", "Monitor stat Error: Root node is not stat");
		return;
	};




	this->m_rra_cycle_step++;
        Log::addMessage(LOG_COMMON, "monitor", QString("Cycle step: %1").arg(this->m_rra_cycle_step) );

	for ( int rra_num=0; rra_num < this->m_rra_conf.size(); rra_num++ ) {
		if ( m_rra_cycle_step % this->m_rra_conf.at(rra_num).first == 0 ) {
			Log::addMessage(LOG_COMMON, "monitor", QString("Should  call: %1").arg(rra_num) );
			QVector <QDomDocument>  *rra_current = this->m_rra_data.at(rra_num);


	
			//first array shhould be as is
			if ( rra_num == 0 ) {
				rra_current->push_front ( stat_doc );
				this->dumpArchiveToDisk ( rra_num );
			} else {

	Log::addMessage(LOG_COMMON, "", "===============Agr start==============================");
				rra_current->push_front ( this->Agregate(rra_num-1, this->m_rra_conf.at(rra_num).first) );
				this->dumpArchiveToDisk ( rra_num );
	Log::addMessage(LOG_COMMON, "", "===============Agr done==============================");
			};
			//remove latest element if array size is more than config specified
			if ( rra_current->size() > this->m_rra_conf.at(rra_num).second ) {
				rra_current->pop_back();
				Log::addMessage(LOG_COMMON, "monitor", QString("Should remove last: %1").arg(rra_num) );
			};
		};
	};
}

QDomDocument DiscompXMLRRD::Agregate ( int rra_num, int agr_count ) 
{
	Log::addMessage(LOG_COMMON, "monitor", QString("Need to agregate for : %1").arg(rra_num) );


	QVector <QDomDocument>  *rra_current = this->m_rra_data.at(rra_num);
	//TODO
	if ( rra_current->size() <= 0 ) { return QDomDocument(); };
	QString first_stat_xml = "<?xml version='1.0'?><stat><info var1='123' var2='445'/></stat>";


	//init base agregation object by first element
	QDomDocument agregateDoc(rra_current->at(0));
/*
	if ( !agregateDoc.setContent( first_stat_xml, true, &errorStr, &errorLine, &errorColumn ) ) {
		Log::addMessage (LOG_ERROR, "monitor rrd", QString("Agregate Error: XML processing error ( line %1, column %2: %3 ). Message skiped.").
		                               arg(errorLine).arg(errorColumn).arg(errorStr) );
		return agregateDoc;
	};
	if ( agregateDoc.documentElement().toElement().tagName() != "stat" ) {
		Log::addMessage (LOG_ERROR, "monitor rrd", "Agregate Error: Root node is not stat");
		return agregateDoc;
	};
*/

	Log::addMessage (LOG_ERROR, "monitor rrd", QString("Current size: %1").arg(rra_current->size())) ;

	int counter = 0;
	for ( int i=1; i<rra_current->size() && i<=agr_count; i++ ) 
	{
		//init current xml stat data
		QDomDocument currentDoc(rra_current->at(i));
		//TODO. nned too check here that document isn't empty!!!!!!!!!!
		if ( 0 ) {
			continue;
		};
		counter++;

		this->mergeDocs(agregateDoc.documentElement().firstChild(), currentDoc.documentElement().firstChild());
	};

	this->divideByCounter(agregateDoc.documentElement().firstChild(), counter+1);
//	Log::addMessage(LOG_COMMON, "", agregateDoc.toString() );
	return agregateDoc;
}

void DiscompXMLRRD::mergeDocs( QDomNode agregateNode, QDomNode targetNode)
{
//	Log::addMessage(LOG_COMMON, "monitor rrd", "Attach");

	while ( !agregateNode.isNull() ) 
	{
		if ( agregateNode.hasAttributes() ) {
//			Log::addMessage(LOG_COMMON, "monitor rrd", "<" +   agregateNode.toElement().tagName() + "> : <" + targetNode.toElement().tagName() + ">" );
			for ( int i=0; i <  agregateNode.attributes().size(); i++ ) {
				QString attr_name = agregateNode.attributes().item(i).nodeName();
				QString agr_value = agregateNode.toElement().attribute(attr_name);
				QString trg_value = targetNode.toElement().attribute(attr_name);
//				if ( agr_value.contains(QRegExp("^[0-9|\.]+$")) ) {
				if ( agr_value.contains(QRegExp("^[0-9]+$")) ) {
//					Log::addMessage(LOG_COMMON, "attr value : ", attr_name +  " : " + agr_value + ":" + trg_value  );
					agregateNode.toElement().setAttribute(attr_name, agr_value.toLongLong() + trg_value.toLongLong());
				};
				
			};
		};
		if ( agregateNode.hasChildNodes() ) {
			this->mergeDocs(agregateNode.firstChild(), targetNode.firstChild());
		};
		agregateNode = agregateNode.nextSibling();
		targetNode = targetNode.nextSibling();
	};
}


void DiscompXMLRRD::divideByCounter( QDomNode agregateNode, int counter )
{
//	Log::addMessage(LOG_COMMON, "monitor rrd", "Divide");

	while ( !agregateNode.isNull() ) 
	{
		if ( agregateNode.hasAttributes() ) {
//			Log::addMessage(LOG_COMMON, "monitor rrd", "<" +   agregateNode.toElement().tagName() + ">");
			for ( int i=0; i <  agregateNode.attributes().size(); i++ ) {
				QString attr_name = agregateNode.attributes().item(i).nodeName();
				QString agr_value = agregateNode.toElement().attribute(attr_name);
				if ( agr_value.contains(QRegExp("^[0-9]+$")) ) {
//					Log::addMessage(LOG_COMMON, "attr value : ", attr_name + " : " + agr_value  );
					agregateNode.toElement().setAttribute(attr_name, (int)(agr_value.toLongLong() / counter ));
				};
				
			};
		};
		if ( agregateNode.hasChildNodes() ) {
			this->divideByCounter(agregateNode.firstChild(), counter);
		};
		agregateNode = agregateNode.nextSibling();
	};
}


void DiscompXMLRRD::dumpArchiveToDisk ( int rra_num ) 
{

	QVector <QDomDocument>  *rra_current = this->m_rra_data.at(rra_num);
	if ( rra_current->size() <= 0 ) { return; };

	QString dir_path = Config::variables.value("MonitorDataDirectory") + QDir::separator() + m_subdir_path + QDir::separator() +  QString("%1").arg(rra_num);
	QDir dir;
	if ( !dir.exists(dir_path) ) {
		dir.mkpath(dir_path);
	};

	//permissions
	QFile::setPermissions ( Config::variables["MonitorDataDirectory"], QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
	QFile::setPermissions ( Config::variables["MonitorDataDirectory"] + QDir::separator() + m_subdir_path, QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );
	QFile::setPermissions ( Config::variables["MonitorDataDirectory"] + QDir::separator() + m_subdir_path + QDir::separator() +  QString("%1").arg(rra_num) , QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadUser|QFile::WriteUser|QFile::ExeUser );

	for ( int i=rra_current->size()-2; i>=0; i-- ) 
	{
//		Log::addMessage (LOG_COMMON, "dsaf",  QString("%1/%2.xml").arg(dir_path).arg(i) + " " + QString("%1/%2.xml").arg(dir_path).arg(i+1));
		File::move( QString("%1/%2.xml").arg(dir_path).arg(i), QString("%1/%2.xml").arg(dir_path).arg(i+1));
	};

	File::save(QString("%1/%2.xml").arg(dir_path).arg(0), rra_current->at(0).toString() );
}

void DiscompXMLRRD::setSubdirPath ( const QString & path )
{
	m_subdir_path = path;
}
