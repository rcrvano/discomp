// +--------------------------------------------------------------------+
// | TelinWeb Desktop                                                   |
// +--------------------------------------------------------------------+
// | Copyright (C) 2005-2008 TelinWeb Inc.                              |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author : Sidorov Ivan <ivan@sidorov.net> <yvan@artwhere.be>        |
// +--------------------------------------------------------------------+
// | File   : $RCSfile: event.js,v $					|
// | Version: $Revision: 1.18 $						|
// | Updated: $Date: 2008/01/04 11:36:17 $				|
// +--------------------------------------------------------------------+


var DesktopEvent = new Object();
DesktopEvent.mouseX=0;
DesktopEvent.mouseY=0;
DesktopEvent.shift =false;
DesktopEvent.ctrl  =false;

function initDocumentEventListeners() 
/*catch document event*/
{

	document.onmousedown	= catchEventOnDocumentMouseDown;
	document.onclick	= catchEventOnDocumentClick;
	/*for layer operations*/
	document.onkeypress 	= spyDocumentKeyPress;
	document.onmousemove	= catchEventOnMouseMove;
};


function catchEventOnMouseMove(evnt)
{
	if ( ! evnt ) { try { evnt = window.event;} catch(e) {}; };
	if ( ! evnt ) { return; }
	DesktopEvent.mouseX	= evnt.clientX;
	DesktopEvent.mouseY	= evnt.clientY;
}

function catchEventOnDocumentMouseDown(evnt) 
{
	if ( ! evnt ) { try { evnt = window.event;} catch(e) {}; };
	if ( ! evnt ) { return; }
	DesktopEvent.shift = evnt.shiftKey;
	DesktopEvent.ctrl  = evnt.ctrlKey;

	if ( evnt.metaKey && Common.BrowserOS.macosx ) {
		DesktopEvent.ctrl = true;
	};
};

function catchEventOnDocumentClick(evnt) 
{
	if ( ! evnt ) { try { evnt = window.event;} catch(e) {}; };
	if ( ! evnt ) { return; }
	if ( evnt.ctrlKey ) {
	}
	if ( evnt.shiftKey ) {
	}
};



function spyDocumentKeyPress(evnt) 
{
	if ( ! evnt ) { try { evnt = window.event;} catch(e) {}; };
	if ( ! evnt ) { return; };
	
	switch (evnt.keyCode) {
		case 27 : 
			//closeEditorDialog();
			break;
		default :
			break;
	};
	var code = Common.BrowserInfo.IsGecko ? evnt.charCode : evnt.keyCode;


	switch (code) {
		//change Tab by pressing Alt+key
		case 49 : //Alt + 1
		case 50 : //Alt + 2
		case 51 : //Alt + 3
		case 52 : //Alt + 4
		case 53 : //Alt + 5
		case 54 : //Alt + 6
		case 55 : //Alt + 7
		case 56 : //Alt + 8
		case 57 : //Alt + 9
			if ( !DESKTOP_DEBUG && Common.BrowserInfo.IsGecko && evnt.altKey) {
				var num = code-49;
				if ( num >= DesktopTabs.tabs.length ) { return; };
				//var tab_id = ( num==0 ? 0 : DesktopTabs.tabs[num] );
				DesktopTabs.changeTab(DesktopTabs.tabs[num]);
				DesktopWebletsMove.tabChanged(DesktopTabs.tabs[num]);
				evnt.stopPropagation();
			};
			break;

		//open debug console
		case 68 : // D
			//ctrl+shift+D Mozilla Windows | Shift+Ctrl+Alt+D - Linux
			if ( Common.BrowserInfo.IsGecko && evnt.ctrlKey && evnt.shiftKey ) {
				Debug.ConsoleShow();
				evnt.stopPropagation();
			};
			break;
		case 4: //ctrl+shift+D IE Windows
			if ( Common.BrowserInfo.IsIE && evnt.ctrlKey && evnt.shiftKey ) {
				Debug.ConsoleShow();
				return false;
			};
		
		default :
			break;
	};

};




/**
 * @defgroup js_event JS Common Events
 * @brief Some events which you can access evrytime

 * \code
 * DesktopEvent.mouseX - current mouse X position
 * DesktopEvent.mouseY - current mouse Y position
 * DesktopEvent.shift  - shift is pressed
 * DesktopEvent.ctrl   - ctrl is pressed

 * \endcode
 *  @{ @} */ // end of js_event group
