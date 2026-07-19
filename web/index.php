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

header("Content-type:text/html; charset=UTF-8\n\n");

include ( "config/config.php" );
include ( "classes/security.php" );
include ( "classes/communicator.php" );
include ( "classes/components.php" );
include ( "classes/interfaces.php" );
include ( "libs/Smarty/Smarty.class.php" );


// Init Discomp Communicator class
$DISCOMP_COMMUNICATOR = new DiscompCommunicator ( $DISCOMP_CONFIG['server']['host'], $DISCOMP_CONFIG['server']['port'], $DISCOMP_CONFIG['server']['timeout'] );

// -------------------- Authorization -------------------------
session_start();
if ( $_REQUEST['component'] == 'login' ) {
	if ( file_exists("components/login/main.php") ) {
		include_once ( "components/login/main.php" );
	};
	$COMPONENT = "login";
	$LOGIN_COMPONENT = new loginComponent();
	$LOGIN_COMPONENT->getComponentData();
	exit;
};
if ( !isset($_SESSION['user_login']) ) {
	header("Location:index.php?component=login");
	exit;
} else {
	$DISCOMP_COMMUNICATOR->setUser( $_SESSION['user_login'], $_SESSION['user_password']);
	if ( $_SESSION['user_password'] == "" ) {
		header("Location:index.php?component=login&action=change_password_form");
		exit;
	};
};
session_write_close();


// ------------------- Main system calls ------------------------
$COMPONENT = $_REQUEST['comp'] ? $_REQUEST['comp'] : $_REQUEST['component'];
if ( !DiscompSecurity::CheckURLVariableForUseInInclude($COMPONENT) ) {
	echo "Hacking attempt"; 
	exit;
};
if ( !$COMPONENT ) {
	$COMPONENT = ( $_SESSION['user_login'] == 'admin' ) ? "nodes" : "user";
};
if ( !DiscompSecurity::CheckURLVariableForUseInInclude($COMPONENT) ) { //check again
	echo "Hacking attempt"; exit;
};

if ( file_exists("components/" . $COMPONENT . "/main.php") ) {
	include_once ( "components/" . $COMPONENT . "/main.php" );
} else {
	echo "Could't find component $COMPONENT";
	exit;
};

$COMPONENTClassName = $COMPONENT."Component";
$COMPONENTClass     = new $COMPONENTClassName();
echo $COMPONENTClass->getComponentData();

?>
