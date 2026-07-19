// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2011 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Authors: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>     |
// |          Madrahimov Bunyod  <bunyodmadraximov@gmail.com>           |
// +--------------------------------------------------------------------+

#include <QDebug>

#include "clips_api.h"

#include "config.h"
#include "lib/common.h"
//#include "database.h"
#include "lib/log.h"

//#include "../lib/clips/clips.h"

#include <stdio.h> 
//#include "../lib/clips/setup.h"
#include "../lib/clips/clips.h"
//#include "../lib/clips/cmptblty.h"
#include "lib/file.h"
#include "lib/xml.h"
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlNamePool>
#include <QtXmlPatterns/QXmlName>

#include <QFile>
#include <iostream>


static FILE *TraceFP = NULL;
int FindTrace( char *logicalName ) {
	return TRUE;
}
int PrintTrace( char *logicalName, char *str ) {
	fprintf(TraceFP,"%s",str);
}
int ExitTrace( int exitCode ) {
	fclose(TraceFP);
}

/**
  * Constructor
  */
DiscompClips::DiscompClips(QObject *parent) 
{
	Log::addMessage(LOG_COMMON, "monitor", "Clips object constructed");

	// Init CLIPS enviroment
	InitializeEnvironment();


	if ( TraceFP == NULL ) {
		TraceFP = fopen("/tmp/trace.txt","w");
		if (TraceFP == NULL) return;
	}

	// Don't print anythyng on display
	AddRouter("display",             /* Router name     */
             20,                         /* Priority        */
             FindTrace,                  /* Query function  */
             PrintTrace,                 /* Print function  */
             NULL,                       /* Getc function   */
             NULL,                       /* Ungetc function */
             ExitTrace);                 /* Exit function   */
}

/**
  * Destructor
  */
DiscompClips::~DiscompClips()
{
	Log::addMessage(LOG_COMMON, "monitor", "Clips object destructed");
}




/**
 * Init CLIPS enviroment, load all facts, and execute rules
 */
void DiscompClips::doAnalyse()
{
        logMessage(LOG_COMMON, "Processing monitoing data from nodes with CLIPS");

	
	IncrementGCLocks();

	// clear clips enfiroment
	Clear();
	
	//save output to file
	DribbleOn((char*) Config::variables.value("ClipsOutFile").toStdString().c_str());


	//load facts templates
        Load ( (char*) Config::variables.value("ClipsFactsTemplates").toStdString().c_str() );
	
	//load facts for each node (it's converted from XML to CLIPS format)
	this->loadMonitoringFacts(  );

	//TODO.remove it. temp file	
	Load ( "/home/discomp/discomp/server/etc/clips-facts-constr.clp" );

	//load CLIPS rules
        Load ( (char*) Config::variables.value("ClipsRules").toStdString().c_str() );

	// reset clips enviroment
        Reset();

	// run rules
	Run(-1);

	//save and close dribble file
	DribbleOff();
	
	DecrementGCLocks();
}


/**
 * Convert XML data from nodes to the CLIPS format using XSLT trnsformations
 * and load it to the CLIPS enviroment
 */
void DiscompClips::loadMonitoringFacts(  )
{	
     //get all files from monitoring directory

     QDir dir( Config::variables.value("MonitorDataDirectory") );
     dir.setFilter( QDir::Dirs | QDir::NoSymLinks );
     dir.setSorting(QDir::Name);
     QFileInfoList list = dir.entryInfoList();
     for (int i = 0; i < list.size(); ++i) 
     {
	     QFileInfo fileInfo = list.at(i);
	     QString node_mon_path = fileInfo.filePath();
	     if ( fileInfo.fileName() == ".." || fileInfo.fileName() == "." ) {
		continue;
	     }


	     //collect period stats for node
	     QString file_content1;
	     File::read ( node_mon_path + "/0/0.xml", file_content1);
	     QString file_content2;
	     File::read ( node_mon_path + "/1/0.xml", file_content2);
	     QString file_content3;
	     File::read ( node_mon_path + "/2/0.xml", file_content3);

	     File::save ( node_mon_path + "/clips-period.xml", 
	     			"<statistics><now>"+file_content1+"</now>" + 
	     			"<five>"+file_content2+"</five>" + 
	     			"<ten>"+file_content3+"</ten>" +
	                        "</statistics>");


             //log paths
//    logMessage( LOG_DEBUG, node_mon_path + "/clips-period.xml" );
//    logMessage( LOG_DEBUG, Config::variables.value("MonitorClipsXSLT") );
//    logMessage( LOG_DEBUG, node_mon_path + "/clips-facts.clp");

	     //open output file
	     QFile outputFile( node_mon_path + "/clips-facts.clp") ;
	     if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
	              return;


             // translate using XSLT
	     QXmlQuery query(QXmlQuery::XSLT20);
	     query.setFocus(QUrl(node_mon_path + "/clips-period.xml" ));
	     query.setQuery(QUrl(Config::variables.value("MonitorClipsXSLT")));
	     query.evaluateTo(&outputFile);


             //close file
	     outputFile.close();


	     logMessage(LOG_DEBUG, "Load facts from: " + node_mon_path + "/clips-facts.clp");


             //load to clips
	     Load ( (char*) QString( node_mon_path + "/clips-facts.clp").toStdString().c_str( ) );
	 }; 
}



/**
 * Private function for write log messages from this class
 * @param log_level - log level from lib/log.h
 * @param message - log message 
 */
void DiscompClips::logMessage ( int log_level, const QString& message ) 
{
	Log::addMessage (log_level, "DiscompClipsAPI", message);
};


