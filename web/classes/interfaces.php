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
 * @class Interfaces
 * @brief Components interfaces
 *
 * This class provide a simple way for Components for works with Smarty templates.
 *
 * For more info about smarty templates see
 * <a href='http://smarty.php.net'>smarty.php.net</a>
 *
 *
 */
class Interfaces
{
	var $language;
	var $LANG;
	var $ERROR;
	var $ERROR_NUM;
	var $COMPONENT;
	var $smarty;


	function Interfaces($type)
	{
		GLOBAL $COMPONENT,
		       $DISCOMP_CONFIG,
		       $DISCOMP_COMMUNICATOR;


		$this->COMPONENT = $COMPONENT;


		$this->setLanguage($DISCOMP_CONFIG["language"]);


		$this->smarty = new Smarty;


		$this->smarty->template_dir  = TEMPLATES_DIR;
		$this->smarty->compile_dir   = CACHE_DIR;
		$this->smarty->config_dir    = LANGUAGES_DIR;

		if ( !$DISCOMP_CONFIG["debug"] ) {
			$this->smarty->error_reporting       = 0;
		};


		$this->smarty->assign("DISCOMP_CONFIG",     $DISCOMP_CONFIG);
		$this->smarty->assign("LANGUAGES_DIR",      LANGUAGES_DIR);
		$this->smarty->assign("DEFAULT_LANGUAGE",   $DISCOMP_CONFIG["language"]);
		$this->smarty->assign("LANGUAGE",           $this->language);
		$this->smarty->assign("COMPONENT",          $COMPONENT);
		$this->smarty->assign("APP_URL",            APP_URL);
		$this->smarty->assign("JS_URL",             APP_URL."/interface/js");
		$this->smarty->assign("EXTJS_URL",          APP_URL."/interface/extjs");
		$this->smarty->assign("IMAGES_URL",         APP_URL."/interface/images");
		$this->smarty->assign("CSS_URL",            APP_URL."/interface/css");
		$this->smarty->assign("HTTP_HOST",          $_SERVER["HTTP_HOST"]);
		$this->smarty->assign("DEBUG",              $DISCOMP_CONFIG["debug"]);
		$this->smarty->assign("AUTHORIZED",         isset($_SESSION['user_login']) );
		$this->smarty->assign("USER_LOGIN",         $_SESSION['user_login'] );
		$this->smarty->assign("USER_TYPE",          $_SESSION['user_type'] );
		$this->smarty->assign("DISCOMP_VERSION",    DISCOMP_VERSION);
	}

	/**
	 * Process template and print content
	 * @param $template_file  - path to the template
	 */
	function display($template_file)
	{
		GLOBAL $COMPONENT;
		GLOBAL $DISCOMP_COMMUNICATOR;
		$this->smarty->assign("UNIQUE_KEY",         getmypid().time());
		$this->smarty->assign("COMMUNICATOR_ERROR", $DISCOMP_COMMUNICATOR->errstr);
		$this->smarty->display( TEMPLATES_DIR."/components/".$COMPONENT."/".$template_file );
	}

	/**
	 * Process template and return content
	 * @param $template_file  - path to the template
	 * @return processed content
	 */
	function display_get_html($template_file)
	{
		ob_start();
		$this->display($template_file);
		$output = ob_get_contents();
		ob_end_clean();

		return $output;
	}


	/**
	 * Assign variable for use it in the smarty templates
	 * @param $var_name - variable name
	 * @param $var_value - variable value
	 */
	function assign($var_name, $var_value)
	{
		$this->smarty->assign($var_name, DiscompSecurity::FixXSS($var_value) );
	}


	/**
	 * Set interfaces language
	 * @param $lng - language (en, fr, ru, etc.)
	 */
	function setLanguage($lng) {
		GLOBAL $CONF, $USER;

		if ( !$lng ) {
			return;
		};
	}

}

?>
