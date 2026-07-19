// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include "server.h"
#include "lib/log.h"

/**
 * @class DiscompServer
 * @brief Discomp Server (Core class)
 *
 *
 * This is a base class in the Discomp server which control calculating
 * processes, nodes, processes queue, anc actions from communicator.
 *
 * TcpServer based on the QTcpServer class and bind 5711 port (by default)
 * when server start (see main.cpp). All nodes works in the different threads 
 * (threaded server) and server works with its via public classes or by
 * signal-slots architecture. Discomp communicator can tell server connect
 * or disconnect some nodes.
 *
 * Discomp Server support multiple processes in queue. Now priority of 
 * this processes based on the order of processes in queue and process which 
 * start first will have more priority. Signals for start and stop process received 
 * from discomp communicator. 
 *
 * The base function of this class is control process queue and give jobs for free
 * nodes. Process queue loop checks processes if they have a job for some not busy node,
 * and if this node found - tryed to start module on them.
 * After start Discomp server works as intermediate between DiscompProcess class and 
 * DiscompNode class throught signal-slots architecture. Node call some signal, 
 * this class receive this signal, check it and call some method in process class if 
 * needed. See methods with _slot suffix in this class
 */

/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/**
 * Constructor 
 * */
DiscompServer::DiscompServer(QObject *parent)
    : QTcpServer(parent)
{
	process_counter = 0;
	this->all_nodes_free = true; //все узлы свободны, хоть их и нет пока
	logMessage (LOG_DEBUG, "Constructed DiscompServer Class");



//	m_discomp_monitor = new DiscompMonitor(this);
//	m_discomp_monitor->start();


	processes.clear();
}

/**
 * Class destructor 
 * */
DiscompServer::~DiscompServer()
{
	logMessage ( LOG_DEBUG, "Discomp Server Class destructor" );

	//delete all processes
	logMessage (LOG_COMMON, "Stoping all processes");
	QVector <DiscompProcessPrivate*>::iterator process_it = processes.begin();
	while ( process_it != processes.end() ) {
		logMessage (LOG_COMMON, "  Process: " + (*process_it)->get_name());
		delete *process_it;
		++process_it;
	};
	
	//delete all nodes
	logMessage (LOG_COMMON, "Disconnecting all nodes");
	QVector<DiscompNode*>::iterator node_it = nodes.begin();
	while ( node_it != nodes.end() ) {
		if ( (*node_it) != NULL ) 
			logMessage (LOG_COMMON, QString("  Node: id:%1, name:%2, IP:%3").
			            arg((*node_it)->id).arg((*node_it)->name).arg((*node_it)->ip));
			(*node_it)->quit();

		++node_it;
	};
	
	logMessage (LOG_DEBUG, "DiscompServer Class destructed");
};






/***********************************************************************/
/*-------------------Connect & disconnect remote node -----------------*/
/***********************************************************************/

/**
 * New node trying to connect to the server. 
 * Create a socket and start new thread
 * @param socket_descriptor - Socket descriptor for incoming connection
 */
void DiscompServer::incomingConnection (int socket_descriptor)
{
	logMessage ( LOG_DEBUG, "New node trying to connect." );
	
	//create new thread
	DiscompNode *newNodeThread  = new DiscompNode(socket_descriptor);
	newNodeThread->id = (int)nodes.size();

	//run thread
	newNodeThread->start();

	//connect signal and slots
	connect( newNodeThread, SIGNAL ( finished() ), 
	         newNodeThread, SLOT   ( deleteLater() ) );
	
	connect( newNodeThread, SIGNAL ( nodeConnected_signal(int) ),
	         this,          SLOT   ( nodeConnected_slot(int) ) );

	connect( newNodeThread, SIGNAL ( nodeDisconnected_signal(int) ),
	         this,          SLOT   ( nodeDisconnected_slot(int) ) );


	nodes.push_back( newNodeThread );
}



/**
 * New node has been connected & authorized. 
 * @param node_id - node indentification number
 * */
