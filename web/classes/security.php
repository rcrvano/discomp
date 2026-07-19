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


class DiscompSecurity {
	
	function FixXSS ( $var ) {
		if ( is_string($var) ) {
			$var = preg_replace("/<script/Umsi", "", $var);
		} else if ( is_array($var) ) {
			foreach ( $var as $key=>$val ) {
				$var[$key] = DiscompSecurity::FixXSS ( $val );
			};
		};
		return $var;
	}

	/**
	 * Check URL variables
	 */
	function CheckURLVariableForUseInInclude( $var ) 
	{
		if ( $var == "" ) { return 1; };
		if ( !preg_match("/^[A-Z|a-z|0-9|_|\/|\.]*$/m", $var) ) {
			return 0;
		}
		if ( preg_match("/\.\./m", $var) ) {
			return 0;
		};
		return 1;
	}

	function CheckUploadedFile ( $file )
	{
		if ( is_array($file) ) { 
			$filename = $file['name'];
		} else {
			$filename = $file;
		};
	
		if (	
			$filename == '.htaccess' ||
			preg_match("/.php$/Umsi", $filename) ||
			preg_match("/.php3$/Umsi", $filename) ||
			preg_match("/.php4$/Umsi", $filename) ||
			preg_match("/.php5$/Umsi", $filename) ||
			preg_match("/.asp$/Umsi", $filename) ||
			preg_match("/.cgi$/Umsi", $filename) ||
			preg_match("/.pl$/Umsi", $filename) 
		   ) {
			return false;
		} 
		return true;
	}
	
}
?>
