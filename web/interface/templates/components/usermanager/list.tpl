{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/usermanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/usermanager.ini"}

<div class='header'>{#USERS_TITLE#}</div>
<div class='content' style='padding:5px'>
	<a href='?component=usermanager&action=show_create_user'>{#USERS_CREATE_NEW#}</a>
</div>

<table border=0 class='content_table' width='100%'>
<tr>
	<th width='100%' >{#USERS_INFO_LOGIN#}</th>
	<th >{#USERS_INFO_TYPE#}</th>
</tr>
{foreach from=$USERS_LIST item=user key=key}
<tr class="{cycle values='odd,even'}" id='{$user.id}' >
	<td>{$user.login}&nbsp;</td>
	<td>{$user.type}&nbsp;</td>
</tr>
{assign var='users_exists' value='1'}
{/foreach}

{if $users_exists != 1}
<tr class='odd'>
	<td colspan='40'>{#USERS_NO_AVAIL#}</td>
</tr>
{/if}

</table>