void DiscompServer::nodeConnected_slot( int node_id )
{
	//disdisconnect previously disconnected signal/slots
	disconnect ( nodes[node_id],  SIGNAL (prepareModuleSuccess_signal (int, const QString &)),
	             this,            SLOT   (prepareModuleSuccess_slot   (int, const QString &)) );
	disconnect ( nodes[node_id],  SIGNAL (prepareModuleFailed_signal  (int, const QString &, int, const QString &)),
	             this,            SLOT   (prepareModuleFailed_slot    (int, const QString &, int, const QString &)) );

	disconnect ( nodes[node_id],  SIGNAL (startModuleSuccess_signal   (int, const QString &)),
	             this,            SLOT   (startModuleSuccess_slot     (int, const QString &)) );
	disconnect ( nodes[node_id],  SIGNAL (startModuleFailed_signal    (int, const QString &, int, const QString &)),
	             this,            SLOT   (startModuleFailed_slot      (int, const QString &, int, const QString &)) );

	disconnect ( nodes[node_id],  SIGNAL (stopModuleSuccess_signal    (int, const QString &)),
	             this,            SLOT   (stopModuleSuccess_slot      (int, const QString &)) );
	disconnect ( nodes[node_id],  SIGNAL (stopModuleFailed_signal     (int, const QString &, const QString &)),
	             this,            SLOT   (stopModuleFailed_slot       (int, const QString &, const QString &)) );

	disconnect ( nodes[node_id],  SIGNAL (endModuleSuccess_signal     (int, const QString &)),
	             this,            SLOT   (endModuleSuccess_slot       (int, const QString &)) );
	disconnect ( nodes[node_id],  SIGNAL (endModuleFailed_signal      (int, const QString &, int, const QString &)),
	             this,            SLOT   (endModuleFailed_slot        (int, const QString &, int, const QString &)) );




	connect ( nodes[node_id],  SIGNAL (prepareModuleSuccess_signal (int, const QString &)),
	          this,            SLOT   (prepareModuleSuccess_slot   (int, const QString &)) );
	connect ( nodes[node_id],  SIGNAL (prepareModuleFailed_signal  (int, const QString &, int, const QString &)),
	          this,            SLOT   (prepareModuleFailed_slot    (int, const QString &, int, const QString &)) );

	connect ( nodes[node_id],  SIGNAL (startModuleSuccess_signal   (int, const QString &)),
	          this,            SLOT   (startModuleSuccess_slot     (int, const QString &)) );
	connect ( nodes[node_id],  SIGNAL (startModuleFailed_signal    (int, const QString &, int, const QString &)),
	          this,            SLOT   (startModuleFailed_slot      (int, const QString &, int, const QString &)) );

	connect ( nodes[node_id],  SIGNAL (stopModuleSuccess_signal    (int, const QString &)),
	          this,            SLOT   (stopModuleSuccess_slot      (int, const QString &)) );
	connect ( nodes[node_id],  SIGNAL (stopModuleFailed_signal     (int, const QString &, const QString &)),
	          this,            SLOT   (stopModuleFailed_slot       (int, const QString &, const QString &)) );

	connect ( nodes[node_id],  SIGNAL (endModuleSuccess_signal     (int, const QString &)),
	          this,            SLOT   (endModuleSuccess_slot       (int, const QString &)) );
	connect ( nodes[node_id],  SIGNAL (endModuleFailed_signal      (int, const QString &, int, const QString &)),
	          this,            SLOT   (endModuleFailed_slot        (int, const QString &, int, const QString &)) );


	//monitoring data
//	connect( nodes[node_id],  SIGNAL ( processMonitoringData_signal       (int, const QString &, const QString &) ),
//	         discomp_monitor, SLOT   ( processMonitoringDataFromNode_slot (int, const QString &, const QString &) ) );



	logMessage ( LOG_COMMON, QString("New node connected [id:%1, name:%2, IP:%3]").
	             arg(nodes[node_id]->id).arg(nodes[node_id]->name).arg(nodes[node_id]->ip));
	
	logMessage ( LOG_COMMON, QString("Log message for this node storied in: %1").arg(nodes[node_id]->log_file) ) ;
	
	//emmit signal that we connect new node (for frontend)
	emit nodeConnected_signal ( node_id );

	checkProcessQueue();
};


/**
 * Connection with remote node is closed 
 * @param node_id - node identifiction number
 * */
void DiscompServer::nodeDisconnected_slot( int node_id )
{
	logMessage ( LOG_COMMON, QString("Node [id:%1, name:%2, IP:%3] has been disconnected").
	             arg(nodes[node_id]->id).arg(nodes[node_id]->name).arg(nodes[node_id]->ip));

	//tell to process that this node has been disconnected and tasks for this node should be cleared
	for ( int process_num=0; process_num<(int)processes.size(); process_num++ ) 
	{
		processes[process_num]->clearJobOnNode( nodes[node_id] );
	}

	//emmit signal (can be connected by frontend)
	emit nodeDisconnected_signal ( node_id, nodes[node_id]->name, nodes[node_id]->ip );

	//stop thread
	nodes[node_id]->quit();
	nodes[node_id] = NULL;
};

