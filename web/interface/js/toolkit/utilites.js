// +----------------------------------------------------------------------+
// | Content Manager System (CMS)                                         |
// +----------------------------------------------------------------------+
// | JavaScript utilites                                                  |
// +----------------------------------------------------------------------+
// | Version 0.09                                                         |
// +----------------------------------------------------------------------+
// | Author: Sidorov Ivan <vano@sidorov.net>                              |
// +----------------------------------------------------------------------+

/**
 * @defgroup js_utilites JS Common Utilites
 * @brief JavaScript utilites
 *  @{
*/



var Utilites = new Object();

/**
* Function for view object subclasses and class propertyes (recursive)\n
* Example : \code Utilites.viewObj(window,3,0,'window'); \endcode
* @param obj - object which you want to view
* @param depth - recursion depth (if object have class inheritance)
* @param objName (optional) - name of object. This parameter used for obviousness.
* @param currdepth (optional) = 0
*/

Utilites.viewObj = function(obj,depth,objName,currdepth) {
	if (!objName)   { var objName   = "this"; };
	if (!currdepth) { var currdepth = 0;  };
	if (currdepth > depth || currdepth > 50 )     { return 0 ; };
	for (var prop in obj) {
		if( typeof obj[prop] == 'object' ) {
			if ( prop != 'parentNode' &&  prop != 'lang' &&  prop != "nextSibling" && prop != "style" && prop != "attributes" && prop != "ownerDocument")
			eval(Utilites.viewObj(obj[prop],depth,objName + "." + prop,currdepth+1));
		} else {
			var msg	= "";
			try {
				msg += objName + "." + prop + " = " + obj[prop];
				if (!confirm(msg + "\r\n\r\nClick 'Cancel' if you want to left this object?")) { return 0; };
			} catch(e) { 
				simpleerror("Could't get method or property of object"); 
			};
		};
	};
	return 1;
};


/**
* Clone simple JS objects (geted from Mozilla editor utilites)
* @param obj - object which must be cloned
* @return - cloned object
*
* Example:
* \code 
* var cloneElement = Utilites.cloneObj($('test_obj');
* \endcode
*/

Utilites.cloneObj = function(obj)
{ 
	var clone = {};
	for (var i in obj) {
		if( typeof obj[i] == 'object')
			clone[i] = cloneObj(obj[i]);
		else
			clone[i] = obj[i];
	};
	return clone;
};

/**
* Some time this funtion works best than Utilites.cloneObj
*/
Utilites.cloneDocumentElement = function (element) 
{
	var new_el = document.createElement(element.nodeName);
	new_el.id  = element.id;
	new_el.className     = element.className;
	new_el.style.cssText = element.style.cssText;
	new_el.onclick       = element.getAttribute('onclick'); 
	new_el.setAttribute('onclick', element.getAttribute('onclick')); 
	new_el.innerHTML     = element.innerHTML;
	return new_el;
};

/**
* @return broser name - (Opera, IE, Gecko)
*/
Utilites.getBrowserName = function() 
{
//	this.dom	= $?1:0;
//	this.opera	= window.opera?1:0;
//	this.ns4	= (document.layers && !this.dom)?1:0;
//	this.ie4	= (document.all && !this.dom)?1:0;
//	return this;
	if ( window.opera ) {
		return "Opera";
	} else if (window.navigator.appName == "Microsoft Internet Explorer") {
		return "IE";
	} else if (window.navigator.appVersion.match(/Safari/) ) {
		return "Safari";
	} else if (window.navigator.appName == "Netscape") {
		return "Gecko";
	};
	return "";
};

/**
 * @return browser OS ( macosx, linux, windows )
 */
Utilites.getBrowserOS = function ()
{
	if ( window.navigator.userAgent.match(/Macintosh/i) ) {
		return 'macosx';
	} else if ( window.navigator.userAgent.match(/Linux/i) ) {
		return 'linux';
	} else if ( window.navigator.userAgent.match(/win/i) ) {
		return 'windows';
	};
};



/**
* function convert decimal number to hex string
*/
Utilites.toHexString = function(dec) 
{
	var hex_arr = new Array('0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f');
	var hex = '';
	do {
		hex = hex_arr[dec % 16] + hex;
		dec = parseInt(dec/16);
	} while (dec > 15);
	return hex_arr[dec] + hex;
};



