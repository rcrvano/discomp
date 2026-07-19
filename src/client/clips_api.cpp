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
#include "database.h"
#include "lib/log.h"

//#include "../lib/clips/clips.h"

#include <stdio.h> 
//#include "../lib/clips/setup.h"
#include "../lib/clips/clips.h"
//#include "../lib/clips/cmptblty.h"

#include <iostream>

/**
  * Constructor
  */
DiscompClips::DiscompClips(QObject *parent) 
{
	Log::addMessage(LOG_COMMON, "monitor", "Clips object constructed");

}

/**
  * Destructor
  */
DiscompClips::~DiscompClips()
{
	Log::addMessage(LOG_COMMON, "monitor", "Clips object destructed");
}


void DiscompClips::processClipsFile ( )
{
        Log::addMessage(LOG_COMMON, "monitor", "Processing file.clp");

        std::cout << "Test msg";
        qDebug() << "Test";

	InitializeEnvironment();
   
	IncrementGCLocks();

        Load ("facts-temp.clp");
        Load ("facts-constr.clp");
        Reset();
        Load("file.clp");


	
	Run(-1);


/*
	DATA_OBJECT ob;
	GetFactList(&ob, NULL);
	str = (char*)GetValue(ob);
	*/
//	std::cout << " ------------------ FACTS ----------------- " << std::endl;
//	Facts("wdisplay", NULL, -1,-1,-1);
//	std::cout << str;
	SaveFacts("tmp.txt", LOCAL_SAVE, NULL);
/*  
	Load("file.clp");
	Reset();
	Run(-1);
 */
	DecrementGCLocks();
}
