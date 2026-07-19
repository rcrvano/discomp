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

class processesComponent extends Components {

	function getComponentData()
	{
		switch ( $this->action ) {
			case 'show_logs':
				$this->showProcessLog($_REQUEST['process']);
				break;
			case 'show_log_file':
				$this->showProcessLogContent($_REQUEST['process'],$_REQUEST['log_file'],true);
				break;
			case 'get_log_file':
				$this->showProcessLogContent($_REQUEST['process'],$_REQUEST['log_file'],false);
				break;
			case 'prepare_start':
				$this->prepareProcessStart($_REQUEST['process']);
				break;
			case 'start':
				$this->prepareProcessSaveParams($_REQUEST['process']);
				//TODO! Need to save all input parameters!!!
				$this->communicator->startProcess($_REQUEST['process']);	
				header ( "Location:index.php?component=processes");
				break;
			default :
				$this->showProcessesList();
				break;
		};
	}



	function showProcessesList ()
	{
		$this->interface->assign ("ACTIVE_PROCESSES_LIST", $this->communicator->getProcessesList());
		
		$schemes = DiscompPackage::getAvailableSchemes();
		$this->interface->assign ("PROCESSES_LIST", $schemes);

		$this->interface->display("list.tpl");
	}



	function prepareProcessStart ($process_name)
	{
		list ( $package, $scheme ) = split ( "\.", $process_name ) ;
		$this->interface->assign ("INPUT_PARAMETERS", DiscompPackage::getSchemeInputParameters ( $package, $scheme, true) );

		$this->interface->display("prepare_process.tpl");
	}

	function prepareProcessSaveParams($process_name )
	{
		list ( $package, $scheme ) = split ( "\.", $process_name ) ;
		$parameters_arr = DiscompPackage::getSchemeInputParameters ( $package, $scheme, true);

		foreach ( $parameters_arr as $key => $param ) {
			$parameters_arr[$key]['value'] = $_REQUEST['param_'.$key] ? $_REQUEST['param_'.$key] : $_REQUEST['param_'.preg_replace("/\./Umsi","_",$key)] ;
		};

		DiscompPackage::saveSchemeInputParameters ( $package, $scheme, $parameters_arr) ;
	}




	function showProcessLog($process_name) 
	{
		$logs_dir   = $this->getProcessAbsoluteDir($process_name)."/log";
		
		require_once ($_SERVER['DOCUMENT_ROOT']."/classes/files.php");
		$this->interface->assign("LOG_FILES", FilesClass::filelist ($logs_dir));
		$this->interface->assign("PROCESS_NAME", $process_name );
		$this->interface->display("log_file_list.tpl");
	}

	function showProcessLogContent($process_name, $log_file,$display=true) 
	{
		$log_file   = $this->getProcessAbsoluteDir($process_name)."/log/".$log_file;
		$log_file_content = file_get_contents($log_file);
		$this->interface->assign("PROCESS_NAME", $process_name );
		$this->interface->assign("LOG_FILE_CONTENT", $log_file_content);
		if ( $display ) { 
			$this->interface->display("log_file_content.tpl");
		} else {
			echo $log_file_content;
		};
	}



	function showProcessStats($process_name) 
	{
		$stats_file = $this->getProcessAbsoluteDir($process_name)."/statistic.xml";
		$logs_dir   = $this->getProcessAbsoluteDir($process_name)."/log";
		print $logs_dir;
	}



	function getProcessAbsoluteDir ( $process_name ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		list ($package, $scheme) = split ( "\.", $process_name);
		return preg_replace("#//#Umsi","/",$DISCOMP_CONFIG['packages_path']."/$package/schemes/$scheme/");
	}
}

?>
