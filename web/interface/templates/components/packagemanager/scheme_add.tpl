{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/packagemanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/packagemanager.ini"}

{include file="components/packagemanager/breadcrumbs.tpl"}

<div class='header'>{#PACKAGEMANAGER_new_scheme_title#} : <u>{$smarty.request.package}</u></div>
<form action='?component=packagemanager&action=do_scheme_add&package={$smarty.request.package}' method='post'>

{if $ERROR != ""}
<div class='error'> 
	{if $ERROR == "exists"}{#PACKAGEMANAGER_scheme_error_exists#}
	{else}{#PACKAGEMANAGER_error_unknown#} : {$ERROR}{/if}
</div>
{/if}
<div class='content'>
<table border=0 >
<tr>
	<td>{#PACKAGEMANAGER_new_scheme_name#}:</th>
	<td><input type='input' name='scheme' value='{$smarty.request.scheme}'/></th>
</tr>
<tr>
	<td colspan='2' align='right'><input type='submit'></td>
</tr>
</table>
</div>
</form>
