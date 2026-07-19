{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/processes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/processes.ini"}


<div class='header'>Input parameters for: <u>{$smarty.request.process}</u></div>

<form method='post' action='?component=processes&action=start&process={$smarty.request.process}' >
<table border=0 class='content_table' width='100%'>
<tr>
</a></td>
	<th width='20%'>Parameter</th>
	<th width='100%'>Value</th>
</tr>
{foreach from=$INPUT_PARAMETERS item=param key=key}
<tr class="{cycle values='odd,even'}">
	<td nowrap='1'>{$param.name}</td>
	<td nowrap='1'>
		<textarea name='param_{$param.name}' style='width:100%; height:100px;'>{$param.value}</textarea>
	</td>
</tr>
{assign var='logs_exists' value='1'}
{/foreach}

{if $logs_exists != 1}
<tr class='odd'>
	<td colspan='40'>No such log files</td>
</tr>
{/if}


</table>

<div align='right'>
<input type='submit' value='Save & start'>
</div>
</form>
