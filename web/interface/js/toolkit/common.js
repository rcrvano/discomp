// +--------------------------------------------------------------------+
// | TelinWeb Desktop                                                   |
// +--------------------------------------------------------------------+
// | Copyright (C) 2005-2008 TelinWeb Inc.                              |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author : Sidorov Ivan <ivan@sidorov.net> <yvan@artwhere.be>        |
// +--------------------------------------------------------------------+
// | File   : $RCSfile: common.js,v $					|
// | Version: $Revision: 1.14 $						|
// | Updated: $Date: 2008/01/04 11:36:17 $				|
// +--------------------------------------------------------------------+


var Common = new Object();


//check browser
Common.BrowserInfo = new Object() ;
Common.BrowserInfo.IsIE    = Common.BrowserInfo.isIE    = Common.BrowserInfo.IE    = ( Utilites.getBrowserName() == "IE" ) ? 1 : 0;
Common.BrowserInfo.IsGecko = Common.BrowserInfo.isGecko = Common.BrowserInfo.Gecko = ( Utilites.getBrowserName() == "Gecko" ) ? 1 : 0;
Common.BrowserInfo.IsOpera = Common.BrowserInfo.isOpera = Common.BrowserInfo.Opera = ( Utilites.getBrowserName() == "Opera" ) ? 1 : 0;
Common.BrowserInfo.IsSafari= Common.BrowserInfo.isSafari= Common.BrowserInfo.Safari= ( Utilites.getBrowserName() == "Safari" ) ? 1 : 0;
if (Common.BrowserInfo.Safari) {
	Common.BrowserInfo.IsGecko = true;
};

Common.BrowserOS = new Object ();
Common.BrowserOS.macosx = ( Utilites.getBrowserOS() == "macosx" ) ;


String.prototype.trim = function() {
	return this.replace(/^\s*|\s*$/g,"");
}

