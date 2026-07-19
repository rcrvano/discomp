{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/nodes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/nodes.ini"}

<div class='header'>{#NODES_Title#} (<a href='?component=nodes'>{#NODES_Refresh#}</a>)</div>


<table>
        <tr>
                <td><b>{#NODES_SYSINFO_MONITOR#}:</b></td>
                <td>
                    <div id="chart-win-shortcut">
                        <a href="#">
                                <div>Show monitor</div>
                        </a>
                    </div>
                </td>
        </tr>
</table>


<table border=0 class='content_table' width='100%'>
<tr>
	<th width='1'>{#NODES_INFO_ID#}</th>
	<th width='1'>{#NODES_INFO_BUSY#}</th>
	<th>{#NODES_INFO_IP#}</th>
	<th>{#NODES_INFO_NAME#}</th>
	<th width='1'>{#NODES_INFO_PING#}</th>
	<th>{#NODES_INFO_PROCESS#}</th>
	<th>{#NODES_INFO_MODULE#}</th>
	<th width='1'>{#NODES_INFO_EXEC_TIME#}</th>
	<th width='1'>{#NODES_INFO_UPTIME#}</th>
</tr>
{foreach from=$NODES_LIST item=node key=key}
<tr class="{cycle values='odd,even'}" id='{$node.id}' style='cursor:pointer;' onClick="WidgetTable.selectRow('nodes_list', this);DiscompNodes.showNodeInfo(this)">
	<td>{$node.id}</td>
	<td style='background:{if $node.busy}#f22222{else}#37e637{/if}'>&nbsp;</td>
	<td>{$node.ip}</td>
	<td>{$node.name}</td>
	<td>{$node.ping}&nbsp;</td>
	<td>{$node.process_name}&nbsp; </td>
	<td>{$node.module_name} {if $node.module_list_num}[el:{$node.module_list_num}]{/if}&nbsp; </td>
	<td>{$node.module_exec_time}&nbsp;</td>
	<td nowrap='1'>{$node.uptime.string}&nbsp;</td>
</tr>
{assign var='nodes_exists' value='1'}
{/foreach}

{if $nodes_exists != 1}
<tr class='odd'>
	<td colspan='40'>{#NODES_NO_AVAIL#}</td>
</tr>
{/if}

</table>

