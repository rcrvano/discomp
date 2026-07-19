{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/usermanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/usermanager.ini"}

<div class='header'>{#USERS_CREATE_NEW#}</div>

<div class='content'>
	{#USERS_CREATE_USER_SUCCESS#}
	<br>
	<a href='?component=usermanager'>{#USERS_BACK_TO_USERS_LIST#}</a>
</div>
