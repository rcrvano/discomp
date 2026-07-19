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


require_once ($DISCOMP_CONFIG['frontend_path']."/classes/files.php");
class DiscompPackage
{


	function getAvailablePackages ( ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		$ret_arr = array();
		$packages_dir = FilesClass::dirlist ( $DISCOMP_CONFIG['packages_path']);
		foreach ( $packages_dir as $key => $package ) {
			array_push($ret_arr, $package['name'] );
		};
		return $ret_arr;
	}


	function createPackage ( $name ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		FilesClass::mkdir($DISCOMP_CONFIG['packages_path']."/$name");
		FilesClass::mkdir($DISCOMP_CONFIG['packages_path']."/$name/modules");
		FilesClass::mkdir($DISCOMP_CONFIG['packages_path']."/$name/schemes");
	}

	function checkPackageExists ( $name ) 
	{
		foreach ( DiscompPackage::getAvailablePackages ( ) as $key => $package ) 
		{
			if ( $package == $name ) {
				return true;
			};
		};
		return false;
	}


	/******************************************************************/
	/*------------------------- MODULES ------------------------------*/
	/******************************************************************/
	function getPackageParameters ( $package ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		$parameters_array = array();
		$parameters_xml = FilesClass::getfilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/parameters.xml" );
		if ( $parameters_xml == "" ) { return $parameters_array; };

		$dom = new DOMDocument();
		$dom->loadXML($parameters_xml);
		
		$params_list = $dom->getElementsByTagName("param");
		for ( $i=0; $i<$params_list->length; $i++ ) {
			$item = $params_list->item($i);
			$param_arr = array (
				'name' => $item->getElementsByTagName("name")->item(0)->nodeValue,
				'type' => $item->getElementsByTagName("type")->item(0)->nodeValue,
				'comment' => $item->getElementsByTagName("comment")->item(0)->nodeValue,
				'filename' => $item->getElementsByTagName("filename")->item(0)->nodeValue,
				'filepattern' => $item->getElementsByTagName("filepattern")->item(0)->nodeValue,
				'constant' => $item->getElementsByTagName("constant")->item(0)->nodeValue,
				'size' => $item->getElementsByTagName("size")->item(0)->nodeValue,
				'regexp' => $item->getElementsByTagName("regexp")->item(0)->nodeValue
			);
			$parameters_array[$param_arr['name']] = $param_arr;
		}
		return $parameters_array;
	}

	function saveParametersArray ( $package, $parameters_array )
	{
		GLOBAL $DISCOMP_CONFIG;
		$parameters_xml = "<?xml version='1.0' encoding='UTF-8'?".">\r\n<parameters>\r\n";
		foreach ( $parameters_array as $key => $param ) {
			$parameters_xml .= "\t<param>\r\n";
			foreach ( $param as $key2=>$val2 ) {
				$parameters_xml .= "\t\t<$key2>".$val2."</$key2>\r\n";
			};
			$parameters_xml .= "\t</param>\r\n";
		};
		$parameters_xml .= "</parameters>\r\n";
		return FilesClass::savefilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/parameters.xml", $parameters_xml );
	}

	function addParameterByRequestVars ( $package ) 
	{
		$parameters_array = DiscompPackage::getPackageParameters ($package);
		$param_arr = array (
			'name' => $_REQUEST["name"],
			'type' => $_REQUEST["type"],
			'comment' => $_REQUEST["comment"],
			'filename' => $_REQUEST["filename"],
			'filepattern' => $_REQUEST["filepattern"],
			'constant' => $_REQUEST["constant"],
			'size' => $_REQUEST["size"],
			'regexp' => $_REQUEST["regexp"],
		);
		$parameters_array[$_REQUEST["old_name"]] = $param_arr;
		DiscompPackage::saveParametersArray($package, $parameters_array);
	}

	function deleteParameter ( $package, $parameter ) {
		$parameters_array = DiscompPackage::getPackageParameters ($package);
		unset($parameters_array[$parameter] );
		DiscompPackage::saveParametersArray($package, $parameters_array);
	}



