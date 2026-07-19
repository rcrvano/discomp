DiscompLog = new Object();

DiscompLog.initArea = function (area_id)
{
	this.area_id = area_id;
	this.area    = Utilites.gEl(area_id);
	this.area.scrollTop = 10000000;
	setTimeout ( "DiscompLog.updateArea()", 3000 );
};

DiscompLog.updateArea = function ()
{
	this.area.scrollTop = 10000000;
	setTimeout ( "DiscompLog.updateArea()", 3000 );
};