/**
* this function called when compiler find error in the "try {} catch {}" operator
*/
Utilites.dumpError = function(error) 
{
	var msg = "";
	var prop;
	var maxlength = 0;
	for (prop in error) {
		try {
			msg += prop + ":" + error[prop] + "\n";
			if (prop.length + error[prop].length > maxlength ) { maxlength = prop.length + error[prop].length; };
		} catch(e) { return; };
	};
	alert(msg);
};

/**
* get child elements from DOM node by CSS class name
* @param node - base node
* @className - CSS class name string
* @return array with all founded elements
*/
Utilites.getElementsByClassName = function (node, className) {
	var children = node.getElementsByTagName('*');
	var elements = new Array();
	for (var i=0; i<children.length; i++) {
		var child = children[i];
		var classNames = child.className.split(' ');
		for (var j = 0; j < classNames.length; j++) {
			if (classNames[j] == className) {
				elements.push(child);
				break;
			}
		}
	}
	return elements;
}

/**
* get child element from DOM node by id
* @param node - base node
* @className - node id
* @return first search node
*/
Utilites.getChildElementById = function (node, id) {
	if ( typeof(node) == "string" ) {
		node = $(node);
	};
	var children = node.getElementsByTagName('*');
//	var elements = new Array();
	for (var i=0; i<children.length; i++) {
		var child = children[i];
		if ( child.id == id ) {
			return child;
//			elements.push(child);
		};
	}
//	return elements;
}

/**
* get child elements from DOM node by id
* @param node - base node
* @className - CSS class name string
* @return array with all founded elements
*/
Utilites.getChildElementsById = function (node, id) {
	if ( typeof(node) == "string" ) {
		node = $(node);
	};
	var children = node.getElementsByTagName('*');
	var elements = new Array();
	for (var i=0; i<children.length; i++) {
		var child = children[i];
		if ( child.id == id ) {
//			return child;
			elements.push(child);
		};
	}
	return elements;
}



/**
* get child elements from DOM node by tag  name
* @param node - base node
* @className - tag name
* @return array with all founded elements
*/
Utilites.getChildElementByName = function (node, searchNodeName) {
	var children = node.getElementsByTagName('*');
	for (var i=0; i<children.length; i++) {
		var child = children[i];
		if ( child.nodeName.toLowerCase() == searchNodeName.toLowerCase() ) {
			return child;
		};
	}
}



Utilites.getNodeID = function(parent, id) {
	var ln = parent.childNodes.length;
	for (var z=0; z<ln; z++) {
		if (parent.childNodes[z].id == id) return parent.childNodes[z];
	}
	return null;
}


Utilites.getChildrenByTagName = function(node, tagName) {
	var ln = node.childNodes.length;
	var arr = [];	
	for (var z=0; z<ln; z++) {
		if (node.childNodes[z].nodeName==tagName) arr.push(node.childNodes[z]);
	}
	return arr;
}

Utilites.getParentByTagName = function(node, tagName) 
{
        var parNode = node;
	while (parNode) {
		if ( parNode && parNode.nodeName && parNode.nodeName.toLowerCase() == tagName ) {
			return parNode;
		}
		parNode = parNode.parentNode;
	};
	return null;
};
Utilites.getParentByClassName = function(node, className) 
{
        var parNode = node;
	while (parNode) {
		if ( parNode && parNode.className == className ) {
			return parNode;
		}
		parNode = parNode.parentNode;
	};
	return null;
};

Utilites.getNodeLength = function (node) {
	var length = 0;
	var tmpNode = node.parentNode;
	while ( tmpNode ) {
		length++;
		tmpNode = tmpNode.parentNode;
	};
	return length;
};



Utilites.getColonTag = function(node, tag, name) {
	return (Browser.isIE) ? node.getElementsByTagName(tag+":"+name)[0] : node.getElementsByTagName(name)[0];
}

Utilites.htmlEncode = function(text) {
	return text.replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
	//"
}

Utilites.isEmailValid = function (e) {
	var ok = "1234567890qwertyuiop[]asdfghjklzxcvbnm.+@-_QWERTYUIOPASDFGHJKLZXCVBNM";
	for(var i=0; i<e.length; i++){
		if (ok.indexOf(e.charAt(i))<0) {
			return false;
		}
	}
	if (document.images) {
		var re = /(@.*@)|(\.\.)|(^\.)|(^@)|(@$)|(\.$)|(@\.)/;
		var re_two = /^.+\@(\[?)[a-zA-Z0-9\-\.]+\.([a-zA-Z]{2,8}|[0-9]{1,3})(\]?)$/;
		if (!e.match(re) && e.match(re_two)) {
			return -1;
		}
	}
}