	/******************************************************************/
	/*------------------------- MODULES ------------------------------*/
	/******************************************************************/


	function getAvailableModulesList ( )
	{
		GLOBAL $DISCOMP_CONFIG;
		$modules_list = array();
		foreach ( DiscompPackage::getAvailablePackages ( ) as $key => $package ) {
			$module_dir = FilesClass::dirlist ( $DISCOMP_CONFIG['packages_path']."/$package/modules");
			foreach ( $module_dir as $key2=>$file ) {
				array_push($modules_list, $package.".".$file['name'] );
			};
		};
		return $modules_list;
	}

	function getPackageModules ( $package ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		$modules_list = array();
		$module_dir = FilesClass::dirlist ( $DISCOMP_CONFIG['packages_path']."/$package/modules");
		foreach ( $module_dir as $key2=>$file ) {
			array_push($modules_list, $file['name'] );
		};
		return $modules_list;
	}

	function getModuleFiles ( $package, $module ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		return FilesClass::getlist ( $DISCOMP_CONFIG['packages_path']."/$package/modules/$module" );
	}

	function deleteModuleFile ( $package, $module, $file )
	{
		GLOBAL $DISCOMP_CONFIG;
		return FilesClass::delete ( $DISCOMP_CONFIG['packages_path']."/$package/modules/$module/$file" );
	}
	function moduleAddFile ( $package, $module, $file_arr )
	{
		GLOBAL $DISCOMP_CONFIG;
		return FilesClass::copy ( $file_arr['tmp_name'], $DISCOMP_CONFIG['packages_path']."/$package/modules/$module/".$file_arr['name'] );
	}

	function getModuleInfo ( $package, $module ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		$module_array = array("info"=>array("name"=>$module));

		
		$module_xml = FilesClass::getfilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/modules/$module/module.xml" );
		if ( $module_xml == "" ) { return $module_array; };
		$dom = new DOMDocument();
		$dom->loadXML($module_xml);

		
		$info_list = $dom->getElementsByTagName("info")->item(0);
		$module_array['info'] = array(
			'name' => $module,
			'fullname' => $info_list->getElementsByTagName("fullname")->item(0)->nodeValue,
			'comment' => $info_list->getElementsByTagName("comment")->item(0)->nodeValue,
			'os' => $info_list->getElementsByTagName("os")->item(0)->nodeValue,
			'type' => $info_list->getElementsByTagName("type")->item(0)->nodeValue
		);

		$plugin = $dom->getElementsByTagName("plugin")->item(0);
		if ( $plugin ) {
			$module_array['plugin'] = $plugin->getAttribute("src");
		};


		$commands_list = $dom->getElementsByTagName("commands")->item(0);
		$commands_start = $commands_list->getElementsByTagName("start")->item(0);
		$commands_stop = $commands_list->getElementsByTagName("stop")->item(0);
		$module_array['commands'] = array ();
		if ( $commands_start ) {
			$module_array['commands']['start'] = array (
				'cmd' => $commands_start->nodeValue,
				'max_time' => $commands_start->getAttribute("max_time"),
				'stdout' => $commands_start->getAttribute("stdout"),
				'detached' => $commands_start->getAttribute("detached")
			);
		};
		if ( $commands_stop ) {
			$module_array['commands']['stop'] = array (
				'cmd' => $commands_stop->nodeValue,
				'detached' => $commands_stop->getAttribute("detached")
			);
		};


		$module_array['parameters'] = array();

		$parameters_input = $dom->getElementsByTagName("input")->item(0);
		if ( $parameters_input ) {
			$params_list = $parameters_input->getElementsByTagName("param");
			for ( $i=0; $i<$params_list->length; $i++ ) {
				$item = $params_list->item($i);
				$param_arr = array (
					'name' => $item->getElementsByTagName("name")->item(0)->nodeValue,
					'type' => $item->getElementsByTagName("type")->item(0)->nodeValue,
					'comment' => $item->getElementsByTagName("comment")->item(0)->nodeValue,
					'filename' => $item->getElementsByTagName("filename")->item(0)->nodeValue,
					'filepattern' => $item->getElementsByTagName("filepattern")->item(0)->nodeValue,
					'constant' => $item->getElementsByTagName("constant")->item(0)->nodeValue,
					'size' => $item->getElementsByTagName("size")->item(0)->nodeValue,
					'regexp' => $item->getElementsByTagName("regexp")->item(0)->nodeValue
				);
				$module_array['parameters']['input'][$param_arr['name']]  = $param_arr;
			}
		};

		$parameters_output = $dom->getElementsByTagName("output")->item(0);
		if ( $parameters_output ) {
			$params_list = $parameters_output->getElementsByTagName("param");
			for ( $i=0; $i<$params_list->length; $i++ ) {
				$item = $params_list->item($i);
				$param_arr = array (
					'name' => $item->getElementsByTagName("name")->item(0)->nodeValue,
					'type' => $item->getElementsByTagName("type")->item(0)->nodeValue,
					'comment' => $item->getElementsByTagName("comment")->item(0)->nodeValue,
					'filename' => $item->getElementsByTagName("filename")->item(0)->nodeValue,
					'filepattern' => $item->getElementsByTagName("filepattern")->item(0)->nodeValue,
					'constant' => $item->getElementsByTagName("constant")->item(0)->nodeValue,
					'size' => $item->getElementsByTagName("size")->item(0)->nodeValue,
					'regexp' => $item->getElementsByTagName("regexp")->item(0)->nodeValue
				);
				$module_array['parameters']['output'][$param_arr['name']] = $param_arr;
			}
		};

		return $module_array;
	}



