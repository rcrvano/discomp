DiscompNodes = new Object();

DiscompNodes.showNodeInfo = function (nodeRow)
{

	var node_id = nodeRow.id;
	if ( nodeRow.nextSibling.className == "node_info" ) {
		nodeRow.parentNode.removeChild(nodeRow.nextSibling);
	} else if ( nodeRow.className == "node_info" ) {
		nodeRow.parentNode.removeChild(nodeRow);
	} else {
		var newTrRow = document.createElement ("tr");
		newTrRow.id = "node_info_data_" + node_id;
		newTrRow.className = 'node_info';
		nodeRow.parentNode.insertBefore(newTrRow, nodeRow.nextSibling);

		AjaxCommunications.Request ( "index.php?component=nodes&action=get_node_info&node_id=" + node_id, DiscompNodes.showNodeInfo_callback, newTrRow.id )
		DiscompNodes.updateNodeSysInfo(node_id, true);
	};
}
DiscompNodes.showNodeInfo_callback = function (received_data, tr_id)
{
	var trEl = Utilites.gEl(tr_id);
	var tdEl;
	if ( trEl.firstChild ) {
		tdEl = trEl.firstChild;
	} else {
		tdEl = document.createElement("td");
		tdEl.colSpan = 40;
		//tdEl.style.display='none';
		trEl.appendChild(tdEl);
	};
	tdEl.innerHTML = received_data;
	//Effect.BlindDown(tdEl);
};

DiscompNodes.updateNodeSysInfo = function (node_id, no_refresh)
{
	if ( no_refresh == undefined ) { no_refresh = false; };
	var trEl = Utilites.gEl("node_info_data_" + node_id);
	if ( trEl.firstChild && !no_refresh ) {
		trEl.firstChild.innerHTML = "Updating...";
	};
	AjaxCommunications.Request ( "index.php?component=nodes&action=update_node_sysinfo&node_id=" + node_id, DiscompNodes.showNodeInfo_callback, trEl.id )
}

DiscompNodes.showModule = function ( module )
{

}


DiscompNodes.gotoPackage = function ( package )
{

}

DiscompNodes.gotoProcess = function ( process )
{
	document.location = "index.php?comp=processes&process="+process;
}

DiscompNodes.deleteFailedModule = function (node_id, module)
{
	AjaxCommunications.Request ( "index.php?component=nodes&action=del_failed_module&node_id=" + node_id+"&module="+module, DiscompNodes.showNodeInfo_callback, "node_info_data_" + node_id );
}

DiscompNodes.showMonitor = function (node_id)
{
	document.location = "?component=monitor&action=node_monitor&node_id=" + node_id;
}

