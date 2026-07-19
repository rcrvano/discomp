{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/log.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/log.ini"}

<div class='header'>{#LOGS_Title#} (<a href='?component=logs'>{#LOGS_Refresh#}</a>)</div>

<table border=0 class='content_table' width='100%'>
<tr>
	<th width='100%'>{#LOGS_INFO_ID#}</th>
	<th width='1'>{#LOGS_INFO_ID#}</th>
</tr>
{foreach from=$LOG_FILES item=file key=key}
<tr class="{cycle values='odd,even'}" id='{$log.id}' style='cursor:pointer;'>
	<td><a href='?comp=log&action=view&dir={$LOG_DIR}&file={$file.name}'>{$file.name}</a></td>
	<td nowrap='1'>{$file.size}</td>
</tr>
{assign var='logs_exists' value='1'}
{/foreach}

{if $logs_exists != 1}
<tr class='odd'>
	<td colspan='40'>{#LOGS_NO_AVAIL#}</td>
</tr>
{/if}

</table>

