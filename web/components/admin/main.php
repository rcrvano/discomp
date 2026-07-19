<?php
// +--------------------------------------------------------------------+ 
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2009 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


class adminComponent extends Components {

	function getComponentData()
	{
		switch ( $this->action ) {
			default :
				$this->showConnectedNodesList();
				break;
		};
	}

	function showConnectedNodesList ()
	{
		$this->interface->assign ("NODES_LIST", $this->communicator->getNodesList());
		$this->interface->display("nodes_list.tpl");
		print_r($this->communicator->startProcess("test"));
	}
}

/*
main();
function main () 
{
//	if ( !$communicator->connected ) {
//		echo "Couldn't connect to remote host. ".$communicator->errstr." (Error: ". $communicator->errno . ")";
//		return;
//	};
	if ( !$communicator->isAuthorized() ) {
		echo "User isn't authorized";
		return;
	};
	print_r($communicator->startProcess("test"));
//	print_r($communicator->stopProcess("test"));
	print_r($communicator->getNodeInfo("0"));
	echo $communicator->checkAuthorization();
	echo $communicator->createNewUser("ddddfff", "4444fffffffffff");

};
*/

?>
