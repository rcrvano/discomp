{config_load file="$LANGUAGES_DIR/en/nodes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/nodes.ini"}

{strip}
<table class='empty_table' width='100%'>
<tr>
<td width='50%' valign='top'>
	<table class='empty_table'>
	<tr>
		<td colspan='2' align='center'>
			<b>{#NODES_SYSINFO_TITLE#} 
			(<a href='javascript:DiscompNodes.updateNodeSysInfo({$NODE_INFO.id})'>{#NODES_SYSINFO_UPDATE#}</a>
			&nbsp;|&nbsp;
			<a href='javascript:DiscompNodes.showMonitor({$NODE_INFO.id})'>Monitor</a> )</b>
		</td>
	</tr>
	<tr>
		<td><b>{#NODES_CLIENT_VERSION#}:</b></td>
		<td>{$NODE_INFO.client_version}</td>
	</tr>

	<tr>
		<td><b>{#NODES_SYSINFO_SYSTEM#}:</b></td>
		<td>{$NODE_INFO.sysinfo.sys_name} {$NODE_INFO.sysinfo.sys_version} {$NODE_INFO.sysinfo.sys_machine} {$NODE_INFO.sysinfo.sys_description}</td>
	</tr>
	<tr>
		<td><b>{#NODES_SYSINFO_CPU#}:</b></td>
		<td>{$NODE_INFO.sysinfo.cpu_mhz} MHz (Idle: {$NODE_INFO.sysinfo.cpu_idle_pct}%)</td>
	</tr>
	<tr>
		<td><b>{#NODES_SYSINFO_CPU_INFO#}:</b></td>
		<td>{$NODE_INFO.sysinfo.cpu_vendor} {$NODE_INFO.sysinfo.cpu_model}</td>
	</tr>
	<tr>
		<td><b>{#NODES_SYSINFO_MEMORY#}:</b></td>
		<td>{$NODE_INFO.sysinfo.mem_total_readable} (Free: {$NODE_INFO.sysinfo.mem_free_pct}%)</td>
	</tr>
	<tr>
		<td><b>{#NODES_SYSINFO_LOAD_AVG#}:</b></td>
		<td>{$NODE_INFO.sysinfo.stat_loadavg1}, {$NODE_INFO.sysinfo.stat_loadavg2}, {$NODE_INFO.sysinfo.stat_loadavg3}</td>
	</tr>
	<tr>
		<td><b>{#NODES_SYSINFO_UPTIME#}:</b></td>
		<td>{$NODE_INFO.sysinfo.uptime.string}</td>
	</tr>
	</table>
</td>
<td style='border-left:#aaaaaa 1px solid;' width='50%' valign='top'>
	<table class='empty_table'>
	<tr>
		<td colspan='2' align='center'>
			<b>{#NODES_MODULES_TITLE#} (<a href='?component=nodes&action=show_install_module&node_id={$NODE_INFO.id}'>{#NODES_MODULES_INSTALL#}</a>)</b>
		</td>
	</tr>
	
	<tr>
		<td valign='top'><b>{#NODES_MODULES_INSTALLED#}:</b></td>
		<td>
			{foreach from=$NODE_INFO.modules_list item=module key=key}
				{if $key!=0},<br>{/if}&nbsp;
				<a href='javascript:DiscompNodes.showModule("{$module}")'>{$module}</a>
			{/foreach}
		</td>
	</tr>
	{if $NODE_INFO.failed_modules_list_count > 0}
	<tr>
		<td valign='top'><b>{#NODES_MODULES_FAILED#}:</b></td>
		<td>
			{foreach from=$NODE_INFO.failed_modules_list item=module key=key}
				{if $key!=0},<br>{/if}&nbsp;
				<a href='javascript:DiscompNodes.showModule("{$module}")'>{$module}</a>
				&nbsp;
				<a href='javascript:DiscompNodes.deleteFailedModule("{$NODE_INFO.id}","{$module}")'><img border='0' src='{$IMAGES_URL}/delete.gif'></a> 
			{/foreach}
		</td>
	</tr>
	{/if}
	
	</table>
</td>
</tr>
</table>

{*
<table>
{foreach from=$NODE_INFO.sysinfo item=item key=key}
<tr>
	<td><b>{$key}</b></td>
	<td>{$item}</td>
</tr>
{/foreach}
</table>
*}
{/strip}
