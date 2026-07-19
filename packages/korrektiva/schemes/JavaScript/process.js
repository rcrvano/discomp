// For now we need to realize following functions:
// *) Init base process structures(modules, parameters) throught javascript calls
// *) DiscompModule, DiscompParameter
// *) Start and Stop Module.
//

//constructor
function Process ()
{
	//this.modules = ["decompose", "solver", "analyzer"];
	//here can be defined some local variables
	this.modules    = new Object();
	this.parameters = new Object();
}

Process.prototype.Init = function ()
{
	this.modules.decompose = new DiscompModule("decompose");

	this.modules.decompose.addEventListener ( "finished", 
		function () { 
			DiscompAPI.logMessage ( "Discomp process successfully finished" );
		} 
	);


	this.modules.analyzer  = new DiscompModule("analyzer");
	this.modules.solver    = new DiscompModule("solver");

	if ( !this.modules.solver ) {
		DiscompAPI.logMessage("Couldn't load solver module");
		return 0;
	};
solver.Start()
anay\

	block()

	this.parameters.d_num = new DiscompParameter("decompose_num");
	if ( this.parameters.d_num.getValue () == "" ) { //set default value
		this.parameters.d_num.setValue ( 50 ) ;
	};
	
	return 1;
};


Process.prototype.Start = function ()
{
	if ( this.modules.decompose.isReady () ) { //all input parameters defined
		this.modules.decompose.Start ()
	};

}

Process.prototype.checkQueue = function ()
{
	//check what process modules not started yet
	//
}

Process.prototype.myFunction = function  ()
{
	//smthing like this for start parallel list.
	for ( int i =0; i < 100; i++ ) {
		this.modules.solver.StartList(i);
	};
}