/**
 * Connect remote node to server. "Backconnect" mechanizm
 * @param IP   - node IP address
 * @param port - node port (default for backkonnect 5713)
 */
void DiscompServer::connectNode ( const QString& IP, int port )
{
	logMessage(LOG_DEBUG, QString("Send request to remote node[IP:%1, port:%2] for connect to us").arg(IP).arg(port));
	
	QTcpSocket* new_node = new QTcpSocket();
	new_node->connectToHost( IP, port );
}

/**
 * Discomnnect node from server by id
 * @param node_id - node identification number
 */
void DiscompServer::disconnectNode ( int node_id ) 
{
	logMessage( LOG_COMMON, QString("Signal for disconnect node [id:%1, name:%2, IP:%3]").
	            arg(node_id).arg(nodes[node_id]->name).arg(nodes[node_id]->ip));

	if ( !nodes[node_id] ) {
		logMessage(LOG_WARNING, QString("  This node already disconnected"));
		return;
	};

	//tell to process that this node has been disconnected and tasks for this node should be cleared
	for ( int process_num=0; process_num<(int)processes.size(); process_num++ ) 
	{
		logMessage(LOG_DEBUG,"Process?");
		processes[process_num]->clearJobOnNode( nodes[node_id] );
	}

	//emmit signal (can be connected by frontend)
	emit nodeDisconnected_signal ( node_id, nodes[node_id]->name, nodes[node_id]->ip );

	//stop thread
	nodes[node_id]->quit();
	nodes[node_id] = NULL;
};











/***********************************************************************/
/*----------------------  Calculating process  ------------------------*/
/***********************************************************************/

/**
 * Public method for add new calculating process to the processes queue
 * @param process_name - process name (string)
 * @return - status of starting
 * */
int DiscompServer::startProcess( const QString& process_name, const QString &user_name )
{
	
	logMessage ( LOG_COMMON, "Starting new process: " + process_name );


	//check maybe we already have same c calculation process which not done
	QVector <DiscompProcessPrivate*>::iterator process_it = processes.begin();
	while ( process_it != processes.end() ) {
		if ( (*process_it)->get_name() == process_name ) {
			logMessage ( LOG_WARNING, "  Found active process with same name. Two process with same name couldn't be started");
			return -1;
		};
		++process_it;
	};

	QString process_type = DiscompProcessPrivate::getProcessType( process_name );
	DiscompProcessPrivate *new_process_class;
	//detecting process type.
	if ( process_type == "xml" ) {
		logMessage ( LOG_COMMON, "Process type: XML" );
		new_process_class = new DiscompProcess();
	} else if ( process_type == "js" ) {
		logMessage ( LOG_COMMON, "Process type: JavaScript" );
		new_process_class = new DiscompProcessJS();
	} else {
		logMessage ( LOG_ERROR, "Couldn't detect process type (process declaration file not found)");
		return -3;
	};


	new_process_class->set_id( process_counter++ );
	new_process_class->set_name ( process_name );


	if ( new_process_class->Init( process_name, user_name ) ) {
		processes.push_back ( new_process_class );
	} else {
		new_process_class->logMessage(LOG_COMMON, "Couldn't start process");
		logMessage(LOG_COMMON, "Couldn't start process:" + process_name);
		delete new_process_class;
		return -2;
	};

		
	connect ( new_process_class,  SIGNAL (processFinished_signal (int)),
        	  this,               SLOT   (processFinished_slot   (int)) );
	connect ( new_process_class,  SIGNAL (processFreeNode_signal (int, int, const QString &)),
	          this,               SLOT   (processFreeNode_slot   (int, int, const QString &)) );
	connect ( new_process_class,  SIGNAL (processStopModuleExecutionOnNode_signal (int, const QString &)),
	          this,               SLOT   (stopModuleOnNode_slot   (int, const QString &)) );

	if (process_type == "js") {
		connect ( new_process_class, SIGNAL (newModulesAvailable_signal ()),
				  this,              SLOT   (newModulesAvailable_slot ()) );
	}

	checkProcessQueue();


	return new_process_class->get_id();
};


/**
 * Public method for add new calculating process to the processes queue
 * @param process_name - process name (string)
 * @return - status of starting
 * */