Utilites.cleanWhitespace = function(node){
	var notspace = /\S/;
	for (var x=0; x<node.childNodes.length; x++) {
		var child = node.childNodes[x];
		//if it is a whitespace text node
		if ((child.nodeType == 3) && (!notspace.test(child.nodeValue)))	{
			node.removeChild(node.childNodes[x]);
			//node.childNodes[x].removeNode();
			x--;
		}
		//elements can have text child nodes of their own
		if(child.nodeType == 1) {
			Utils.cleanWhitespace(child);
		}
	}
}

Utilites.gEl = function (id) {
	return $(id);
};
Utilites.gElByTag = function (tag) {
	return document.getElementsByTagName ? document.getElementsByTagName(tag) : new Array();
};

/**
* escape URL string
*/
Utilites.escape = function ( str ) {
	return window.encodeURIComponent ? encodeURIComponent(str) : escape(str);
};

/**
* unescape URL
*/
Utilites.unescape = function ( str ) {
	return window.decodeURIComponent ? decodeURIComponent(str) : unescape(str);
};




/*
function xmlToString(thexml){
	if(thexml.xml){
		// MSIE
		xmlString = thexml.xml;
	}else{
		// Gecko
		xmlString = (new XMLSerializer).serializeToString(thexml);
	}
	return xmlString;
}
*/


/**
* remove all <script></script> sections from content
* @param str - input string
* @return content without <script></script>
*/
Utilites.getContentWithoudDesktopScripts = function (str)
{
	var pos_b  = -1;
	var pos_e  = -9;
	var result = "";
	var new_str = "";
	
	// simple parser which replace some tags and attributes
	while ( 1 ) {
		
		pos_b = str.toLowerCase().indexOf('<script>',pos_e);

		if ( pos_b != -1 ) {
			new_str += str.substring(pos_e + 9,pos_b);

			if ( pos_e != 0 ) pos_e++;
			pos_e = str.toLowerCase().indexOf('</script>',pos_b);
			
			if ( pos_e != -1 ) {
				//execute script
				//eval(str.substring(pos_b + 8,pos_e));
			} else {
				break;
			};
		} else {
			//if ( pos_e != 0 ) pos_e++;
			new_str += str.substring(pos_e + 9, str.length);
			break;
		};
	};

//	alert(str);
//	str = str.replace(/<desktopscript>.*<\/desktopscript>/gsi,"");
//	alert(new_str);
	return new_str;
}



