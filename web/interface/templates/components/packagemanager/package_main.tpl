{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/packagemanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/packagemanager.ini"}

{include file="components/packagemanager/breadcrumbs.tpl"}

<style>
	TD {ldelim} color: #000 !important; {rdelim}
</style>

<div class='header'>Package: <u>{$smarty.request.package}</u></div>


<table width='100%' cellspacing='20px'>
<tr>
<td class='content_table_border' width='50%' valign='top'>
	<script>
		function editSelectedParameter ( ) {ldelim}
			if ( WidgetTable.getSelectedRows('parameters_list')  ) 
				document.location="?component=packagemanager&package={$smarty.request.package}&action=parameter_edit&parameter="+WidgetTable.getSelectedRows('parameters_list');
			
		{rdelim}
		function deleteSelectedParameter ( ) {ldelim}
			if ( WidgetTable.getSelectedRows('parameters_list')  ) 
				document.location="?component=packagemanager&package={$smarty.request.package}&action=parameter_delete&parameter="+WidgetTable.getSelectedRows('parameters_list');
			
		{rdelim}

	</script>

	<div align='center' class='package_info_header'>{#PACKAGEMANAGER_Parameters#}</div>

	<div class='content' style='padding:5px'>
		<a href='?component=packagemanager&package={$smarty.request.package}&action=parameter_edit'>{#PACKAGEMANAGER_Add#}</a> | 
		<a href='javascript:editSelectedParameter()'>{#PACKAGEMANAGER_Edit#}</a> | 
		<a href='javascript:deleteSelectedParameter()'>{#PACKAGEMANAGER_Delete#}</a>
	</div>

	<table border=0 class='content_table' style='border:0px' width='100%'>
	<tr>
		<th width='100%'>{#PACKAGEMANAGER_Name#}</th>
		<th width='100%'>{#PACKAGEMANAGER_Type#}</th>
	</tr>
	
	{foreach from=$PARAMETERS_LIST item=parameter key=key}
	<tr class="{cycle values='odd,even'}" id='{$parameter.name}' style='cursor:pointer;' onClick="WidgetTable.selectRow('parameters_list', this, true);" onDblClick="WidgetTable.selectRow('parameters_list', this, true);editSelectedParameter ( )">
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
	<script>
		function editSelectedModule ( ) {ldelim}
			if ( WidgetTable.getSelectedRows('modules_list')  ) 
				document.location="?component=packagemanager&package={$smarty.request.package}&action=module_edit&module="+WidgetTable.getSelectedRows('modules_list');
		{rdelim}
	</script>



	<div align='center' class='package_info_header'>{#PACKAGEMANAGER_Modules#}</div>

	<div class='content' style='padding:5px'>
		<a href='?component=packagemanager&package={$smarty.request.package}&action=module_add'>{#PACKAGEMANAGER_Add#}</a> | 
		<a href='javascript:editSelectedModule()'>{#PACKAGEMANAGER_Edit#}</a> | 
		<a href='?component=packagemanager&package={$smarty.request.package}&action=module_edit'>{#PACKAGEMANAGER_Delete#}</a>
	</div>

	<table border=0 class='content_table' style='border:0px' width='100%'>
	<tr>
		<th width='100%'>{#PACKAGEMANAGER_Name#}</th>
		<th width='100%'>{#PACKAGEMANAGER_Type#}</th>
	</tr>

	{foreach from=$MODULES_LIST item=module key=key}
	<tr class="{cycle values='odd,even'}" id='{$module}' style='cursor:pointer;' onClick="WidgetTable.selectRow('module_list', this, true);" onDblClick="WidgetTable.selectRow('modules_list', this, true);editSelectedModule ( )">
		<td>{$module}</td>
		<td>file</td>
	</tr>
	{assign var='modules_exists' value='1'}
	{/foreach}
	
	{if $modules_exists != 1}
	<tr class='odd'>
		<td colspan='40'>{#PACKAGEMANAGER_Modules_empty#}</td>
	</tr>
	{/if}
	
	
	</table>
	
</td>
</tr>


<tr>
<td colspan='2' class='content_table_border'>
	<script>
		function editSelectedScheme ( ) {ldelim}
			if ( WidgetTable.getSelectedRows('schemes_list')  ) 
				document.location="?component=packagemanager&package={$smarty.request.package}&action=scheme_edit&scheme="+WidgetTable.getSelectedRows('schemes_list');
		{rdelim}
	</script>

	<div align='center' class='package_info_header'>{#PACKAGEMANAGER_Schemes#}</div>

	<div class='content' style='padding:5px'>
		<a href='?component=packagemanager&package={$smarty.request.package}&action=scheme_add'>{#PACKAGEMANAGER_Add#}</a> | 
		<a href='javascript:editSelectedScheme()'>{#PACKAGEMANAGER_Edit#}</a> | 
		<a href='?component=packagemanager&package={$smarty.request.package}&action=scheme_delete'>{#PACKAGEMANAGER_Delete#}</a>
	</div>

	<table border=0 class='content_table' style='border:0px' width='100%'>
	<tr>
		<th width='100%'>{#PACKAGEMANAGER_Name#}</th>
	</tr>
	
	{foreach from=$SCHEMES_LIST item=scheme key=key}
	<tr class="{cycle values='odd,even'}" id='{$scheme}' style='cursor:pointer;' onClick="WidgetTable.selectRow('schemes_list', this, true);" onDblClick="WidgetTable.selectRow('schemes_list', this, true);editSelectedScheme ( )">
		<td>{$scheme}</td>
	</tr>
	{assign var='schemes_exists' value='1'}
	{/foreach}
	
	{if $schemes_exists != 1}
	<tr class='odd'>
		<td colspan='40'>{#PACKAGEMANAGER_Schemes_empty#}</td>
	</tr>
	{/if}
	
	
	</table>


</td>
</tr>
</table>