int DiscompServer::startSingleProcess( const QString& process_dir, const QString &user_name )
{
	
	logMessage ( LOG_COMMON, "Starting new task from the dir: " + process_dir );

	//check maybe we already have same c calculation process which not done
	QVector <DiscompProcessPrivate*>::iterator process_it = processes.begin();
	while ( process_it != processes.end() ) {
		if ( (*process_it)->get_name() == process_dir ) {
			logMessage ( LOG_WARNING, "  Found active process with same name. Two process with same name couldn't be started" );
			return -1;
		};
		++process_it;
	};


	DiscompProcess* new_process_class = new DiscompProcess();
	new_process_class->id       = process_counter++;
	new_process_class->name     = process_dir;
		
	connect ( new_process_class,  SIGNAL (processFinished_signal (int)),
	          this,               SLOT   (processFinished_slot   (int)) );

	connect ( new_process_class,  SIGNAL (processFreeNode_signal (int, int, const QString &)),
	          this,               SLOT   (processFreeNode_slot   (int, int, const QString &)) );

	connect ( new_process_class,  SIGNAL (processStopModuleExecutionOnNode_signal (int, const QString &)),
	          this,               SLOT   (stopModuleOnNode_slot   (int, const QString &)) );

	if ( new_process_class->InitSingleTask( process_dir, user_name ) ) {
		processes.push_back ( new_process_class );
	} else {
		new_process_class->logMessage(LOG_COMMON, "Couldn't start single task");
		logMessage(LOG_COMMON, "Couldn't start single task: " + process_dir);
		delete new_process_class;
		return -2;
	};

	checkProcessQueue();

	return new_process_class->id;
};


/**
 * Stop and delete calculating process from process queue
 * @param process_id - process identificator
 */
void DiscompServer::stopProcess ( int process_id )
{
	int process_num = getProcessNumById ( process_id );
	if ( process_num == -1 ) { return; }
	
	logMessage ( LOG_COMMON, QString("Stop process: %1").arg(processes[process_num]->get_name()) );

	processes[process_num]->logMessage ( LOG_COMMON, QString("Received request for stop process") );

	//clear all nodes TODO!!!!!!!!!!!!!!!!!
	// TODO!!!! Stop all calculations.
	// free nodes, remove some files, etc

	delete processes[process_num];
	processes.remove(process_num);
	
	checkProcessQueue();
};




/**
 * Main processes queu loop. This method called when some event occured.
 * For ex. new node connected, node finished calculating of some module, or
 * new processes added to the queue. 
 *
 * Algorith of queue loop now based on the processes prority.
 * Process which was pushed to the queue first have more priority.
 *
 * Queue loop first trying to detect free node. If it's exists
 * Main processes queue loop. It called when some event occur.
 * First we search free nodes. If node found we inquiry all 
 * processes for check if they have job for this node. 
 * */
void DiscompServer::checkProcessQueue()
{
	logMessage ( LOG_DEBUG, "Check processes queue" );

	bool global_task_found_flag = false;
	//check all processes
	for ( int process_num=0; process_num<(int)processes.size(); process_num++ ) 
	{
		
		//if current process completed, then destroy it.
		if ( processes[process_num]->get_completed() ) {
			//don't allow infinite loop (precaution)
			processes[process_num]->set_completed ( false ); 
			//destruct process class
			processFinished_slot( processes[process_num]->get_id() );
			//start from first element of processes array
			process_num=0; 
		};

		//flag for check if some module was executed in the processes queue
		bool task_found_flag = false;

		//search free node
		for ( int node_num=0; node_num < (int)nodes.size(); node_num++ )
		{
			if ( nodes[node_num] != NULL && !nodes[node_num]->busy ) 
			{
				//trying to give job for free node
				QString moduleName = processes[process_num]->giveJobToNode( nodes[node_num] );

				if ( moduleName != "" ) 
				{
					//node can execute module (moduleName) for this process
					
					//mark this node as busy
					nodes[node_num]->busy       = true;
					nodes[node_num]->process_id = processes[process_num]->get_id();
					nodes[node_num]->process_dir= processes[process_num]->get_absolute_process_dir();
					

					//success this node can execute module for this proccess
					//QString moduleName = nodes[node_num]->modules[nodes[node_num]->getModuleNumById(moduleId)]->name;
					
					QString addit_list_info = "";
					if ( nodes[node_num]->process_list_element_num ) {
						addit_list_info = QString(" [element: %1]").arg(nodes[node_num]->process_list_element_num);
					};

					//write log messages
					logMessage (LOG_COMMON, 
						QString("Process %2 (id:%1) will start module %3%6 on the node %5 (id:%4)").
							arg(processes[process_num]->get_id()).arg(processes[process_num]->get_name()).
							arg(moduleName).arg(nodes[node_num]->id).arg(nodes[node_num]->name).arg(addit_list_info) );

					processes[process_num]->logMessage (LOG_COMMON, 
						QString("[Queue] Module %3%4 INITIALIZING for start on the node %2 (id:%1)").
							arg(nodes[node_num]->id).arg(nodes[node_num]->name).arg(moduleName).arg(addit_list_info));

					//start procedure of module execution on the remote host
					nodes[node_num]->prepareModule( moduleName );

					task_found_flag = true;
					global_task_found_flag = true;
				};	
			};
		};

		if ( !task_found_flag ) {
			//TODO. this log will be (can and be) repeated many times
			processes[process_num]->logMessage ( LOG_COMMON, "[Queue] No more modules for execution on the current stage" );
		};

	};
	if ( !global_task_found_flag ) {
		logMessage ( LOG_COMMON, "Nothing for start. Currently tasks list is empty" );
	};
	this->all_nodes_free = true;
	for ( int node_num = 0; node_num < (int)nodes.size(); node_num++ )
	{
		if ( nodes[node_num] != NULL && nodes[node_num]->busy )
		{
			this->all_nodes_free = false;
		}
	}


};

