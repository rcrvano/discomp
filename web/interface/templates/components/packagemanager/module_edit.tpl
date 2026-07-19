{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/packagemanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/packagemanager.ini"}

{include file="components/packagemanager/breadcrumbs.tpl"}

<div class='header'>Edit module <u>{$smarty.request.package}.{$smarty.request.module}</u></div>

<table width='100%' cellspacing='30px'> 
<tr> 
<td class='content_table_border' width='50%' valign='top' align='center'> 
	<div align='center' class='package_info_header'>Info</div> 

	<form action='?component=packagemanager&package={$smarty.request.package}&module={$smarty.request.module}&action=save_module_info' method='post'>
	<table border=0 style='padding:20px'>
	<tr>
		<td>Name</td>
		<td><input type='input' name='name' class='form_input' value='{$MODULE.info.name}' disabled/></td>
	</tr>
	<tr>
		<td>Description</td>
		<td><textarea name='comment' class='form_textarea'>{$MODULE.info.comment}</textarea></td>
	</tr>
	<!--
	<tr>
		<td>Version</td>
		<td><input type='input' name='version' class='form_input'/></th>
	</tr>
	<tr>
		<td>Type</th>
		<td>
			<select name='type' class='form_select'>
				<option value='file'>Binary</option>
				<option value='filelist'>Sources</option>
			</select>
		</td>
	</tr>
	<tr>
		<td>OS</th>
		<td>
			<select name='type' class='form_select' multiple>
				<option value='file'>Windows x86</option>
				<option value='file'>Windows x86_64</option>
				<option value='filelist'>Linux x86</option>
				<option value='filelist'>Linux x86_64</option>
				<option value='filelist'>Mac OS X</option>
			</select>
		</td>
	</tr>
-->	
	<tr>
		<td>Start cmd</td>
		<td><input type='input' name='start_cmd' class='form_input' value='{$MODULE.commands.start.cmd}'/></td>
	</tr>
	<tr>
		<td>Stop cmd</td>
		<td><input type='input' name='stop_cmd' class='form_input' value='{$MODULE.commands.stop.cmd}'/></td>
	</tr>
	
	<tr>
		<td>Plugin</td>
		<td><input type='input' name='plugin' class='form_input' value='{$MODULE.plugin}'/></td>
	</tr>

	
	
	<tr>
		<td colspan='2' align='right'><input type='submit' value='Save'></td>
	</tr>
	</table>
	</form>
</td>
<td class='content_table_border' width='50%' valign='top'> 

<!-- ********************************** FILES ********************************** -->

	<div align='center' class='package_info_header'>Files list</div> 
	<div class='content' style='padding:5px'> 
		<a href='javascript:document.getElementById("module_upload_file_area").style.display="block"'>Upload</a> | 
		<a href='javascript:deleteSelectedModuleFile()'>Delete</a> 
	</div> 


	<script>
		function deleteSelectedModuleFile ( ) {ldelim}
			if ( WidgetTable.getSelectedRows('module_files_list')  ) 
				document.location="?component=packagemanager&package={$smarty.request.package}&module={$smarty.request.module}&action=delete_module_file&file="+WidgetTable.getSelectedRows('module_files_list');
		{rdelim}

	</script>

	<div style='display:none' id='module_upload_file_area'>
		<form action='?component=packagemanager&action=do_module_upload_file&package={$smarty.request.package}&module={$smarty.request.module}' method='post' style='margin:0px;padding:0px' enctype="multipart/form-data">
		<table>
		<tr>
		<td>
			<input type='file' class='form_file' name='file'>
		</td>
		<td>
			<input type='submit' value='Upload &raquo;' class='form_button'>
		</td>
		</tr>
		</table>
		</form>
	</div>

 
	<table border=0 class='content_table' style='border:0px' width='100%'> 
	<tr>
		<th width='100%'>{#PACKAGEMANAGER_Name#}</th>
		<th width='100%'>{#PACKAGEMANAGER_Size#}</th>
		<th width='100%'>{#PACKAGEMANAGER_Date#}</th>
	</tr>
	
	{foreach from=$MODULE_FILES item=file key=key}
	<tr class="{cycle values='odd,even'}" id='{$file.name}' style='cursor:pointer;' onClick="WidgetTable.selectRow('module_files_list', this, true);">
		<td>{$file.name}</td>
		<td nowrap='1'>{$file.size}</td>
		<td nowrap='1'>{$file.date}</td>
	</tr>
	{assign var='files_exists' value='1'}
	{/foreach}
	
	{if $files_exists != 1}
	<tr class='odd'>
		<td colspan='40'>{#PACKAGEMANAGER_Parameters_empty#}</td>
	</tr>
	{/if}
	
	</table> 
	


</td>
</tr>








<tr> 
<td class='content_table_border' width='50%' valign='top'> 
	<div align='center' class='package_info_header'>Input parameters</div> 
	<div class='content' style='padding:5px'> 
		<a href='javascript:document.getElementById("input_parameters_area").style.display="block"'>Add</a> | 
		<a href='javascript:deleteSelectedInputParameter()'>Delete</a> 
	</div> 

	<script>
		function deleteSelectedInputParameter ( ) {ldelim}
			if ( WidgetTable.getSelectedRows('parameters_input_list')  ) 
				document.location="?component=packagemanager&package={$smarty.request.package}&module={$smarty.request.module}&action=delete_module_parameter&input_parameter="+WidgetTable.getSelectedRows('parameters_input_list');
		{rdelim}

	</script>

	<div style='display:none' id='input_parameters_area'>
		<form action='?component=packagemanager&action=do_module_add_parameter&package={$smarty.request.package}&module={$smarty.request.module}' method='post' style='margin:0px;padding:0px'>
		<table>
		<tr>
		<td>
			<select class='form_select' name='input_parameter'>
				<option value=''>Select parameter</option>
				{foreach from=$PACKAGE_PARAMETERS item=parameter key=key}
					{assign var='input_param_exists' value='0'}
					{foreach from=$MODULE.parameters.input item=exists_parameter key=exists_key}
						{if $parameter.name == $exists_parameter.name}
							{assign var='input_param_exists' value='1'}
						{/if}
					{/foreach}
					{if $input_param_exists != 1}
						<option value='{$parameter.name}'>{$parameter.name}</option>
					{/if}
				{/foreach}
			</select>
		</td>
		<td>
			<input type='submit' value='Add &raquo;' class='form_button'>
		</td>
		</tr>
		</table>
		</form>
	</div>

 
	<table border=0 class='content_table' style='border:0px' width='100%'> 
	<tr>
		<th width='100%'>{#PACKAGEMANAGER_Name#}</th>
		<th width='100%'>{#PACKAGEMANAGER_Type#}</th>
	</tr>
	
	{foreach from=$MODULE.parameters.input item=parameter key=key}
	<tr class="{cycle values='odd,even'}" id='{$parameter.name}' style='cursor:pointer;' onClick="WidgetTable.selectRow('parameters_input_list', this, true);">
		<td>{$parameter.name}</td>
		<td>{$parameter.type}</td>
	</tr>
	{assign var='parameters_exists' value='1'}
	{/foreach}
	
	{if $parameters_exists != 1}
	<tr class='odd'>
		<td colspan='40'>{#PACKAGEMANAGER_Parameters_empty#}</td>
	</tr>
	{/if}
	
	</table> 
	
	
</td> 
<td class='content_table_border' width='50%' valign='top'> 
 
	<div align='center' class='package_info_header'>Output parameters</div> 

	<div class='content' style='padding:5px'> 
		<a href='javascript:document.getElementById("output_parameters_area").style.display="block"'>Add</a> | 
		<a href='javascript:deleteSelectedOutputParameter()'>Delete</a> 
	</div> 

	<script>
		function deleteSelectedOutputParameter ( ) {ldelim}
			if ( WidgetTable.getSelectedRows('parameters_output_list')  ) 
				document.location="?component=packagemanager&package={$smarty.request.package}&module={$smarty.request.module}&action=delete_module_parameter&output_parameter="+WidgetTable.getSelectedRows('parameters_output_list');
		{rdelim}

	</script>

	<div style='display:none' id='output_parameters_area'>
		<form action='?component=packagemanager&action=do_module_add_parameter&package={$smarty.request.package}&module={$smarty.request.module}' method='post' style='margin:0px;padding:0px'>
		<table>
		<tr>
		<td>
			<select class='form_select' name='output_parameter'>
				<option value=''>Select parameter</option>
				{foreach from=$PACKAGE_PARAMETERS item=parameter key=key}
					{assign var='output_param_exists' value='0'}
					{foreach from=$MODULE.parameters.output item=exists_parameter key=exists_key}
						{if $parameter.name == $exists_parameter.name}
							{assign var='output_param_exists' value='1'}
						{/if}
					{/foreach}
					{if $output_param_exists != 1}
						<option value='{$parameter.name}'>{$parameter.name}</option>
					{/if}
				{/foreach}
			</select>
		</td>
		<td>
			<input type='submit' value='Add &raquo;' class='form_button'>
		</td>
		</tr>
		</table>
		</form>
	</div>

 
	<table border=0 class='content_table' style='border:0px' width='100%'> 
	<tr>
		<th width='100%'>{#PACKAGEMANAGER_Name#}</th>
		<th width='100%'>{#PACKAGEMANAGER_Type#}</th>
	</tr>
	
	{foreach from=$MODULE.parameters.output item=parameter key=key}
	<tr class="{cycle values='odd,even'}" id='{$parameter.name}' style='cursor:pointer;' onClick="WidgetTable.selectRow('parameters_output_list', this, true);">
		<td>{$parameter.name}</td>
		<td>{$parameter.type}</td>
	</tr>
	{assign var='parameters_exists' value='1'}
	{/foreach}
	
	{if $parameters_exists != 1}
	<tr class='odd'>
		<td colspan='40'>{#PACKAGEMANAGER_Parameters_empty#}</td>
	</tr>
	{/if}
	
	</table> 
	

</td> 
</tr> 
</table>
