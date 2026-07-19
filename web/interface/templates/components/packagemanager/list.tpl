{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/processes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/processes.ini"}

<div class='header'>Package manager</div>

<div class='content' style='padding:5px'>
	<a href='?component=packagemanager&action=create_new'>Create new package</a>
</div>

<table border=0 class='content_table' width='100%'>
<tr>
	<th width='100%'>Name</th>
</tr>
{foreach from=$PACKAGES_LIST item=package key=key}
<tr class="{cycle values='odd,even'}">
	<td><a href='?component=packagemanager&package={$package}'>{$package}</a></td>
</tr>
{assign var='packages_exists' value='1'}
{/foreach}

{if $packages_exists != 1}
<tr class='odd'>
	<td colspan='40'>No packages</td>
</tr>
{/if}


</table>