/***********************************************************************/
/*-------------------  Calculating process slots  ---------------------*/
/***********************************************************************/

/************ Slots called by signals from DiscompNode *****************/
/**
 * Node successfully prepared for execution module. All parameters sent to 
 * the remote node. Now should be called start module execution
 * @param node_id - node identificator
 * @param module_name - module name
 */
void DiscompServer::prepareModuleSuccess_slot( int node_id, const QString &module_name )
{
	if ( !nodes[node_id] ) { 
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::prepareModuleSuccess_slot]").arg(node_id) );
		return; 
	}
	
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) { 
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::prepareModuleSuccess_slot]").
		                        arg(nodes[node_id]->process_id));
		return;
	}

	//write log message to the process.log file 
	if ( nodes[node_id]->process_list_element_num ) {
		processes[process_num]->logMessage ( LOG_COMMON, QString("[Queue] Executing module %1 (list_num:%2) on the node %4 (id:%3)").
				arg(module_name).arg(nodes[node_id]->process_list_element_num).arg(node_id).arg(nodes[node_id]->name));
	} else {
		processes[process_num]->logMessage ( LOG_COMMON, QString("[Queue] Executing module %1 on the node %3 (id:%2)").
				arg(module_name).arg(node_id).arg(nodes[node_id]->name));
	}

	//start module
	nodes[node_id]->startModule(module_name);
};

/**
 * Prepare module failed. For ex. some input parameters isn't defined
 * @param node_id - node identificator
 * @param module_name - module name
 * @param error_num - error identificator
 * @param error_msg - error msg
 */
void DiscompServer::prepareModuleFailed_slot( int node_id, const QString &module_name, int error_num, const QString &error_msg )
{
	if ( !nodes[node_id] ) { 
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::prepareModuleFailed_slot]").arg(node_id) );
		return; 
	}

	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) { 
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::prepareModuleFailed_slot]").
		                        arg(nodes[node_id]->process_id));
		return;
	}

	QString addit_list_info = "";
	if ( nodes[node_id]->process_list_element_num ) {
		addit_list_info = QString(" [el:%1]").arg(nodes[node_id]->process_list_element_num);
	};

	//clear module execution state.
	processes[process_num]->clearJobOnNode( nodes[node_id] );

	//todo. need advanced checks that  nodes[node_id]->modules is defined here
	if ( error_num == 6 ) { //md5 is differ. 
		processes[process_num]->logMessage ( LOG_WARNING, QString("[Queue] Module %3 could NOT be STARTED on the node %2 (id:%1) because module files on the server and on the node sides are differ. If node support auto-update, then this module will be updated.").
			arg(node_id).arg(nodes[node_id]->name).arg(module_name) );
		processes[process_num]->logMessage ( LOG_DEBUG, QString("Reason: %1").arg(error_msg) );

		//mark this node as free
		nodes[node_id]->clearNodeState();
		return;
	};

	//write log message to the process.log file 
	processes[process_num]->logMessage ( LOG_WARNING, QString("[Queue] Module %1%5 could NOT be STARTED on the node %3 (id:%2). Reason: %4").
			arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(error_msg).arg(addit_list_info) );
	
	//clear module execution state. it's mean that this failed module
	//can be executed on the another node
	processes[process_num]->clearJobOnNode( nodes[node_id] );

	//add this module to failed modules list. it's mean that for this process
	//module will be never executed on the node again.
	nodes[node_id]->addFailedModule (processes[process_num]->get_id(), module_name, error_num, error_msg);

	//mark this node as free
	nodes[node_id]->clearNodeState();

	//check processes queue loop
	checkProcessQueue();
};

