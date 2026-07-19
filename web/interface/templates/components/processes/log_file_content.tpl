{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/processes.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/processes.ini"}


<div class='header'>Log file <u>{$smarty.request.log_file}</u> for <u>{$smarty.request.process}</u></div>

<div class='log_content'><pre>{$LOG_FILE_CONTENT}</pre></div> 
<br>
<a href='?component=processes&action=show_logs&process={$smarty.request.process}'>&laquo; Back</a>
