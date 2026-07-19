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
require_once ( $DISCOMP_CONFIG['frontend_path']."/classes/security.php" );

class packagemanagerComponent extends Components 
{

	function getComponentData()
	{
		if ( !DiscompSecurity::CheckURLVariableForUseInInclude( $_REQUEST['package'] ) ||
		     !DiscompSecurity::CheckURLVariableForUseInInclude( $_REQUEST['package_name'] ) ||
		     !DiscompSecurity::CheckURLVariableForUseInInclude( $_REQUEST['scheme'] ) ||
		     !DiscompSecurity::CheckURLVariableForUseInInclude( $_REQUEST['module'] ) ||
		     !DiscompSecurity::CheckURLVariableForUseInInclude( $_REQUEST['parameter'] )  ) 
		{
			echo "Hacking attempt"; exit;
		};
		     
		switch ( $this->action ) 
		{
			case 'create_new':
				$this->interface->display("new_package.tpl");
				break;

			case 'do_create_new':
				if ( DiscompPackage::checkPackageExists ( $_REQUEST['package_name'] ) ) {
					$this->interface->assign("ERROR", "exists");
					$this->interface->display("new_package.tpl");
				} else {
					DiscompPackage::createPackage($_REQUEST['package_name']);
					header("Location:index.php?component=packagemanager&package=".$_REQUEST['package_name']);
				}
				break;




			case 'parameter_edit':
				if ( $_REQUEST['parameter'] != "" ) {
					$parameters_arr = DiscompPackage::getPackageParameters($_REQUEST['package']);
					$this->interface->assign("PARAM", $parameters_arr[$_REQUEST['parameter']]);
				};
				$this->interface->display("parameter_edit.tpl");
				break;
			case 'do_parameter_edit':
				$parameters_arr = DiscompPackage::getPackageParameters($_REQUEST['package']);
				if ( ( $_REQUEST['old_name'] == "" || ($_REQUEST['name'] != $_REQUEST['old_name']) ) && is_array($parameters_arr[$_REQUEST['name']]) ) {
					$this->interface->assign("ERROR", "exists");
					$this->interface->assign("PARAM", $_REQUEST);
					$this->interface->display("parameter_edit.tpl");
				} else {
					DiscompPackage::addParameterByRequestVars($_REQUEST['package']);
					header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']);
				}
				break;
			case 'parameter_delete':
				DiscompPackage::deleteParameter($_REQUEST['package'], $_REQUEST['parameter']);
				header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']);
				break;



			case 'module_add':
				$this->interface->display("module_add.tpl");
				break;
			case 'do_module_add':
				if ( DiscompPackage::checkModuleExists ( $_REQUEST['package'], $_REQUEST['module'] ) ) {
					$this->interface->assign("ERROR", "exists");
					$this->interface->assign("MODULE", array("info"=>$_REQUEST));
					$this->interface->display("module_add.tpl");
				} else {
					DiscompPackage::addNewModule ( $_REQUEST['package'], $_REQUEST['module'] );
					header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=module_edit&module=".$_REQUEST['module']);
				};
				break;

			case 'module_edit':
				$this->interface->assign( "MODULE", DiscompPackage::getModuleInfo($_REQUEST['package'], $_REQUEST['module']) );
				$this->interface->assign( "MODULE_FILES", DiscompPackage::getModuleFiles($_REQUEST['package'], $_REQUEST['module']) );
				$this->interface->assign( "PACKAGE_PARAMETERS", DiscompPackage::getPackageParameters($_REQUEST['package']) );
				$this->interface->display("module_edit.tpl");
				break;
			case 'save_module_info':
				$module_arr = DiscompPackage::getModuleInfo($_REQUEST['package'], $_REQUEST['module']);
				foreach ( $module_arr['info'] as $key=>$val ) {
					$module_arr['info'][$key] = $_REQUEST[$key];
				}
				$module_arr['commands']['start']['cmd'] = $_REQUEST['start_cmd'];
				$module_arr['commands']['stop']['cmd'] = $_REQUEST['stop_cmd'];
				$module_arr['plugin'] = $_REQUEST['plugin'];
				DiscompPackage::saveModuleInfo ( $_REQUEST['package'], $_REQUEST['module'], $module_arr);
				header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=module_edit&module=".$_REQUEST['module']);
				break;	
			case 'do_module_add_parameter':
				$module_arr = DiscompPackage::getModuleInfo($_REQUEST['package'], $_REQUEST['module']);
				$parameters_arr = DiscompPackage::getPackageParameters($_REQUEST['package']);
				if ( $_REQUEST['input_parameter'] != "" && is_array($parameters_arr[$_REQUEST['input_parameter']]) ) {
					$module_arr['parameters']['input'][$_REQUEST['input_parameter']] = $parameters_arr[$_REQUEST['input_parameter']];
					DiscompPackage::saveModuleInfo ( $_REQUEST['package'], $_REQUEST['module'], $module_arr);
				};
				if ( $_REQUEST['output_parameter'] != "" && is_array($parameters_arr[$_REQUEST['output_parameter']])  ) {
					$module_arr['parameters']['output'][$_REQUEST['output_parameter']] = $parameters_arr[$_REQUEST['output_parameter']];
					DiscompPackage::saveModuleInfo ( $_REQUEST['package'], $_REQUEST['module'], $module_arr);
				};
				header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=module_edit&module=".$_REQUEST['module']);
				break;

			case 'delete_module_parameter':
				$module_arr = DiscompPackage::getModuleInfo($_REQUEST['package'], $_REQUEST['module']);
				if ( $_REQUEST['input_parameter'] != "" ) {
					unset($module_arr['parameters']['input'][$_REQUEST['input_parameter']]);
					DiscompPackage::saveModuleInfo ( $_REQUEST['package'], $_REQUEST['module'], $module_arr);
				};
				if ( $_REQUEST['output_parameter'] != "" ) {
					unset($module_arr['parameters']['output'][$_REQUEST['output_parameter']]);
					DiscompPackage::saveModuleInfo ( $_REQUEST['package'], $_REQUEST['module'], $module_arr);
				};
				header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=module_edit&module=".$_REQUEST['module']);
				break;

			case 'do_module_upload_file':
				DiscompPackage::moduleAddFile ( $_REQUEST['package'], $_REQUEST['module'], $_FILES['file'] );
				header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=module_edit&module=".$_REQUEST['module']);
				break;

		
			case 'delete_module_file':
				DiscompPackage::deleteModuleFile($_REQUEST['package'], $_REQUEST['module'], $_REQUEST['file'] );
				header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=module_edit&module=".$_REQUEST['module']);
				break;



			case 'scheme_add':
				$this->interface->display("scheme_add.tpl");
				break;
			case 'do_scheme_add':
				if ( DiscompPackage::checkSchemeExists ( $_REQUEST['package'], $_REQUEST['scheme'] ) ) {
					$this->interface->assign("ERROR", "exists");
					$this->interface->display("scheme_add.tpl");
				} else {
					DiscompPackage::addNewScheme($_REQUEST['package'], $_REQUEST['scheme']);
					header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=scheme_edit&scheme=".$_REQUEST['scheme']);
				}
				break;
			case 'scheme_edit':
				$this->interface->assign("SCHEME_SRC", DiscompPackage::getSchemeSource ( $_REQUEST['package'], $_REQUEST['scheme'] ));

				$this->interface->display("scheme_edit.tpl");
				break;
			case 'do_scheme_save':
				if ( !DiscompPackage::saveSchemeSource ( $_REQUEST['package'], $_REQUEST['scheme'], $_REQUEST['scheme_src'] ) ) {
					$this->interface->assign("SCHEME_SRC", $_REQUEST['scheme_src']);
					$this->interface->assign("ERROR", "notsaved");
					$this->interface->display("scheme_edit.tpl");
				} else {
	
					header("Location:index.php?component=packagemanager&package=".$_REQUEST['package']."&action=scheme_edit&scheme=".$_REQUEST['scheme']);
				};
				break;




			default :
				if ( $_REQUEST['package'] != "" ) {
					$this->interface->assign("PARAMETERS_LIST",  DiscompPackage::getPackageParameters($_REQUEST['package']) );
					$this->interface->assign("MODULES_LIST",     DiscompPackage::getPackageModules($_REQUEST['package']) );
					$this->interface->assign("SCHEMES_LIST",     DiscompPackage::getPackageSchemes($_REQUEST['package']) );
					$this->interface->display ( "package_main.tpl" );
				} else {
					$packages = DiscompPackage::getAvailablePackages();
		
					$this->interface->assign ("PACKAGES_LIST", $packages);
		
					$this->interface->display("list.tpl");
				};
				break;
		};
	}

	function moduleEdit ( $package, $module ) 
	{
		$this->interface->display("module_edit.tpl");
	}

}

?>
