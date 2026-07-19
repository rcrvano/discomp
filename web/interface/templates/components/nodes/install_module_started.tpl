{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/nodes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/nodes.ini"}

<div class='header'>{#NODES_MODULES_INSTALL_TITLE#} (<b>{$smarty.request.module}</b>)</div>

<div class='content'>
	{#NODES_MODULES_INSTALL_STARTED#}
	<br>
	<a href='?component=nodes'>{#NODES_GOTO_NODES_LIST#}</a>
</div>
