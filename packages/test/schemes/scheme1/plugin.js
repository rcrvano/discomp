/* 
 * This is a example plugin file which 
 * allow to see some of DiscompAPI functionality
 */

// this is a comment 1
/* this is a comment 2 */

DiscompAPI.logMessage ("You should see this message in the log when on process init stage");

var GLOBAL_COUNTER;
Init ();

//---------------------------------- Init -------------------------------------//
/* 
 * This function init some variables.
 * And shows some DiscompAPI methods 
 */
function Init ()
{
	DiscompAPI.logMessage ( "Process dir is: "    + DiscompAPI.getProcessDir() );
	DiscompAPI.logMessage ( "Current stage num: " + DiscompAPI.getCurrentStage() );
	printProcessInputParameters ( );


	GLOBAL_COUNTER=0;
};

//print process input parameters
function printProcessInputParameters ( )
{
	DiscompAPI.logMessage ( "Process input parameters: ");
/*
	//Qt.QDir();
	var input_dir   = new QDir (DiscompAPI.getProcessDir() + "/parameters/input" );
	var input_files = input_dir.entryList('*');
	for (var i = 0; i < input_files.length; ++i) {
		DiscompAPI.logMessage ( "   " + input_files[i]);
	};
*/
}



//print process ouput parameters
function printProcessOutputParameters ( )
{
/*
	DiscompAPI.logMessage ( "Process ouptut parameters: ");

	var output_dir   = new QDir (DiscompAPI.processDir() + "/parameters/output" );
	var output_files = output_dir.entryList('*');
	for (var i = 0; i < output_files.length; ++i) {
		DiscompAPI.logMessage ( "   " + output_files[i]);
	};
*/
}

function clearOutputParametersDir ( ) 
{
/*
	var output_dir   = new QDir (DiscompAPI.processDir() + "/parameters/output" );
	var output_files = output_dir.entryList('*');
	for (var i = 0; i < output_files.length; ++i) {
		//delete file.
		DiscompAPI.logMessage ( "Delete ouput parameter: " + output_files[i]);
		File.unlink ( DiscompAPI.processDir() + "/parameters/output/" + output_files[i] );
	};
*/
}


 
//---------------------------------- Stage 1  ---------------------------------//
function stage1Start ( ) 
{
	DiscompAPI.logMessage ( "Stage 1 started");

	//and here will be called undefined function or 
	//method for show how interpreter check errors
	DiscompAPI.undefinedMethod();
};

function stage1Finish ( ) 
{
	DiscompAPI.logMessage ( "Stage 0 finished");

	//print output parameters to the log
	printProcessOutputParameters ();
};

function moduleStart ( module_id ) 
{
	DiscompAPI.logMessage ( "Started module " + module_id + ". Current stage is: " + DiscompAPI.getCurrentStage() );
}

function moduleFinish (module_id)
{
	DiscompAPI.logMessage ( "Finished module " + module_id );

	//if 500 module finish, then module 501 should be stoped
	if ( module_id == "decompose" ) {
	//	DiscompAPI.gotoStage(0);
		//DiscompAPI.stopModule ( "test2" );
	};
}

function moduleStop ( module_id ) 
{
	DiscompAPI.logMessage ( "Module stoped " + module_id );
}



//---------------------------------- Stage 2  ---------------------------------//
function listStageStart () 
{
	//for test.
	for ( i = 0; i < 10000 ; i ++ ) { };

	//and here we can set/get some parameter
}

function listStageStop  () {
	DiscompAPI.logMessage ( "List stage stoped" ) ;
}


/* This function will be called each 100 msec at the first stage
 */
function listStageCheckResults ( )
{
	DiscompAPI.logMessage ( "Periodically call" );

};


function listModuleStart ()  { 
	DiscompAPI.logMessage ( "List module started" ); 
};
function listModuleStop  ()  { 
	DiscompAPI.logMessage ( "List module stoped" ); 
};
function listModuleFinish()  
{ 
	DiscompAPI.logMessage ( "List module finished" ); 
	
	DiscompAPI.logMessage ( "Checking list results (searching TRUE in results)");

	var res_parameter_size =  DiscompAPI.getListParameterSize ( 3 );
	DiscompAPI.logMessage ( "List size: " + res_parameter_size );

	for ( i=1; i <= res_parameter_size; i++ ) 
	{
		var res_list_el = DiscompAPI.getListParameterValue (3, i);
		if ( res_list_el.match(/TRUE/) ) {
			DiscompAPI.logMessage ( "Found TRUE list element: res" + i + "" );
			DiscompAPI.logMessage ( "Stop module execution (module_id:" + 503 + ")"  );

			DiscompAPI.stopModule ( 503 );

			DiscompAPI.gotoStage(0);
			return;
		};
	};
	DiscompAPI.logMessage ( "TRUE not found" );

	//and here we should have a access to the statistics.

};