/**
 * Module successfully started on the remote node
 * @param node_id - node identificator
 * @param module_name - module name
 */
void DiscompServer::startModuleSuccess_slot( int node_id, const QString &module_name )
{
	if ( !nodes[node_id] ) {
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::startModuleSuccess_slot]").arg(node_id) );
		return; 
	}
	
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) {
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::startModuleSuccess_slot]").
		                        arg(nodes[node_id]->process_id));
		return;
	};

	QString addit_list_info = "";
	if ( nodes[node_id]->process_list_element_num ) {
		addit_list_info = QString(" [element: %1]").arg(nodes[node_id]->process_list_element_num);
	};

	//write log message to the process.log file 
	processes[process_num]->logMessage ( LOG_COMMON, QString("[Queue] Module %1%4 successfully STARTED on the node %3 (id:%2)").
					arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(addit_list_info) );
};

/**
 * Module is not started on the remote node. Some problems occcur
 * @param node_id - node identificator
 * @param module_name - module name
 * @param error_num - error identificator
 * @param error_msg - error msg
 */
void DiscompServer::startModuleFailed_slot( int node_id, const QString &module_name, int error_num, const QString & error_msg )
{
	if ( !nodes[node_id] ) {
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::startModuleFailed_slot]").arg(node_id) );
		return; 
	}
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) { 
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::startModuleFailed_slot]").
		                        arg(nodes[node_id]->process_id));
		return;
	}
	
	QString addit_list_info = "";
	if ( nodes[node_id]->process_list_element_num ) {
		addit_list_info = QString(" [element: %1]").arg(nodes[node_id]->process_list_element_num);
	};

	//write log message to the process.log file 
	processes[process_num]->logMessage ( LOG_WARNING, QString("[Queue] Module %1%5 could NOT be STARTED on the node %3 (id:%2). Reason: %4").
			arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(error_msg).arg(addit_list_info) );

	//clear module execution state. it's mean that this failed module
	//can be executed on the another node
	processes[process_num]->clearJobOnNode( nodes[node_id] );

	//add this module to failed modules list. it's mean that for this process
	//module will be never executed on the node again.
	nodes[node_id]->addFailedModule (processes[process_num]->get_id(), module_name, error_num, error_msg);

	//mark this node as free
	nodes[node_id]->clearNodeState();

	//check processes queue loop
	checkProcessQueue();
};

/**
 * Module successfully finished on the remote host
 * @param node_id - node identificator
 * @param module_name - module name
 */
void DiscompServer::endModuleSuccess_slot ( int node_id, const QString &module_name )
{
	if ( !nodes[node_id] ) {
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::endModuleSuccess_slot]").arg(node_id) );
		return; 
	}
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) {
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::endModuleSuccess_slot]").
		                        arg(nodes[node_id]->process_id));
		return;
	}

	QString addit_list_info = "";
	if ( nodes[node_id]->process_list_element_num ) {
		addit_list_info = QString(" [element: %1]").arg(nodes[node_id]->process_list_element_num);
	};

	//write log message to the process.log file 
	processes[process_num]->logMessage ( LOG_COMMON, QString("[Queue] Module %1%4 successfully FINISHED on the node %3 (id:%2)").
					arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(addit_list_info) );

	//mark this module as done in the process modules list
	processes[process_num]->finishModuleExecutionOnNode( nodes[node_id], module_name );

	//mark node as free
	nodes[node_id]->clearNodeState();

	//and check process queue again
	checkProcessQueue();
};

/**
 * Module finished with some problems. For ex. module finished, 
 * but some output parameters is not defined.
 * @param node_id - node identificator
 * @param module_name - module name
 * @param error_num - error identificator
 * @param error_msg - error msg
 */
void DiscompServer::endModuleFailed_slot ( int node_id, const QString &module_name, int error_num, const QString & error_msg )
{
	if ( !nodes[node_id] ) {
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::endModuleFailed_slot]").arg(node_id) );
		return; 
	}
	
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) { 
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::endModuleFailed_slot]").
		                        arg(nodes[node_id]->process_id));
		return;
	}
	
	QString addit_list_info = "";
	if ( nodes[node_id]->process_list_element_num ) {
		addit_list_info = QString(" [element: %1]").arg(nodes[node_id]->process_list_element_num);
	};

	//write log message to the process.log file 
	processes[process_num]->logMessage ( LOG_WARNING, QString("[Queue] Module %1%5 NOT FINISHED on the node %3 (id:%2). Reason: %4").
			arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(error_msg).arg(addit_list_info) );

	if ( error_num == 0 ) { //output parameters not defined or other problems
		//clear module state in the process modules list
		processes[process_num]->clearJobOnNode( nodes[node_id] );
		//mark this module as failed
		nodes[node_id]->addFailedModule (processes[process_num]->get_id(), module_name, 0, error_msg);

	} else if ( error_num == 2 ) { //module max time is reached
		processes[process_num]->moduleMaxTimeReached(nodes[node_id], module_name);
	};

	//mark module as free
	nodes[node_id]->clearNodeState();

	//check process queue
	checkProcessQueue();
}

