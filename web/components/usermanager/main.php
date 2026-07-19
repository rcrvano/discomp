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


class usermanagerComponent extends Components {

	function getComponentData()
	{
		switch ( $this->action ) {
			case 'show_create_user':
				$this->interface->display("create_user.tpl");
				break;
			case 'do_create_user':
				if ( $_REQUEST['login'] == "" || $_REQUEST['password'] == "" ) {
					$this->interface->assign("ERROR", "Login or password is wrong");
					$this->interface->display("create_user.tpl");
					return;
				}
				if ( $this->communicator->createNewUser($_REQUEST['login'], $_REQUEST['password'], $_REQUEST['type']) ) {
					$this->interface->display("create_user_success.tpl");
				} else {
					$this->interface->assign("ERROR", "Could not create user");
					$this->interface->display("create_user.tpl");
				}
				break;
			default :
				$this->showUsersList();
				break;
		};
	}

	function showUsersList ()
	{
		$this->interface->assign ("USERS_LIST", $this->communicator->getUsersList());
		$this->interface->display("list.tpl");
	}
}

?>
