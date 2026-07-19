{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/nodes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/nodes.ini"}

<div class='header'>Node monitor</div>

{include file="components/monitor/menu.tpl"}

<script src='{$JS_URL}/monitor/node-filesystem.js'></script>


<div id='graph-area'></div>