/**
 * Module successfully stoped on the remote host
 * @param node_id - node identificator
 * @param module_name - module name
 */
void DiscompServer::stopModuleSuccess_slot( int node_id, const QString &module_name )
{
	if ( !nodes[node_id] ) { 
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::stopModuleSuccess_slot]").arg(node_id) );
		return; 
	}
	
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) { 
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::stopModuleSuccess_slot]").
		                        arg(nodes[node_id]->process_id));
		nodes[node_id]->clearNodeState();
	} else {
		QString addit_list_info = "";
		if ( nodes[node_id]->process_list_element_num ) {
			addit_list_info = QString(" [element: %1]").arg(nodes[node_id]->process_list_element_num);
		};

		//write log message to the process.log file 
		processes[process_num]->logMessage ( LOG_COMMON, QString("[Queue] Module %1%4 successfully STOPED on the node %3 (id:%2)").
					arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(addit_list_info) );

//TODO: need to call clear Job on node. not finish
		//clear module execution
		processes[process_num]->finishModuleExecutionOnNode( nodes[node_id], module_name );
		nodes[node_id]->clearNodeState();
	};

	//check process queue
	checkProcessQueue();
};

/*
 * Module stoping failed. Some error occur.
 * @param node_id - node identificator
 * @param module_name - module name
 * @param msg - error msg
 */
void DiscompServer::stopModuleFailed_slot( int node_id, const QString &module_name, const QString& error_msg )
{
	if ( !nodes[node_id] ) {
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::stopModuleFailed_slot]").arg(node_id) );
		return; 
	};
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) {
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::stopModuleFailed_slot]").
		                        arg(nodes[node_id]->process_id));

		//clear state
		nodes[node_id]->clearNodeState();
	} else {
		QString addit_list_info = "";
		if ( nodes[node_id]->process_list_element_num ) {
			addit_list_info = QString(" [element: %1]").arg(nodes[node_id]->process_list_element_num);
		};

		//write log message to the process.log file 
		processes[process_num]->logMessage ( LOG_WARNING, QString("[Queue] Module %1%5 NOT STOPED on the node %3 (id:%2). Reason: %4").
				arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(error_msg).arg(addit_list_info) );

//TODO: need to call clear Job on node. not finish
		//clear state
		nodes[node_id]->clearNodeState();
		processes[process_num]->finishModuleExecutionOnNode( nodes[node_id], module_name );
	};
	//check process queue
	checkProcessQueue();
}


/************ Slots called by signals from DiscompProcess **************/
/**
 * All process stages was finished. Destruct this process
 * @param process_id - process identificator
 */
void DiscompServer::processFinished_slot ( int process_id ) 
{	
	//remove it from processes list
	int process_num = getProcessNumById ( process_id );
	if ( process_num == -1 ) {
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::processFinished_slot]").arg(process_id));
		return;
	};

	logMessage (LOG_COMMON, QString("Process %1 (id:%2) completed").arg(processes[process_num]->get_name()).arg(process_id));

	//destruct process class
	delete processes[process_num];
	processes.remove(process_num);

	//check process queue
	checkProcessQueue();
};

/**
 * This slot called when DiscompProcess is destructing.
 * When user send signal to stop calculating process called 
 * stopProcess method which delete DiscompProcess class and call destructor 
 * function. when process destructing it's chceck all nodes which he used
 * and send signal to free them
 * @param process_id - process identificator
 * @param node_id - node identificator
 * @param module_name - module name
 */
