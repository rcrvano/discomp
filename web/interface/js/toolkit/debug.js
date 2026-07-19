// +--------------------------------------------------------------------+
// | TelinWeb Desktop                                                   |
// +--------------------------------------------------------------------+
// | Copyright (C) 2005-2008 TelinWeb Inc.                              |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author : Sidorov Ivan <ivan@sidorov.net> <yvan@artwhere.be>        |
// +--------------------------------------------------------------------+
// | File   : $RCSfile: debug.js,v $					|
// | Version: $Revision: 1.8 $						|
// | Updated: $Date: 2008/01/04 11:36:17 $				|
// +--------------------------------------------------------------------+

/**
 * @defgroup js_debug JS Common Debug
 * @brief JavaScript object for debug weblets and other data on myoffice
 * 
 * For use it enable debug in the config/config.php and after that you should see debug link at the top left.
 * It will open popup where by default showed all myoffice requests. You can add your own message to this popup\n
 * Example:
 * \code
 * Debug.ConsoleClear();
 * Debug.ConsoleShow();
 * Debug.addMsg("test");
 * \endcode
 *
 *  @{
 */

var Debug = new Object();

Debug.wind_obj = null;
Debug.messages = Array();

/**
 * Add message to the debug console
 * @param msg - message string
 */
Debug.addMsg = function (msg) 
{
	if ( !Debug.ConsoleDisplayed() ) {
		return;
	};
	if ( !msg ) {
		return;
	};
	msg = msg.replace(/</g,"&lt;");
	msg = msg.replace(/>/g,"&gt;");
	msg = msg.replace(/\r/g,"<br>");
	this.messages.push(msg);
	//this.ConsoleRefresh();
	
	if (  this.wind_obj && this.wind_obj.displayed() && $('desktop_debug_console_messages') ) {
		var divEl = document.createElement('div');
		divEl.style.width = '100%';
		divEl.style.borderTop = '#000000 1px solid';
		divEl.innerHTML = msg;

		$('desktop_debug_console_messages').insertBefore(divEl, $('desktop_debug_console_messages').firstChild);
	};
};

/**
 * Clear debug console
 */
Debug.ConsoleClear = function ()
{
	this.messages = new Array();
	this.ConsoleRefresh();
};

/**
 * Show debug console
 */
Debug.ConsoleShow = function ()
{
	if (  this.wind_obj  ) {
		this.wind_obj.destroy();
	};
	this.wind_obj  = new WidgetWindow(550, 350);
	this.wind_obj.setTitle("Debug");
	this.wind_obj.setPosition(700, 0);
	this.wind_obj.setPadding (0, 0, 0, 0);
	this.wind_obj.setContent (this.ConsoleGenerateContent());
	this.wind_obj.show();
}

Debug.ConsoleDisplayed = function ()
{
	return (  this.wind_obj && this.wind_obj.displayed() );

}

Debug.ConsoleGenerateContent = function ()
{
	var content = "<table width='"+this.wind_obj.width+"' border='0'><tr>"+
			"<td><a href='javascript:Debug.ConsoleClear()'>Clear</a></td>"+
			"<td width='100%'>&nbsp;</td>"+
			"<td nowrap='1'><a href='javascript:Debug.ConsoleWindIncSize()'>inc(+) size</a></td>" +
			"<td>|</td>"+
			"<td nowrap='1'><a href='javascript:Debug.ConsoleWindDecSize()'>dec(-) size</a></td>" +
			"</tr>"+
			"</table>";
	content += "<div id='desktop_debug_console_messages'>";
	for ( var i=(this.messages.length >= 25 ? 25 : this.messages.length-1) ; i>=0; i-- ) {
		content += "<div style='width:100%;border-top:#000000 1px solid;'>" + this.messages[i] + "</div>";
	}
	content += "</div>"
	content = "<div style='background:#ffffff;width:1000;'>"+content+"</div>";
	return content;
}

Debug.ConsoleRefresh = function ()
{
	if (  this.wind_obj && this.wind_obj.displayed() ) {
		this.wind_obj.setContent (this.ConsoleGenerateContent());
		this.wind_obj.show(1);
	};
};

/**
 * Hide debug console 
 */
Debug.ConsoleHide = function ()
{
	this.wind_obj.destroy();
	this.wind_obj = null;
}

/**
 * Increase debug console size on the 50px
 */
Debug.ConsoleWindIncSize = function () {
	this.wind_obj.height += 50;
	this.wind_obj.width  += 50;
	this.ConsoleRefresh();
};

/**
 * Decrease debug console size on the 50px
 */
Debug.ConsoleWindDecSize = function() {
	this.wind_obj.height -= 50;
	this.wind_obj.width  -= 50;
	this.ConsoleRefresh();
};


/** @} */ // end of js_debug group
