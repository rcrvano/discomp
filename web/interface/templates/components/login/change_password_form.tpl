{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/login.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/login.ini"}


<div class='header'>{#LOGIN_FORM_chpass_title#}</div>

		{if $ERROR_CODE != ""}
			<div class='error'><b>{#ERROR_TITLE#}</b>:
				{if $ERROR_CODE == "111"}{#LOGIN_FORM_ERROR_chpass_wrong#}{/if}
				{if $ERROR_CODE == "112"}{#LOGIN_FORM_ERROR_chpass_failed#}{/if}
				{if $ERROR_CODE == "113"}{#LOGIN_FORM_ERROR_chpass_old_incorrect#}{/if}
			</div>
		{/if}

		

		<form action='index.php' method='POST'>
			<input type='hidden' name='component' value='login'/>
			<input type='hidden' name='action' value='do_change_password'/>
			<table border='0' cellspacing="10px">
			<tr>
				<td>{#LOGIN_FORM_old_password#}:</td>
				<td><input type='password' name='old_password' ></td>
			</tr>
			<tr>
				<td>{#LOGIN_FORM_new_password#}:</td>
				<td><input type='password' name='password' ></td>
			</tr>
			<tr>
				<td colspan='2' align='right'>
					<input type='submit' value='{#LOGIN_FORM_chpass_submit#}'/>
				</td>
			</tr>
			</table>

		</form>

