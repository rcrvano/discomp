{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/log.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/log.ini"}

<div class='header'>{#LOGS_Title#} (<a href='?component=logs'>{#LOGS_Refresh#}</a>)</div>

<div class='log-area' id='discomp-log-area'>
{$FILE_CONTENT}
</div>
</center>
<hr>
1 | 1 |:w

<script>
DiscompLog.initArea('discomp-log-area');
</script>
