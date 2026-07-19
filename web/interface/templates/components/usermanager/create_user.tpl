{include file="common/header.tpl"}
{config_load file="$LANGUAGES_DIR/en/main.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/main.ini"}
{config_load file="$LANGUAGES_DIR/en/usermanager.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/usermanager.ini"}

<div class='header'>{#USERS_CREATE_NEW#}</div>

{if $ERROR != ""}
	<div class='error'><b>{#ERROR_TITLE#}</b>: {$ERROR} </div>
{/if}

<div align='left'>
	<form name='authorization_form' method='post' action='index.php' >
		<input type='hidden' name='action' value='do_create_user'>
		<input type='hidden' name='component' value='usermanager'>
			<table border='0' cellspacing="10px">
			<tr>
				<td >{#USERS_INFO_LOGIN#}:</td>
				<td><input type='input' name='login' style='width:200px' value='{$smarty.request.login}' ></td>
			</tr>
			<tr>
				<td >{#USERS_INFO_PASSWORD#}:</td>
				<td><input type='password' name='password' style='width:200px' ></td>
			</tr>
			<tr>
				<td >{#USERS_INFO_TYPE#}:</td>
				<td>
					<select name='type' style='width:200px'>
						<option value='user' {if $smarty.request.type == "user"}selected{/if}>user</option>
						<option value='developer' {if $smarty.request.type == "developer"}selected{/if}>developer</option>
					</select>
				</td>
			</tr>
			<tr>
				<td colspan='2' align='right'>
					<input type='submit' value="{#USERS_FORM_Submit#}">
				</td>
			</tr>
			</table>
	</form>
</div>