/**
* Search and execute javascript in the content
* @param str - input string
*/
Utilites.SearchAndExecuteJavaScripts = function(str, start_tag, end_tag) {
	var pos_b  = 0;
	var pos_e  = 0;
	var result = "";
	
	if ( !start_tag ) { start_tag = "<script>"; end_tag = "</script>"; };

	// simple parser which replace some tags and attributes
	while ( 1 ) {
		
		pos_b = str.indexOf( start_tag, pos_e);

		if ( pos_b != -1 ) {
			
			if ( pos_e != 0 ) pos_e++;
			pos_e = str.indexOf(end_tag, pos_b);
			
			if ( pos_e != -1 ) {
				//execute script
				//eval(str.substring(pos_b + start_tag.length, pos_e));
				//var scriptEl = document.createElement("script");
				//scriptEl.innerHTML = str.substring(pos_b + start_tag.length, pos_e) ;
				//scriptEl.text = str.substring(pos_b + start_tag.length, pos_e) ;
				//scriptEl.innerHTML = "function asd (a) {alert(a)}; asd('aaaa');";

				//document.getElementsByTagName("head")[0].appendChild(scriptEl);
				var script_cont=str.substring(pos_b + start_tag.length, pos_e);//place(/\\\\/g,'\\\\\\\\').replace(/"/g,'\\\\"');

				if(window.execScript) {
					//window.execScript(script_cont);
					var scriptEl = document.createElement("script");
					scriptEl.text = script_cont;
					document.getElementsByTagName("head")[0].appendChild(scriptEl);
				} else {
					setTimeout(script_cont,0);
				};
			} else {
				break;
			};
		} else {
			if ( pos_e != 0 ) pos_e++;
			break;
		};
	};

	//css

	pos_b  = 0;
	pos_e  = 0;
	result = "";

	start_tag = "<style>"; 
	end_tag = "</style>";

	// simple parser which replace some tags and attributes
	while ( 1 ) {
		pos_b = str.indexOf( start_tag, pos_e);

		if ( pos_b != -1 ) {
			
			if ( pos_e != 0 ) pos_e++;
			pos_e = str.indexOf(end_tag, pos_b);
			
			if ( pos_e != -1 ) {
				var styleEl = document.createElement("style");
				styleEl.setAttribute("type", "text/css");
				styleEl.setAttribute("media", "all");
				document.getElementsByTagName("head")[0].appendChild(styleEl);

				var cssText  = str.substring(pos_b + start_tag.length, pos_e);
				var cssrules = cssText.split("}");
				/*
				//r newcss = document.styleSheets[0];
				if(styleEl.rules) { //IE
					for(i=cssrules.length-2;i>=0;i--) {
						var newrule = cssrules[i].split("{");
						styleEl.addRule(newrule[0],newrule[1])
					}
				} else if(styleEl.cssRules) { //Firefox etc
					for(i=cssrules.length-1;i>=0;i--) {
						if (!/\s$/.test(cssrules[i])) styleEl.insertRule(cssrules[i]+"}",0);
					};
				} else {
					var cssTextEl = document.createTextNode(cssText);
					alert(cssText);
					styleEl.appendChild(cssText);
					alert("problems with css");
				};
				*/
				if( styleEl.styleSheet ) {
					styleEl.styleSheet.cssText=cssText;
				} else {
					styleEl.appendChild(document.createTextNode(cssText));
				}

				/*
				var cssText = str.substring(pos_b + start_tag.length, pos_e) ;
				if(styleEl.styleSheet){// IE
					styleEl.styleSheet.cssText = cssText;
				} else {
					alert(1);
					var cssTextEl = document.createTextNode(cssText);
					styleEl.appendChild(cssText);
				}
				*/
			} else {
				break;
			};
		} else {
			if ( pos_e != 0 ) pos_e++;
			break;
		};
	};

};

/**
* preload all images from content
* @param content - input html content
*/
Utilites.preloadImagesFromContent = function (content) 
{
	//alert(content);
	var a = content.match(/<img.*src=['|"]?([^'"\s>]+)['|"]?>/gi);
	//"'
	for (var i = 0; a && i < a.length; i++) {
		var tmp = a[i];
		if (tmp) {
			if ( tmp.match(/<img.*src=['|"]?([^'"\s>]+)['|"]?>/gi) ) {
			//"'
				var img_src = RegExp.$1;
				if ( img_src.match(/tab_/i) ) {
					alert(img_src);
				};
				if ( !DesktopImages[img_src] ) {
					DesktopImages[img_src]     = new Image();
					DesktopImages[img_src].src = img_src;
				}
			};
		}
	}
};

Utilites.preloadImagesFromArray = function ( images_arr ) 
{
	for (var i = 0; i < images_arr.length; i++) {
		img_src = images_arr[i];
		if ( !DesktopImages[img_src] ) {
			DesktopImages[img_src]     = new Image();
			DesktopImages[img_src].src = img_src;
		};
	};
};


/**
* Hide all frames, select, applets, etc, bottom of parameter element
* @param el - DOM element
*/
Utilites.hideWindowedDocumentElements = function(el) {
	if ( !Common.BrowserInfo.IsIE ) {
		return;
	}
	function getVisib(obj){
		var value = obj.style.visibility;
		if (!value) {
			if (document.defaultView && typeof (document.defaultView.getComputedStyle) == "function") { // Gecko, W3C
				value = document.defaultView.getComputedStyle(obj, "").getPropertyValue("visibility");
			} else if (obj.currentStyle) { // IE
				value = obj.currentStyle.visibility;
			} else
				value = '';
		}
		return value;
	};

	var tags = new Array("applet", "iframe", "select");

	var p = Utilites.getAbsolutePos(el);
	var EX1 = p.x;
	var EX2 = el.offsetWidth + EX1;
	var EY1 = p.y;
	var EY2 = el.offsetHeight + EY1;
		
		for (var k = tags.length; k > 0; ) {
			var ar = document.getElementsByTagName(tags[--k]);
			var cc = null;

			for (var i = ar.length; i > 0;) {
				cc = ar[--i];

				p = Utilites.getAbsolutePos(cc);
				var CX1 = p.x;
				var CX2 = cc.offsetWidth + CX1;
				var CY1 = p.y;
				var CY2 = cc.offsetHeight + CY1;

				if (self.hidden || (CX1 > EX2) || (CX2 < EX1) || (CY1 > EY2) || (CY2 < EY1)) {
					if (!cc.__msh_save_visibility) {
						cc.__msh_save_visibility = getVisib(cc);
					}
					cc.style.visibility = cc.__msh_save_visibility;
				} else {
					if (!cc.__msh_save_visibility) {
						cc.__msh_save_visibility = getVisib(cc);
					}
					
					var flag = false;
					var tmp_ar = el.getElementsByTagName(tags[k]);
					for ( var tmp_i=0; tmp_i<tmp_ar.length; tmp_i++ ) {
						if ( cc == tmp_ar[i] ) {
							flag = true;
							break;
						};
					};
					if ( !flag ) {
						cc.style.visibility = "hidden";
					} else {
						cc.style.visibility = "visible";
					}
				}
			}
		}
};

