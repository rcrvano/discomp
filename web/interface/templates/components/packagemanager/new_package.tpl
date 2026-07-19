{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/packagemanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/packagemanager.ini"}

{include file="components/packagemanager/breadcrumbs.tpl"}

<div class='header'>{#PACKAGEMANAGER_new_title#}</div>
<form action='?component=packagemanager&action=do_create_new' method='post'>
{if $ERROR != ""}
<div class='error'> 
	{if $ERROR == "exists"}{#PACKAGEMANAGER_error_exists#}
	{else}{#PACKAGEMANAGER_error_unknown#} : {$ERROR}{/if}
</div>
{/if}
<div class='content'>
<table border=0 >
<tr>
	<td>{#PACKAGEMANAGER_new_name#}:</th>
	<td><input type='input' name='package_name' value='{$smarty.request.package_name}'/></th>
</tr>
<tr>
	<td colspan='2' align='right'><input type='submit'></td>
</tr>
</table>
</div>
</form>
