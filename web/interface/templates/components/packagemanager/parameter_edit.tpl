{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/processes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/processes.ini"}


{include file="components/packagemanager/breadcrumbs.tpl"}

<div class='header'>Edit parameter</div>

<form action='?component=packagemanager&package={$smarty.request.package}&action=do_parameter_edit' method='post'>
<input type='hidden' name='old_name' value='{$PARAM.name}'/></th>
<table border=0 style='padding:20px'>
<tr>
	<td>Name</th>
	<td><input type='input' name='name' class='form_input' value='{$PARAM.name}'/></th>
</tr>
<tr>
	<td>Type</th>
	<td>
		<select name='type' class='form_select'>
			<option value='file' {if $PARAM.type == 'file'}selected{/if}>File</option>
			<option value='filelist' {if $PARAM.type == 'filelist'}selected{/if}>FileList</option>
		</select>
	</td>
</tr>
<tr>
	<td>Description</td>
	<td><textarea name='comment' class='form_textarea'>{$PARAM.comment}</textarea></td>
</tr>
<tr>
	<td>Filename</td>
	<td><input type='input' name='filename' class='form_input' value='{$PARAM.filename}'/></td>
</tr>
<tr>
	<td>Filepattern</td>
	<td><input type='input' name='filepattern' class='form_input' value='{$PARAM.filepattern}'/></td>
</tr>
<tr>
	<td>Size</td>
	<td><input type='input' name='size' class='form_input' value='{$PARAM.size}'/></td>
</tr>
<tr>
	<td>Constant</td>
	<td><input type='checkbox' name='constant' value='1' {if $PARAM.constant == 1}checked{/if}></td>
</tr>


<tr>
	<td colspan='2' align='right'><input type='submit' value='Save'></td>
</tr>
</table>
</form>