//based on the webcalendar
Utilites.getAbsolutePos = function(el) {
	var SL = 0, ST = 0;
	var is_div = /^div$/i.test(el.tagName);
	if (is_div && el.scrollLeft)
		SL = el.scrollLeft;
	if (is_div && el.scrollTop)
		ST = el.scrollTop;
	var r = { x: el.offsetLeft - SL, y: el.offsetTop - ST };
	if (el.offsetParent) {
		var tmp = this.getAbsolutePos(el.offsetParent);
		r.x += tmp.x;
		r.y += tmp.y;
	}
	return r;
};






/**
* Execute javascript command when condition is true. Max time for wait condition - 30 sec.\n
* Example:
* \code
* Utilites.executeAfterConditionTrue ("$('waited_element')","alert(1)");
* \endcode
* @param condition - condition js string
* @param script    - script which should be executed when condition is true
*/
Utilites.executeAfterConditionTrue = function( condition, script, interval, execution_num )
{
	if ( !execution_num) { execution_num = 0; };
	if ( !interval ) { interval = 3000; };
	
	if ( execution_num > 35 ) {
		return;
	};

	var condition_true = false;
	try {
		var loaded = eval ( condition );
		if ( loaded ) {
			condition_true = true;
		} else {
			setTimeout('Utilites.executeAfterConditionTrue("'+condition+'", "' +script +'", '+interval+', '+(execution_num+1)+')', interval);
		}
	} catch (e) {
		if ( execution_num > 30 ) {
			Debug.addMsg("Unable to execute script.\r\n"+script+"\r\nReason:\r\n"+e);
			return;
		};
		setTimeout('Utilites.executeAfterConditionTrue("'+condition+'", "' +script +'", '+interval+', '+(execution_num+1)+')', interval);
	};
	if ( condition_true ) {
		eval ( script );
		Debug.addMsg("Following script has been executed.\r\n"+script);
	};

}


Utilites.insertAlphabet = function(obj_id, process_function, curr_item, weblet_id)
{
	var lang_arr = new Array();
	var lang_arr_addit = new Array();
	
	lang_arr = new Array("All|All", "a|A","b|B","c|C","d|D","e|E","f|F","g|G","h|H","i|I","j|J","k|K","l|L","m|M","n|N","o|O","p|P","r|R","s|S","t|T","u|U","v|V","w|W","x|X","y|Y","z|Z");

	switch(LANGUAGE)
	{
		case 'ru':
			lang_arr[0] = "All|Все";
			lang_arr_addit = new Array("BR","а|А","б|Б","в|В","г|Г","д|Д","е|Е","ж|Ж","з|З","и|И","й|Й","к|К","л|Л","м|М","н|Н","о|О","п|П","р|Р","с|С","т|Т","у|У","ф|Ф","х|Х","ц|Ц","ч|Ч","щ|Щ","ш|Ш","у|У","ы|Ы","э|Э","ю|Ю","я|Я");
			break;
		case 'fr':
			lang_arr[0] = "All|All";
			lang_arr_addit = new Array("â|Â","ê|Ê","î|Î","ô|Ô","à|À","é|É","è|È","ç|Ç","û|û","ë|ë","ï|ï","ö|ö","ü|ü");
			break;
	};
	if ( lang_arr_addit.length > 0 ) {
		for ( var i=0; i<lang_arr_addit.length; i++ ) {
			lang_arr.push(lang_arr_addit[i]);
		};
	};

	var content = "";
	for ( var i=0; i < lang_arr.length; i++ ) {
		if ( lang_arr[i] == "BR" ) { content += "<br>"; continue; };

		if ( (lang_arr[i] == curr_item) || (lang_arr[i].substr(0,3) == 'All' && curr_item == "")  ) {
			content += "<a href='javascript:void(0);' id='current' onClick=\""+process_function+"('"+lang_arr[i]+"','"+weblet_id+"')\">" + lang_arr[i].split(/\|/)[1] + "</a> ";
		} else {
			content += "<a href='javascript:void(0);' onClick=\""+process_function+"('"+lang_arr[i]+"','"+weblet_id+"')\">" + lang_arr[i].split(/\|/)[1] + "</a> ";
		}
	};
	$(obj_id).innerHTML = content;
};




