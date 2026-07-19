// +--------------------------------------------------------------------+
// | TelinWeb Desktop                                                   |
// +--------------------------------------------------------------------+
// | Copyright (C) 2005-2008 TelinWeb Inc.                              |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author : Sidorov Ivan <ivan@sidorov.net> <yvan@artwhere.be>        |
// +--------------------------------------------------------------------+
// | File   : $RCSfile: communications.js,v $				|
// | Version: $Revision: 1.28 $						|
// | Updated: $Date: 2008/04/25 13:52:39 $				|
// +--------------------------------------------------------------------+

/**
 * @defgroup js_ajaxcommunications JS Ajax Communications
 * @brief JavaScript Ajax Communications Object. 
 * 
 * @{
 */


var AjaxCommunications = new Object();
AjaxCommunications.RequestsInStack  = 0;
AjaxCommunications.StackInitialized = false;
AjaxCommunications.maxStackCount    = 0;
AjaxCommunications.latestContent    = "";


AjaxCommunications.Init = function () 
{
	
	var ajax_comm_obj;

	if ( Common.BrowserInfo.IsIE ) {
		try {
			ajax_comm_obj=new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
				ajax_comm_obj=new ActiveXObject("Microsoft.XMLHTTP");
			} catch (oc) {
				ajax_comm_obj=null;
			};
		};
	} else if ( Common.BrowserInfo.IsGecko ) {
		ajax_comm_obj = new XMLHttpRequest();
	} else if ( Common.BrowserInfo.Opera ) {
		ajax_comm_obj = new XMLHttpRequest();
	};
	
	if (!ajax_comm_obj) {
		Dialog.errorMsg(LANG["ERROR_INIT_XMLHTTP"]);
		return null;
	};
			
	return ajax_comm_obj;
};

AjaxCommunications.RequestNum = 0;
/**
 * Base function for make a request to server. This function make a GET request.\n
 * Example:
 * \code
 * function deleteItem ( item_id, some_var1, some_var2, some_var3 )
 * {
 *     AjaxCommunications.Request( "index.php?comp=desktop&desktop_comp_action=delete_item&item_id="+item_id, 
 *                                 deleteItem_callback, some_var1, some_var2, some_var3 );
 * }
 * function deleteItem_callback ( received_content, some_var1, some_var2, some_var3 )
 * {
 *     //here you can check received_content. and some_varN - it's a variables from deleteItem function
 * }
 * \endcode
 * @sa DesktopWeblets.Request
 * @param request_str - URL for make a request to server.
 * @param callback_func - Function which will be called after receiving answer from server
 * @param addit_var1, ..., addit_var5 - also you can specify five additional variables which will be specified \n
          as additional parameters to the callback func.
 */
