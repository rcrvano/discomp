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

require_once ( $DISCOMP_CONFIG['frontend_path']."/classes/package.php" );

class nodesComponent extends Components {

	function getComponentData()
	{
		switch ( $this->action ) {
			case 'get_node_info' :
				$this->showNodeInfo ( $_REQUEST['node_id'] );
				break;
			case 'update_node_sysinfo':
				$this->communicator->updateNodeSysInfo($_REQUEST['node_id']);
				sleep (3);
				$this->showNodeInfo ( $_REQUEST['node_id'] );
				break;
			case 'del_failed_module':
				$this->communicator->deleteFailedModule($_REQUEST['node_id'], $_REQUEST['module']);
				$this->showNodeInfo ( $_REQUEST['node_id'] );
				break;
			case 'show_install_module';
				$this->showInstallModule( $_REQUEST['node_id'] );
				break;
			case 'install_module';
				$this->installModule( $_REQUEST['node_id'], $_REQUEST['module'] );
				break;
			default :
				$this->showConnectedNodesList();
				break;
		};
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
