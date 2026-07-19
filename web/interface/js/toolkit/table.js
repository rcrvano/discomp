// +--------------------------------------------------------------------+
// | TelinWeb Desktop                                                   |
// +--------------------------------------------------------------------+
// | Copyright (C) 2005-2008 TelinWeb Inc.                              |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author : Sidorov Ivan <ivan@sidorov.net> <yvan@artwhere.be>        |
// +--------------------------------------------------------------------+
// | File   : $RCSfile: table.js,v $				|
// | Version: $Revision: 1.8 $						|
// | Updated: $Date: 2008/05/14 09:50:33 $				|
// +--------------------------------------------------------------------+

/**
 * @defgroup js_widgettable JS Desktop Content Table
 * @brief Widget Table allow you select rows and get selected row id.
 * 
 * Also it allow select multiple rows with shift or ctrl pressed.\n
 * See webmail weblet for example
 *
 *  @{
 */


var WidgetTable = new Object();

WidgetTable.rows = new Array();
WidgetTable.first_selected_row_id = new Array();
WidgetTable.last_row_element = new Array();
/**
 * Select table row.\n
 * Example:
 * \code <tr id='row_uid' onClick="WidgetTable.selectRow('{$WEBLET_ID}', this)" \endcode
 * @param unique_id - unique identificator
 * @param row_element - DOM TR element.
 * @param no_multiple - if true - disable multiple selection with shift or ctrl pressed
 */
WidgetTable.selectRow = function (unique_id, row_element, no_multiple) 
{
	var row_id = row_element.id;

	if ( !WidgetTable.rows[unique_id] ) {
		WidgetTable.rows[unique_id] = new Array();
	};

	//search if this row id already exists
	var exists     = false;
	var exists_pos = -1;
	for ( var i=0; i < WidgetTable.rows[unique_id].length; i++ ) {
		if ( WidgetTable.rows[unique_id][i] == row_id ) {
			exists     = true;
			exists_pos = i;
			break;
		};
	};

	if ( no_multiple ) {
		DesktopEvent.shift = DesktopEvent.ctrl = 0;
	};

	//only one this row should be selected
	if ( !DesktopEvent.shift && !DesktopEvent.ctrl ) {
		if ( WidgetTable.last_row_element[unique_id] ) {
			WidgetTable.rows[unique_id] = new Array();
			try {
				WidgetTable.last_row_element[unique_id].className = WidgetTable.last_row_element[unique_id].className.replace(/selected/g,'');
			} catch (e) {};
		} else {
			WidgetTable.clearAllSelection(unique_id, row_element);
		};

		WidgetTable.rows[unique_id].push(row_id);
		row_element.className = 'selected ' + row_element.className;
		WidgetTable.last_row_element[unique_id] = row_element;
	
		WidgetTable.first_selected_row_id[unique_id] = row_element.id;
		return;
	} else {
		WidgetTable.last_row_element[unique_id] = "";
	};

	//multiple rows
	if ( DesktopEvent.ctrl ) {
		if ( exists ) {
			WidgetTable.rows[unique_id][exists_pos] = "";
			row_element.className = row_element.className.replace(/selected /g,'');
		} else {
			WidgetTable.rows[unique_id].push(row_id);
			row_element.className = 'selected ' + row_element.className;
		};
	}

	//list rows from first selected to current
	if ( DesktopEvent.shift ) {
		WidgetTable.clearAllSelection(unique_id,row_element);

		var tableElement = row_element.parentNode;
		var trElement    = tableElement.firstChild;

		//here we search first selected row and current row
		var first_found = false;
		var curr_found = false;
		var this_row_should_be_selected = false;
		while ( trElement ) {
			this_row_should_be_selected = false;
			if ( trElement.id == WidgetTable.first_selected_row_id[unique_id] ) {
				first_found                 = true;
				this_row_should_be_selected = true;
			};
			if ( trElement.id == row_id ) {
				curr_found                  = true;
				this_row_should_be_selected = true;
			};
		
			if ( (curr_found && !first_found) || (!curr_found && first_found ) || this_row_should_be_selected ) {
				WidgetTable.rows[unique_id].push(trElement.id);
				trElement.className = 'selected ' + trElement.className;
			};
			
			trElement    = trElement.nextSibling;
		};
	
	};

};

/**
 * Clear selection
 */
WidgetTable.clearAllSelection = function (unique_id, row_element)
{
	WidgetTable.rows[unique_id] = new Array();
	
	var tableElement = row_element.parentNode;
	var trElement    = tableElement.firstChild;
	
	while ( trElement ) {
		if ( trElement.className && trElement.className.match(/selected/i) ) {
			trElement.className = trElement.className.replace(/selected/g,'');
		};
		trElement    = trElement.nextSibling;
	};
};

/**
 * Get selected rows
 * @param unique_id - unique identificator
 * @return - array with id's of table row (tr).
 * @sa WidgetTable.selectRow
 */
WidgetTable.getSelectedRows = function (unique_id)
{
	if ( !WidgetTable.rows[unique_id] ) {
		return;
	};

	var arr_for_return = new Array();
	for ( var i=0; i < WidgetTable.rows[unique_id].length; i++ ) {
		if ( WidgetTable.rows[unique_id][i] != "" ) {
			arr_for_return.push(WidgetTable.rows[unique_id][i]);
		};
	};

	return arr_for_return;
}

WidgetTable.clearSelectionValues = function (unique_id)
{
	WidgetTable.rows[unique_id] = new Array();
};

/** @} */ // end of js_widgettable group