	function saveModuleInfo ( $package, $module, $module_array ) 
	{
		GLOBAL $DISCOMP_CONFIG;

		$module_xml  = "<?xml version='1.0' encoding='UTF-8'?".">\r\n";
		$module_xml .= "<module>\r\n";

		$module_xml .= "\t<info>\r\n";
		foreach ( $module_array['info'] as $key => $param ) {
			if ( $key == 'name' ) { $param = $module; }
			$module_xml .= "\t\t<$key>".$param."</$key>\r\n";
		};
		$module_xml .= "\t</info>\r\n";



		$module_xml .= "\t<commands>\r\n";
		$module_xml .= "\t\t<start max_time='".$module_array['commands']['start']['max_time']."' stdout='".$module_array['commands']['start']['stdout']."' detached='".$module_array['commands']['start']['detached']."'>".$module_array['commands']['start']['cmd']."</start>\r\n";
		if ( $module_array['commands']['stop'] ) {
			$module_xml .= "\t\t<stop detached='".$module_array['commands']['stop']['detached']."'>".$module_array['commands']['stop']['cmd']."</stop>\r\n";
		};
		$module_xml .= "\t</commands>\r\n";


		if ( $module_array['plugin'] ) {
			$module_xml .= "\t<plugin src='".$module_array['plugin']."'/>\r\n";
		}; 

		
		$module_xml .= "\t<parameters>\r\n";
		if ( is_array($module_array['parameters']['input'] ) ) {
			$module_xml .= "\t\t<input>\r\n";
			foreach ( $module_array['parameters']['input'] as $key => $param ) {
				$module_xml .= "\t\t\t<param>\r\n";
				foreach ( $param as $key2=>$val2 ) {
					$module_xml .= "\t\t\t\t<$key2>".$val2."</$key2>\r\n";
				};
				$module_xml .= "\t\t\t</param>\r\n";
			};
			$module_xml .= "\t\t</input>\r\n";
		};

		if ( is_array($module_array['parameters']['output'] ) ) {
			$module_xml .= "\t\t<output>\r\n";
			foreach ( $module_array['parameters']['output'] as $key => $param ) {
				$module_xml .= "\t\t\t<param>\r\n";
				foreach ( $param as $key2=>$val2 ) {
					$module_xml .= "\t\t\t\t<$key2>".$val2."</$key2>\r\n";
				};
				$module_xml .= "\t\t\t</param>\r\n";
			};
			$module_xml .= "\t\t</output>\r\n";
		};


		$module_xml .= "\t</parameters>\r\n";
		$module_xml .= "</module>\r\n";
		
		return FilesClass::savefilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/modules/$module/module.xml", $module_xml );
	}

