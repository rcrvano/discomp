{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/nodes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/nodes.ini"}

<div class='header'>{#NODES_MODULES_INSTALL_TITLE#} (<b>{$NODE_INFO.name}</b>)</div>

<table class='content_table' width='100%'>
<tr>
	<th>{#NODES_MODULES_MODULE_INSTALL#}</th>
	<th>{#NODES_MODULES_PACKAGE_NAME#}</th>
	<th>{#NODES_MODULES_MODULE_NAME#}</th>
</tr>
{foreach from=$MODULES_LIST key=key item=module}
{if $module.installed == 0}
<tr class="{cycle values='odd,even'}">
	<td><a href='?component=nodes&action=install_module&node_id={$NODE_INFO.id}&module={$module.package}.{$module.name}'>{#NODES_MODULES_MODULE_INSTALL#}</a></td>
	<td><a href='javascript:DiscompNode.gotoPackage("{$module.package}")'>{$module.package}</a></td>
	<td><a href='javascript:DiscompNode.gotoPackage("{$module.package}.{$module.name}")'>{$module.name}</a></td>

</tr>
{assign var='modules_exists' value='1'}
{/if}
{/foreach}

{if !$modules_exists}
<tr class='odd'>
	<td colspan='3'>{#NODES_MODULES_NO_AVAIL#}</td>
</tr>
{/if}
</table>

<br>
<br>
<div class='header'>{#NODES_MODULES_INSTALLED#}</div>
<table class='content_table' width='100%'>
<tr>
	<th>{#NODES_MODULES_PACKAGE_NAME#}</th>
	<th>{#NODES_MODULES_MODULE_NAME#}</th>
</tr>
{foreach from=$MODULES_LIST key=key item=module}
{if $module.installed== 1}
<tr class="{cycle values='odd,even'}">
	<td><a href='javascript:DiscompNode.gotoPackage("{$module.package}")'>{$module.package}</a></td>
	<td><a href='javascript:DiscompNode.gotoPackage("{$module.package}.{$module.name}")'>{$module.name}</a></td>
</tr>
{assign var='installed_modules_exists' value='1'}
{/if}
{/foreach}

{if !$installed_modules_exists}
<tr class='odd'>
	<td colspan='3'>{#NODES_MODULES_NO_INSTALLED#}</td>
</tr>
{/if}

</table>
