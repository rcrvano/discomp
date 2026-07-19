{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/processes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/processes.ini"}


<div class='header'>Log files for: <u>{$PROCESS_NAME}</u></div>

<table border=0 class='content_table' width='100%'>
<tr>
	<th width='100%'>File</th>
	<th width='1'>Size</th>
	<th width='1'>Date</th>
</tr>
{foreach from=$LOG_FILES item=log_file key=key}
<tr class="{cycle values='odd,even'}">
	<td><a href='?component=processes&action=show_log_file&log_file={$log_file.name}&process={$PROCESS_NAME}'>{$log_file.name}</a></td>
	<td nowrap='1'>{$log_file.size}</td>
	<td nowrap='1'>{$log_file.date}</td>
</tr>
{assign var='logs_exists' value='1'}
{/foreach}

{if $logs_exists != 1}
<tr class='odd'>
	<td colspan='40'>No such log files</td>
</tr>
{/if}


</table>