void DiscompServer::processFreeNode_slot ( int process_id, int node_id, const QString &module_name )
{
	if ( !nodes[node_id] ) {
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::processFreeNode_slot]").arg(node_id) );
		return; 
	}
	int process_num = getProcessNumById ( process_id );
	if ( process_num == -1 ) {
		if ( nodes[node_id]->busy ) {
			nodes[node_id]->stoping_module_execution = true;
			nodes[node_id]->stopModule(module_name);

			logMessage (LOG_COMMON, QString("Node %3 (%4) has been free.").arg(node_id).arg(nodes[node_id]->name) );
		};

		//logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::processFreeNode_slot]").arg(process_id));
	} else {
		if ( nodes[node_id]->busy ) {
			nodes[node_id]->stoping_module_execution = true;
			nodes[node_id]->stopModule(module_name);

			processes[process_num]->logMessage (LOG_COMMON, QString("[Queue] Node %1 (%2) was has been free.").arg(node_id).arg(nodes[node_id]->name) );
			logMessage (LOG_COMMON, QString("[Process: %1, id:%2] Node %3 (%4) has been free.").
					arg(process_id).arg(processes[process_num]->get_name()).arg(node_id).arg(nodes[node_id]->name) );
		};
	};

	//here we don't need start process queue loop.
	//if some module for this process will stoped, then it's will call 
	//by moduleStoped slot
};


/**
 * This slot called when DiscompProcess want to stop module
 * execution on some node. It can occur when result of calculation
 * is found and all modules should be stoped
 *
 * @param node_id  - node identificator
 * @param module_name - module name
 */
void DiscompServer::stopModuleOnNode_slot ( int node_id, const QString &module_name )
{
	if ( !nodes[node_id] ) {
		logMessage ( LOG_ERROR, QString("Node with id: %1 is NULL [DiscompServer::stopModuleOnNode_slot]").arg(node_id) );
		return; 
	}
	int process_num = getProcessNumById ( nodes[node_id]->process_id );
	if ( process_num == -1 ) {
		logMessage ( LOG_ERROR, QString("Couldn't get process_num by process_id: %1 [DiscompServer::stopModuleOnNode_slot]").
		                        arg(nodes[node_id]->process_id));
		return;
	}

	QString addit_list_info = "";
	if ( nodes[node_id]->process_list_element_num ) {
		addit_list_info = QString(" [element: %1]").arg(nodes[node_id]->process_list_element_num);
	};


	processes[process_num]->logMessage ( LOG_COMMON, QString("[Queue] Module %1%4 SHOULD BE STOPED on the node %3 (id:%2)").
			arg(module_name).arg(node_id).arg(nodes[node_id]->name).arg(addit_list_info) );

	//trying to stop module on the remote host
	nodes[node_id]->stoping_module_execution = true;
	nodes[node_id]->stopModule(module_name);
};

/**
 * This slot called when DiscompProcess has new modules to execute
 */
void DiscompServer::newModulesAvailable_slot () 
{
	/*sometimes there are dead situations when all events
	that can initilize new checkProcessQueue loop already took place.
	And after this some process, especially with js type wants to start new modules,
	server doesn't know it because it can't ask.

	so to avoid these situations let's call checkProcessQueue loop when
	when there are new tasks
	*/
	if (this->all_nodes_free) {
		this->checkProcessQueue();
	}
}

/* Method for clear failed modules list.
 * User can call it by RPC if he found a error in the module specification 
 * and want to restart it again
 */
void DiscompServer::clearFailedModulesForNodes () 
{
	for ( int node_num=0; node_num < (int)nodes.size(); node_num++ )
	{
		nodes[node_num]->clearFailedModules ();
	};
}

/***********************************************************************/
/*-----------------------  Private functions  -------------------------*/
/***********************************************************************/

/**
 * Get process num for processes array by process_id
 * @param process_id - process identificator
 * @return process number in the processes array
 */
int DiscompServer::getProcessNumById( int process_id )
{
	for ( int process_num=0; process_num < (int)processes.size(); process_num++ ) {
		if ( processes[process_num]->get_id() == process_id ) {
			return process_num;
		};
	};
//	logMessage ( LOG_CORE, QString("Process_id is : %1").arg(process_id)  );

	return -1;
};

/**
 * Get process identificator by process_name
 * @param process_name - process name
 * @return process identificator
 */
int DiscompServer::getProcessIdByName( const QString& process_name )
{
	QVector <DiscompProcessPrivate*>::iterator process_it = processes.begin();
	while ( process_it != processes.end() ) {
		if ( (*process_it)->get_name() == process_name ) {
			return (*process_it)->get_id();
		};
		++process_it;
	};

	return -1;
};

/**
 * Private function for write log messages from this class
 * @param log_level - log level from lib/log.h
 * @param message - log message 
 */
void DiscompServer::logMessage ( int log_level, const QString& message ) 
{
	Log::addMessage (log_level, "DiscompServer", "[Queue]" + message);
};

// vim: set fenc=utf-8 tabstop=8 :
