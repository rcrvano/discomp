function test ( )
{
	DiscompAPI.logMessage ( "Hello world" );
	var a = DiscompAPI.getFileParameterValue("decomposition.count");
	DiscompAPI.logMessage(a);
};

function test2 ( ) 
{
	var list_value = DiscompAPI.getListParameterValue ( "konus.out", 2);
	DiscompAPI.logMessage ( "List value is:" + list_value );
}