function urlDecode(str){
    str=str.replace(new RegExp('\\+','g'),' ');
    return unescape(str);
}

function urlEncode(str){
    str=escape(str);
    str=str.replace(new RegExp('\\+','g'),'%2B');
    return str.replace(new RegExp('%20','g'),'+');
}


Utilites.convertTextToFilename = function ( txt )
{
	var specSymbols = new Array();
	var resultString = '';
	specSymbols = {
	  //russian characters
	  1072 : "a",
	  1073 : "b",
	  1074 : "v",
	  1075 : "g",
	  1076 : "d",
	  1077 : "e",
	  1105 : "ye", //¸
	  1078 : "zh", //æ
	  1079 : "z",
	  1080 : "i",
	  1081 : "y",
	  1082 : "k",
	  1083 : "l",
	  1084 : "m",
	  1085 : "n",
	  1086 : "o",
	  1087 : "p",
	  1088 : "r",
	  1089 : "s",
	  1090 : "t",
	  1091 : "u",
	  1092 : "f",
	  1093 : "h",
	  1094 : "tc",
	  1095 : "ch",
	  1096 : "sh",
	  1097 : "shya",
	  1098 : "", //ú
	  1099 : "i", //û
	  1100 : "",  //ü
	  1101 : "e",
	  1102 : "yu",
	  1103 : "ya",

	  32 : "-"

	  
	}

	//stripping all last spaces
	//txt = trim(txt);

	//convert characters
	for (i = 0; i < txt.length; i++){
		charCode = txt.toLowerCase().charCodeAt(i);
	  
	  if ( 
	    charCode >= 48 &&  // 1-9
	    charCode <= 57 ||  // |

	    charCode >= 97 &&  // a-z
	    charCode <= 122 || // |

	    charCode == 45 ||  // -
	    charCode == 95 ||  // _
	    charCode == 46     // .
	   )
	     currentChar = String.fromCharCode(charCode);

	   else if ( charCode >= 192 && charCode <= 198 || charCode >= 224 && charCode <= 230 )
	     currentChar = "a";
	   else if ( charCode == 199 || charCode == 231 )
	     currentChar = "c";
	   else if ( charCode >= 200 && charCode <= 203 || charCode >= 232 && charCode <= 235 )
	     currentChar = "e";
	   else if ( charCode >= 204 && charCode <= 207 || charCode >= 236 && charCode <= 236 )
	     currentChar = "i";
	   else if ( charCode >= 210 && charCode <= 214 || charCode >= 242 && charCode <= 246 )
	     currentChar = "o";
	   else if ( charCode >= 217 && charCode <= 220 || charCode >= 250 && charCode <= 252 )
	     currentChar = "u";

	   else {
	     currentChar = specSymbols[charCode];
	   }

	   if ( typeof currentChar == "undefined" )
	     currentChar = '';

	   resultString += currentChar;

	}

	/*
	if (obj.id == out.id) {
	  //var caret = getCaretPos(obj);
	  var caret = getCaretPosition(obj);
	  caret = caret + resultString.length - oldSize;
	  out.value = resultString;
	  setSelectionRange(obj, caret, caret);
	}
	else
	  out.value = resultString;
	*/
	return resultString;
};


Utilites.getUniqueValue_lastNum = 0;
Utilites.getUniqueValue = function ()
{
	return "discomp_uniquie_id_"+ (Utilites.getUniqueValue_lastNum++);
};

/** @} */ // end of js_utilites group