	function checkModuleExists ( $package, $module ) 
	{
		foreach ( DiscompPackage::getPackageModules ($package) as $key=>$exists_module ) 
		{
			if ( $exists_module == $module ) { return true; };
		}
		return false;
	}


	function addNewModule ( $package, $module ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		FilesClass::mkdir ( $DISCOMP_CONFIG['packages_path']."/$package/modules/".$module);
	}



	/******************************************************************/
	/*------------------------- SCHEMES ------------------------------*/
	/******************************************************************/
	function getAvailableSchemes ( )
	{
		GLOBAL $DISCOMP_CONFIG;
		$schemes_list = array();
		foreach ( DiscompPackage::getAvailablePackages ( ) as $key => $package ) {
			$scheme_dir = FilesClass::dirlist ( $DISCOMP_CONFIG['packages_path']."/$package/schemes");
			foreach ( $scheme_dir as $key2=>$file ) {
				array_push($schemes_list, $package.".".$file['name'] );
			};
		};
		return $schemes_list;
	}

	function getPackageSchemes ( $package )
	{
		GLOBAL $DISCOMP_CONFIG;
		$schemes_list = array();
		$scheme_dir = FilesClass::dirlist ( $DISCOMP_CONFIG['packages_path']."/$package/schemes");
		foreach ( $scheme_dir as $key2=>$file ) {
			array_push($schemes_list, $file['name'] );
		};
		return $schemes_list;
	}

	function checkSchemeExists ( $package, $scheme ) 
	{
		foreach ( DiscompPackage::getPackageSchemes ($package) as $key=>$exists_scheme ) 
		{
			if ( $exists_scheme == $scheme ) { return true; };
		}
		return false;
	}

	function addNewScheme ( $package, $scheme ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		FilesClass::mkdir ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme);
		FilesClass::mkdir ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/log");
		FilesClass::mkdir ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/parameters");
		FilesClass::mkdir ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/parameters/input");
		FilesClass::mkdir ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/parameters/output");
	}

	function getSchemeSource ( $package, $scheme ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		$source = FilesClass::getfilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/process.xml");
		if ( $source == "" ) {
			$source = "<?xml version='1.0' encoding='UTF-8'?".">\r\n<process>\r\n\t<plugin src='plugin.js'/>\t\n\t<stage>\r\n\t\t<module name='???'/>\r\n\t</stage>\r\n</process>";
		};
		return $source;
	}


	function saveSchemeSource ( $package, $scheme, $source ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		return FilesClass::savefilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/process.xml", stripslashes($source));
	}

	function getSchemeInputParameters ( $package, $scheme ) 
	{
		//TODO:: INFORMAZIONNOE PLANIOROVANIE!!!
		GLOBAL $DISCOMP_CONFIG;
		$source_xml = FilesClass::getfilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/process.xml");

		if ( $source_xml == "" ) { return array(); }

		$dom = new DOMDocument();
		$dom->loadXML($source_xml);

		$module_info = $dom->getElementsByTagName("module")->item(0);
		if ( !$module_info ) { return array (); };


		$module_name = $module_info->getAttribute("name");
		$module_arr  = DiscompPackage::getModuleInfo($package, $module_name);

		$module_input_params = $module_arr['parameters']['input'];
		foreach ( $module_input_params as $key => $val ) {
			$module_input_params[$key]['value'] = FilesClass::getfilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/parameters/input/".$val['filename']);
		};
		return $module_input_params;
	}

	function saveSchemeInputParameters ( $package, $scheme, $parameters_arr ) 
	{
		GLOBAL $DISCOMP_CONFIG;
		foreach ( $parameters_arr as $key => $val ) {
			FilesClass::savefilecontent ( $DISCOMP_CONFIG['packages_path']."/$package/schemes/".$scheme."/parameters/input/".$val['filename'], $val['value']);
		};
	}



}
