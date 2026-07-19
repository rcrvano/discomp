{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/login.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/login.ini"}

<div class='header'>{#LOGIN_FORM_Title#}</div>

{if $ERROR_CODE != ""}
	<div class='error'><b>{#ERROR_TITLE#}</b>:
		{if $ERROR_CODE == "110"}{#LOGIN_FORM_ERROR_authorization#}{/if}
	</div>
{/if}


<div align='center'>
	<form name='authorization_form' method='post' action='index.php' >
		<input type='hidden' name='action' value='auth'>
		<input type='hidden' name='component' value='login'>
			<table border='0' cellspacing="10px">
			<tr>
				<td >{#LOGIN_FORM_Login#}:</td>
				<td><input type='input' name='login' ></td>
			</tr>
			<tr>
				<td >{#LOGIN_FORM_Password#}:</td>
				<td><input type='password' name='password' ></td>
			</tr>
			<tr>
				<td colspan='2' align='right'>
					<input type='submit' value="{#LOGIN_FORM_Submit#}">
				</td>
			</tr>
			</table>
	</form>
</div>
