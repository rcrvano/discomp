{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/packagemanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/packagemanager.ini"}

{include file="components/packagemanager/breadcrumbs.tpl"}

<div class='header'>{#PACKAGEMANAGER_edit_scheme#}: <u>{$smarty.request.package}.{$smarty.request.scheme}</u></div>

{if $ERROR != ""}
<div class='error'> 
	{if $ERROR == "notsaved"}{#PACKAGEMANAGER_scheme_error_save#}
	{else}{#PACKAGEMANAGER_error_unknown#} : {$ERROR}{/if}
</div>
{/if}

<form action='?component=packagemanager&action=do_scheme_save&package={$smarty.request.package}&scheme={$smarty.request.scheme}' method='post'>
<table border=0 style='padding:20px' width=100%>
<tr>
<tr>
	<td><textarea name='scheme_src' class='form_textarea' style='width:100%;height:400px'>{$SCHEME_SRC}</textarea></td>
</tr>
<tr>
	<td colspan='2' align='right'><input type='submit' value='Save'><input type='button' onclick='document.location="index.php?component=packagemanager&package={$smarty.request.package}"' value='Cancel'></td>
</tr>
</table>
</form>