AjaxCommunications.Request = function ( request_str, callback_func) 
{
	var ajax_post_data = null;
	var ajax_request_type = "POST";
	var ajax_comm_obj;


	var request_str_orig = request_str;

	if (ajax_request_type == "POST") {
		var url_data = request_str.split("?");
		request_str = url_data[0];
		ajax_post_data = url_data[1];
	};
	//alert(request_str + ":\r\n" + ajax_post_data);
	
	//to make sure
	//request_str = request_str.replace(/index.php/g, (AjaxCommunications.RequestNum++) + "index.php");

	
	var ajax_arguments = arguments;
	var ajax_addit_parameters = new Array();
	for(var i = 2; i < arguments.length; i++) {
		ajax_addit_parameters.push(arguments[i]);
	}

	ajax_comm_obj = AjaxCommunications.Init();
	if ( !ajax_comm_obj ) {
		return null;
	}

	try {
		ajax_comm_obj.open(ajax_request_type, request_str, true);
	} catch (e) {
		try {
			ajax_comm_obj.open(ajax_request_type, request_str, false);
		} catch (e2) {
			Dialog.errorMsg("Could't open ajax communications object");
		}
	}
	
	if (ajax_request_type == "POST") {
		ajax_comm_obj.setRequestHeader("Method", "POST " + request_str + " HTTP/1.1");
		ajax_comm_obj.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
	}

	AjaxCommunications.RequestsInStack++;
	AjaxCommunications.StackInitialized = true;
	if ( !this.dont_show_loading ) {
		AjaxCommunications.SetLoading();
	};
	ajax_comm_obj.onreadystatechange = function() {
		if (ajax_comm_obj.readyState != 4) 
			return;
		
		AjaxCommunications.RequestsInStack--;
		if ( !this.dont_show_loading ) {
			AjaxCommunications.SetLoading();
		};

		var received_content = ajax_comm_obj.responseText;
		//fix bug for Safari
		if ( !received_content ) { received_content = ""; };

		//trim received content
		received_content = received_content.replace(/[\s|\r|\n|\t]*$/,'');
		received_content = received_content.replace(/^[\s|\r|\n|\t]*/,'');

		Debug.addMsg("URL:" + request_str_orig + "\n\rData:" + received_content);
		
		//Utilites.SearchAndExecuteJavaScripts(received_content);
		//Utilites.preloadImagesFromContent(received_content);
		
		if ( callback_func ) {
			//TODO:!!!
			switch ( ajax_addit_parameters.length ) {
				case 0:
					if ( typeof(callback_func) == "string" ) {
						AjaxCommunications.latestContent = received_content;
						eval ( callback_func + "('')" );
					} else {
						callback_func( received_content );
					}
					break;
				case 1:
					if ( typeof(callback_func) == "string" ) {
						AjaxCommunications.latestContent = received_content;
						eval ( callback_func + "('','"+ajax_addit_parameters[0]+"')" );
					} else {
						callback_func( received_content, ajax_addit_parameters[0] );
					}
					break;
				case 2:
					callback_func( received_content, ajax_addit_parameters[0], ajax_addit_parameters[1] );
					break;
				case 3:
					callback_func( received_content, ajax_addit_parameters[0], ajax_addit_parameters[1], ajax_addit_parameters[2] );
					break;

				case 4:
					callback_func( received_content, ajax_addit_parameters[0], ajax_addit_parameters[1], ajax_addit_parameters[2], ajax_addit_parameters[3] );
					break;
					
				case 5:
					callback_func( received_content, ajax_addit_parameters[0], ajax_addit_parameters[1], ajax_addit_parameters[2], ajax_addit_parameters[3], ajax_addit_parameters[4] );
					break;

				case 6:
					callback_func( received_content, ajax_addit_parameters[0], ajax_addit_parameters[1], ajax_addit_parameters[2], ajax_addit_parameters[3], ajax_addit_parameters[4], ajax_addit_parameters[5] );
					break;
				default:
					alert("too much parameters");
					break;
			};
		};
	}
	ajax_comm_obj.send(ajax_post_data);
	delete ajax_comm_obj;
};


/* For Debug */
AjaxCommunications.SetLoading = function ()
{
	/*
	if ( DESKTOP_DEBUG ) {
		if ( !$("desktop_ajax_communications") ) {
			var divEl = document.createElement('div');
			divEl.style.position = 'absolute';
			divEl.style.top      = 0;
			divEl.style.left     = 50;
			divEl.id             = "desktop_ajax_communications";
			document.body.appendChild(divEl);
		};
		$("desktop_ajax_communications").innerHTML = "Requests in cache: <b>" + AjaxCommunications.RequestsInStack + "</b>";
	};
	*/

	if ( $("desktop_ajax_communications_loading_request") ) {
		if ( AjaxCommunications.RequestsInStack ) {
			$("desktop_ajax_communications_loading_request").style.visibility = 'visible';
		} else {
			$("desktop_ajax_communications_loading_request").style.visibility = 'hidden';
		}
	};

};



/** @} */ // end of js_ajaxcommunicatons group
