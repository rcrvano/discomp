{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/processes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/processes.ini"}


<div class='header'>{#PROCESSES_LIST_ACTIVE_TITLE#} (<a href='?component=processes'>{#PROCESSES_LIST_REFRESH#}</a>)</div>

<table border=0 class='content_table' width='100%'>
<tr>
	<th width='1'>{#PROCESSES_INFO_ID#}</th>
	<th width='100%'>{#PROCESSES_INFO_NAME#}</th>
	<th>{#PROCESSES_INFO_LOG#}</th>
	<th>{#PROCESSES_INFO_STATS#}</th>
	<th>{#PROCESSES_INFO_PRIORITY#}</th>
	<th>{#PROCESSES_INFO_USER#}</th>
	<th nowrap='1'>{#PROCESSES_INFO_CURRENT_STAGE#}</th>
	<th nowrap='1'>{#PROCESSES_INFO_EXEC_TIME#}</th>
</tr>
{foreach from=$ACTIVE_PROCESSES_LIST item=process key=key}
<tr class="{cycle values='odd,even'}" id='{$node.id}' style='cursor:pointer;' onClick="WidgetTable.selectRow('processes_list', this);DiscompProcesses.showProcessInfo(this)">
	<td>{$process.id}</td>
	<td>{$process.name}</td>
	<td><a href='?component=processes&action=show_logs&process={$process.name}'>{#PROCESSES_INFO_LOG_VIEW#}</a></td>
	<td><a href='?component=processes&action=show_stats&process={$process.name}'>{#PROCESSES_INFO_STATS_VIEW#}</a></td>
	<td>{$process.priority}&nbsp;</td>
	<td>{$process.user}&nbsp;</td>
	<td>{$process.current_stage}&nbsp;</td>
	<td nowrap='1'>{$process.exec_time.string}&nbsp;</td>
</tr>
{assign var='processes_exists' value='1'}
{/foreach}

{if $processes_exists != 1}
<tr class='odd'>
	<td colspan='40'>{#PROCESSES_NO_AVAIL#}</td>
</tr>
{/if}


</table>


<div class='separator'></div>

<div class='header'>Start new process</div>

<table border=0 class='content_table' width='100%'>
<tr>
	<th width='100%'>Scheme</th>
</tr>
{foreach from=$PROCESSES_LIST item=process key=key}
<tr class="{cycle values='odd,even'}" id='{$node.id}' style='cursor:pointer;' onClick="WidgetTable.selectRow('processes_list', this);DiscompProcesses.showProcessInfo(this)">
	<td><a href='?component=processes&action=prepare_start&process={$process}'>{$process}</a></td>
</tr>
{assign var='processes_exists' value='1'}
{/foreach}

{if $processes_exists != 1}
<tr class='odd'>
	<td colspan='40'>{#PROCESSES_NO_AVAIL#}</td>
</tr>
{/if}


</table>


