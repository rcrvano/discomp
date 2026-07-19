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


class logComponent extends Components {

	function getComponentData()
	{
		switch ( $this->action ) {
			case 'view':
				$this->viewLogFile($_REQUEST['dir'], $_REQUEST['file']);
				break;
			default :
				$this->showLogFiles();
				break;
		};
	}

	function viewLogFile ( $dir, $file ) 
	{
		//check dir and file
		if ( !DiscompSecurity::CheckURLVariableForUseInInclude ( $dir ) || !DiscompSecurity::CheckURLVariableForUseInInclude ( $file ) ) {
			echo "Hacking attempt";
			return 0;
		};
		

		$file_content = $this->getLogContent ( $dir, $file, 0, 4096 );
		$file_content = preg_replace("/</Umsi","&lt;", $file_content);
		$file_content = preg_replace("/>/Umsi","&gt;", $file_content);
		$file_content = preg_replace("/\n/Umsi","<br>", $file_content);
		$file_content = preg_replace("/\[common\]/Umsi", "<span class='log-common'>[common]</span>",$file_content);
		$file_content = preg_replace("/\[debug\]/Umsi", "<span class='log-debug'>[debug]</span>",$file_content);
		$file_content = preg_replace("/\[protocol\]/Umsi", "<span class='log-protocol'>[protocol]</span>",$file_content);
		$file_content = preg_replace("/\[notice\]/Umsi", "<span class='log-notice'>[notice]</span>",$file_content);
		$file_content = preg_replace("/\[WARNING\]/Umsi", "<span class='log-warning'>[WARNING]</span>",$file_content);
		$file_content = preg_replace("/\[ERROR\]/Umsi", "<span class='log-error'>[ERROR]</span>",$file_content);
		$file_content = preg_replace("/\[CORE\]/Umsi", "<span class='log-core'>[CORE]</span>",$file_content);

		$this->interface->assign ("LOG_DIR",      $dir );
		$this->interface->assign ("LOG_FILE",     $file);
		$this->interface->assign ("FILE_CONTENT", $file_content);
		$this->interface->display("file.tpl");
	}

	function getLogContent ( $dir, $file, $seek, $size ) 
	{
		$fh = fopen (SERVER_PATH."/".$dir."/".$file, "rb");;
		fseek ( $fh, $seek );
		$file_content = fread ($fh, $size );
		fclose ( $fh );
		
		// trim end characters
		$file_content = preg_replace("/\n[^\n]*$/Umsi","", $file_content);

		return $file_content;
	}

	function showLogFiles ()
	{
		$dir = "/log/";

		$this->interface->assign ("LOG_DIR",   $dir );
		$this->interface->assign ("LOG_FILES", $this->getLogFiles ( SERVER_PATH."/".$dir ) );
		$this->interface->display("list.tpl");
	}

	function getLogFiles ( $dir ) {

		$dir_handler = @opendir( $dir );
		
		if($dir_handler === false)
		{
			$this->error = "Couldn't open dir: $dir";
			return array();
		}

		$filelist = array();
		while ( $sFile = readdir( $dir_handler ) )
		{
			if ( is_file($dir."/".$sFile) ) {
				$fileinfo = pathinfo($dir."/".$sFile);
				$filelist[$sFile] = array(
					"name" => $sFile,
					"size" => $this->humanReadableSize(filesize($dir."/".$sFile)),
					"_size" => filesize($dir."/".$sFile),
					"ext"  => $info['extension'],
					"date" => date("Y-m-d H:i", filemtime($dir."/".$sFile)),
					"timestamp" => filemtime($dir."/".$sFile) 
				);
			}
		};
		ksort($filelist);
		return $filelist;
	}

	function humanReadableSize ( $file_size ) 
	{
		if ( $file_size < 1024 ) {
			return $file_size."";
		} else if ( $file_size < 1024*1024 ) {
			return (int)($file_size/1024)." Kb";
		} else if ( $file_size < 1024*1024*1024 ) {
			return (int)($file_size/(1024*1024))." Mb";
		} else if ( $file_size < 1024*1024*1024*1024 ) {
			return (int)($file_size/(1024*1024*1024))." Gb";
		};
	}


}

?>
