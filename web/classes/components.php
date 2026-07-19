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



/**
 * @class Components
 * @brief Base class for creating Components
 *
 * Inherit this class for your component class
 */
class Components {
	var $interface;      //!< component Interfaces
	var $lang_variables; //!< component language variables
	var $communicator;
	var $config;
	var $action;

	/**
	 * Constructor.
	 * Here created interfaces, lang variables, etc.
	 */
	function Components () 
	{
		GLOBAL $DISCOMP_CONFIG;
		GLOBAL $DISCOMP_COMMUNICATOR;
		$this->config = $DISCOMP_CONFIG;
		$this->communicator = $DISCOMP_COMMUNICATOR;
		$this->action = $_REQUEST['action'];
		
		
		if ( DiscompSecurity::CheckURLVariableForUseInInclude($this->comp) ) {
			if ( file_exists(FRONTEND_PATH."/components/".$this->comp."/interface.php") ) {
				include ( FRONTEND_PATH."/components/".$this->comp."/interface.php" );

				$interface_class_name = $this->comp."Interface";
				$this->interface   = new $interface_class_name('component');
			} else {
				$this->interface   = new Interfaces('component');
			};

			if ( file_exists(INTERFACE_DIR."/languages/en/components/".$this->comp.".ini") ) {
				$this->lang_variables = parse_ini_file(INTERFACE_DIR."/languages/en/components/".$this->comp.".ini");
			};
			if ( file_exists(INTERFACE_DIR."/languages/".$this->user->lang."/components/".$this->comp.".ini") ) {
				$tmp_langs = parse_ini_file(INTERFACE_DIR."/languages/".$this->user->lang."/components/".$this->comp.".ini");
				foreach ( $tmp_langs as $key=>$val ) {
					$this->lang_variables[$key] = $val;
				};
			};
			$this->interface->lang_variables = $this->lang_variables;
		};
	}	
}

?>
