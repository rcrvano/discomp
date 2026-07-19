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


class FilesClass
{
	var $user;
	var $preferences;
	var $error;
	var $current_dir;
	var $start_dir;
	var $sizeLimit = 52428800;
	var $weblet_id = "";

	function getlist($dir) 
	{
		if ( !$dir ) 
			return;

		$dirlist  = array();
		$filelist = array();

		$dir_handler = @opendir( $dir );

		if($dir_handler === false)
		{
			$this->error = "Directory isn't exists";
			return array();
		}

		while ( $sFile = readdir( $dir_handler ) )
		{
			if ( is_dir($dir."/".$sFile) && $sFile != "." && $sFile != ".." && $sFile != "CVS" && $sFile != ".svn" ) {
				$item = array("name" => $sFile, "type" => "dir");
				$filelist[$sFile] = $item;
			} else if ( is_file($dir."/".$sFile) && $sFile != ".htaccess" && $sFile != ".htpasswd" && substr($sFile,0,1) != '.' ) {
				$filelist[$sFile] = FilesClass::getFileInfo( $dir."/".$sFile );
			}
		};
		sort($dirlist);
		sort($filelist);


		return array_merge($dirlist, $filelist);
	}

	function filelist ( $dir ) {
		$new_arr = array();
		foreach ( FilesClass::getlist ( $dir ) as $key => $file ) {
			if ( $file['type'] == 'file' ) { 
				array_push ( $new_arr, $file ) ;
			};
		};
		return $new_arr;
	}

	function dirlist ( $dir ) {
		$new_arr = array();
		foreach ( FilesClass::getlist ( $dir ) as $key => $file ) {
			if ( $file['type'] == 'dir' ) { 
				array_push ( $new_arr, $file ) ;
			};
		};
		return $new_arr;
	}


	function getFileInfo ($file_path) 
	{

		$info = pathinfo($file_path);
		$item = array(
			"name" => basename($file_path),
			"type" => "file",
			"size" => FilesClass::humanReadableSize(filesize($file_path)),
			"_size" => filesize($file_path),
			"ext"  => $info['extension'],
			"date" => date("Y-m-d H:i", filemtime($file_path)),
			"timestamp" => filemtime($file_path) );

		return $item;
	}


	function mkdir ( $dir ) {
		mkdir ($dir);
	}


	function getfilecontent ( $file ) 
	{
		if ( is_file ( $file ) ) {
			return file_get_contents($file);
		} else { 
			return "";
		};
	}

	function savefilecontent ( $file, $content ) 
	{
		$fh = fopen ( $file, "w" );
		if ( !$fh ) { return 0; };
		fwrite ( $fh, $content );
		fclose ( $fh );

		return 1;
	}

	function backupfile ( $file ) {
		copy ( $file, $file.".bak" );
	}

	function delete ( $file ) {
		unlink ( $file );
	}

	function copy ( $file_s, $file_d ) 
	{
		copy ( $file_s, $file_d );
	}


	function humanReadableSize ( $file_size ) 
	{
		if ( $file_size < 1024 ) {
			return $file_size." byte";
		} else if ( $file_size < 1024*1024 ) {
			return (int)($file_size/1024)." Kb";
		} else if ( $file_size < 1024*1024*1024 ) {
			return (int)($file_size/(1024*1024))." Mb";
		} else if ( $file_size < 1024*1024*1024*1024 ) {
			return (int)($file_size/(1024*1024*1024))." Gb";
		};
	}


}
