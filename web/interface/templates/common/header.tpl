<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
{config_load file="$LANGUAGES_DIR/en/main.ini"}
{config_load file="$LANGUAGES_DIR/$LANGUAGE/main.ini"}
<html>
<head>
	<title>{#WINDOW_TITLE#}</title>

	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

	<link rel='stylesheet' type='text/css' href='{$CSS_URL}/main.css' />
	<link rel="shortcut icon" href="{$IMAGES_URL}/favicon.ico"/>

	<script src='{$JS_URL}/toolkit/event.js'></script>
	<script src='{$JS_URL}/toolkit/table.js'></script>
	<script src='{$JS_URL}/toolkit/utilites.js'></script>
	<script src='{$JS_URL}/toolkit/common.js'></script>
	<script src='{$JS_URL}/toolkit/ajax.js'></script>
	<script src='{$JS_URL}/toolkit/debug.js'></script>
	<script src='{$JS_URL}/toolkit/aculous/prototype.js'></script>
	<script src='{$JS_URL}/toolkit/aculous/scriptaculous.js'></script>
	<script src='{$JS_URL}/toolkit/aculous/unittest.js'></script>



	<script src='{$JS_URL}/nodes.js'></script>
	<script src='{$JS_URL}/log.js'></script>
	<script src='{$JS_URL}/init.js'></script>
</head>

<body bgColor="#F7F7F7" leftmargin="0" topmargin="0" marginwidth="0" marginheight="0" width="100%" height="100%" >	

<table border='0' height='100%' width='100%' cellspacing='0' cellpadding='0'>
<!-- header & logo -->
<tr width='100%' style='background:#767d8c;'>
	<td align='center'><img src='{$IMAGES_URL}/logo.png' height='65px'></td>
	<td width='100%' nowrap='1' align='left' height='60' style='padding:10px 10px 0px 10px'>
		<table border='0'>
		<tr>
			<td>
			<div style='color:#e1e1e1;font:31px Verdana;'>
				<b>DISCOMP</b>
				<b style='font:22px Verdana;'><i> - Distributed Computing System of Modular Programming</i></b>
			</div>
			</td>
		</tr>
		<tr>
			<td style='color:#e1e1e1;font:10px Verdana;' valign='bottom' align='right' nowrap=1>{#DISCOMP_VERSION#}: {$DISCOMP_VERSION}</td>
		</tr>
		</table>
	</td>
</tr>
<tr>
	<td colspan='3' background='{$IMAGES_URL}/top_separator.gif' height='18' width='100%' ></td>
</tr>
<!-- / header & logo -->
<tr height='100%'>
	<td width='175px' style='background:#e6e6e6;border-right:#a1a2a3 1px solid;width:175px' valign='top'>
		<img src='{$IMAGES_URL}/empty.gif' height='1px' width='175px'>
		<ul class='menu'>
			<!-- menu -->
			{include file="common/menu.tpl"}
			<!-- / menu -->
		</ul>
	</td>
	<td colspan='2' width='100%' valign='top' style='width:100%;background:#ececec'>
	<div style='width:100%;height:15px;background:#ececec;'></div>
		<!-- main content -->
		{include file='common/errors.tpl'}

