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

class loginComponent extends Components
{
	function getComponentData ()
	{
		switch ($this->action) {
			case 'auth':
				$login    = $_REQUEST['login'];
				$password = $_REQUEST['password'] == "" ? $_REQUEST['password'] : md5($_REQUEST['password']);
				$this->communicator->setUser($login, $password);
				if ( $user_type = $this->communicator->checkAuthorization() ) {
					$_SESSION['user_login']    = $login;
					$_SESSION['user_password'] = $password;
					$_SESSION['user_type']     = $user_type;
				} else {
					$this->interface->assign("ERROR_CODE", 110);
					$this->interface->display("main.tpl");
					return;
				};
				header("Location:index.php");
				break;
	
			case 'logout':
				session_destroy();
				header("Location:index.php?component=login");
				return;
	
			case 'change_password_form':
				$this->interface->display("change_password_form.tpl");
				break;

			case 'do_change_password':
				if ( $_REQUEST['password'] == "" ) {
					$this->interface->assign("ERROR_CODE", 111);
					$this->interface->display("change_password_form.tpl");
					return;
				};
				if ( md5($_REQUEST['old_password']) != $_SESSION['user_password'] && $_SESSION['user_password']!="" ) {
					$this->interface->assign("ERROR_CODE", 113);
					$this->interface->display("change_password_form.tpl");
					return;
				};
				$this->communicator->setUser($_SESSION['user_login'], $_SESSION['user_password']);
				if ( $this->communicator->changeUserPassword($_REQUEST['password']) ) {
					$_SESSION['user_password'] = md5($_REQUEST['password']);
				} else {
					$this->interface->assign("ERROR_CODE", 112);
					$this->interface->display("change_password_form.tpl");
					return;
				};
				header("Location:index.php");
				break;
	
			default:
				$this->interface->display("main.tpl");
				break;
		};
	}
};
	

?>
