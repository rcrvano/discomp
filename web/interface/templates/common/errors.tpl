{config_load file="$LANGUAGES_DIR/en/errors.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/errors.ini"}
{if $COMMUNICATOR_ERROR!= ""}
	<div class='error'><b>{#ERROR_TITLE#}</b>: {$COMMUNICATOR_ERROR}</div>
{/if}
