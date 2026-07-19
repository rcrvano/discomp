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


class monitorComponent extends Components {

	function getComponentData()
	{
		switch ( $this->action ) {
			case 'get_current_data' :
				$this->getCurrentData ( $_REQUEST['node_id'] );
				break;

			case 'node_monitor' :
				$this->interface->display("node.tpl");
				break;
			case 'node_cpu_monitor' :
				$this->interface->display("node_cpu.tpl");
				break;
			case 'node_network_monitor' :
				$this->interface->display("node_network.tpl");
				break;
			case 'node_filesystem_monitor' :
				$this->interface->display("node_filesystem.tpl");
				break;
			case 'node_temperature_monitor' :
				$this->interface->display("node_temperature.tpl");
				break;





			default :
				$this->showConnectedNodesList();
				break;
		};
	}

	function getCurrentData($node_id)
	{

		$xml = $this->communicator->getMonitoringData($node_id,1);
		$sxml = simplexml_load_string($xml);
		echo json_encode($sxml);

		/*	
		$dom = new DOMDocument();
		$dom->loadXML($xml);

		$stats = array(
			"cpu" => $dom->getElementsByTagName("core")->item(0)->getAttribute("used_p"),//$cpu,
			"mem_free" => $dom->getElementsByTagName("memory")->item(0)->getAttribute("free_p"),
			"temp" => $dom->getElementsByTagName("cputemp")->item(0)->getAttribute("core0")
		);
		echo json_encode($stats);
		//print_r($stats);
		*/
	}



	function showConnectedNodesList ()
	{
		$this->interface->assign ("NODES_LIST", $this->communicator->getNodesList());
		$this->interface->display("list.tpl");
	}

	function showNodeInfo ( $node_id ) 
	{
		$this->interface->assign("NODE_INFO", $this->communicator->getNodeInfo($node_id) );
		$this->interface->display("node_info.tpl");
	}

	function showInstallModule ( $node_id ) 
	{
		include ( "classes/package.php" );
		$available_modules = DiscompPackage::getAvailableModulesList();
		$node_info = $this->communicator->getNodeInfo($node_id);
		
		$modules_list = array();
		foreach ( $available_modules as $av_key=>$av_module ) {
			$found = false;
			foreach ( $node_info['modules_list'] as $node_key => $node_module ) {
				$node_module_name = split (":", $node_module);
				if ( $av_module == $node_module_name[0] ) {
					$found = true;
				};
			};
			$module_info = split('\.', $av_module);
			$modules_list[] = array ("name"=>$module_info[1], "package"=>$module_info[0], "installed" => $found ? 1 :0 );
		};

		$this->interface->assign("MODULES_LIST", $modules_list );
		$this->interface->assign("NODE_INFO", $node_info );
		$this->interface->display("install_module.tpl");
	}
	function installModule ( $node_id, $module ) {
		$this->communicator->installModule($node_id, $module);
		$this->interface->display("install_module_started.tpl");
	}
}

?>
