{config_load file="$LANGUAGES_DIR/en/main.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/main.ini"}
{if $AUTHORIZED != 1}
	<li><a href='?component=login'>{#MENU_LOGIN#}</a>
{else}
	{if $USER_TYPE == "admin"}
		<li><a href='?component=nodes'>{#MENU_NODES_MANAGER#}</a>
		<li><a href='?component=processes'>{#MENU_PROCESSES_MANAGER#}</a>
	{/if}

	{if $USER_TYPE == "developer" || $USER_TYPE == "admin"}
		<li  style='padding-top:20px'><a href='?component=packagemanager'>{#MENU_PACKAGE_MANAGER#}</a>
	{/if}

	<li  style='padding-top:20px'><a href='?component=usermanager'>{#MENU_USERS_MANAGER#}</a>
	<li><a href='?component=login&action=change_password_form'>Account</a>
	<li><a href='?component=login&action=logout'>{#MENU_LOGOUT#}</a>
{/if}
<li><a href='?component=help'>{#MENU_HELP#}</a>
