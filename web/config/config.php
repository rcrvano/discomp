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


$DISCOMP_CONFIG['server']['host']    = "127.0.0.1";
$DISCOMP_CONFIG['server']['port']    = "5712";
$DISCOMP_CONFIG['server']['timeout'] = "30";
$DISCOMP_CONFIG['language'] = 'en';
$DISCOMP_CONFIG['debug'] = 0;

// Specify paths here if this way isn't working or 
// web-fronted located in the another dir
$DISCOMP_CONFIG['frontend_path'] = $_SERVER['DOCUMENT_ROOT'];
$DISCOMP_CONFIG['server_path']   = "/store1/discomp/server/";
$DISCOMP_CONFIG['packages_path']   = "/store1/discomp/server/packages/";


// some global defines
define ( "DISCOMP_VERSION",    "0.2c");
define ( "APP_URL",        "");
define ( "FRONTEND_PATH",  $DISCOMP_CONFIG['frontend_path']);
define ( "FULL_PATH",  $DISCOMP_CONFIG['frontend_path']);
define ( "SERVER_PATH",    $DISCOMP_CONFIG['server_path']);
define ( "INTERFACE_DIR",  FRONTEND_PATH."/interface" );
define ( "TEMPLATES_DIR",  FRONTEND_PATH."/interface/templates" );
define ( "LANGUAGES_DIR",  FRONTEND_PATH."/interface/languages" );
define ( "CACHE_DIR",      FRONTEND_PATH."/temp/smarty_cache" );


// Session parameters
$session_expiration_time = 60*60*24; // 30 minutes
//ini_set("session.gc_maxlifetime",   $session_expiration_time); 
//ini_set("session.gc_probability",   "1");
//ini_set("session.gc_divisor",       "100");
ini_set("session.save_path",        $DISCOMP_CONFIG['frontend_path']."/temp/sessions/");
session_set_cookie_params($session_expiration_time);
session_name("DISCOMPSESSID");

// Global PHP settings
#ini_set("display_errors","Off");
ini_set("log_errors",          "On");
ini_set("error_log",           FULL_PATH."/temp/logs/php_errors.log");
ini_set("upload_max_filesize", "50M");
ini_set("error_reporting",     "6135");
ini_set("memory_limit",        "48M");
ini_set("allow_url_fopen",     "Off");



?>
