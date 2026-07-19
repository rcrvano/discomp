    function initStore(node_stats)
    {
    	var fields_array = new Array('interval');

	for ( var i=1; i<=node_stats.cpu.core.length; i++ ) {
		fields_array[fields_array.length] = "core"+i+"_user";
		fields_array[fields_array.length] = "core"+i+"_sys";
	};

	monitoringStore = Ext4.create('store.json', { fields: fields_array });
	initDefaultStore(node_stats);
    };


    var store_initialized = false;
    var graph_initialized = false;
    function processCurrentNodeStats(node_stats)
    {
    	    if ( store_initialized == false ) {
	    	initStore(node_stats);
		store_initialized = true;
	    };



            monitoringStore.data.removeAt(0);
            monitoringStore.data.each(function(item, key) 
            {
                item.data.interval = key;
            });
            
	    var lastData = monitoringStore.last().data;

            var data = {};
	    data["interval"] = lastData.interval + 1;
	    for ( var i=1; i<=node_stats.cpu.core.length; i++ ) {
		data["core"+i+"_user"] = parseInt(node_stats.cpu.core[i-1]["@attributes"].user_p)
		data["core"+i+"_sys"] = parseInt(node_stats.cpu.core[i-1]["@attributes"].sys_p)
            };
	    
	    //alert(data.core1_used);
            monitoringStore.loadData([data], true);

            if ( !isGraphInitialized() ) {
                initGraphs(node_stats);
            };
    };



var graph_initialized = false;
function isGraphInitialized()
{
	return graph_initialized;
};

function initGraphs (node_stats)
{
 for ( var i=1; i<=node_stats.cpu.core.length; i++ ) {
 var cpu_core = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Core '+i,
        renderTo: 'graph-area',
        layout: 'fit',
        items: {
            xtype: 'chart',
            animate: false,
            store: monitoringStore,
            insetPadding: 10,
            legend: {
                position: 'bottom',
		padding: 1,
		margin:1,
		font: {
                    family: 'Tahoma',
                    size: 8
                }
            },

            axes: [{
                type: 'Numeric',
                minimum: 0,
                maximum: 100,
                position: 'left',
                fields: ['core'+i+'_user', 'core'+i+'_sys',],
                title: false,
		grid: true,
                label: {
                    renderer: Ext.util.Format.numberRenderer('0,0'),
                    font: '10px Arial'
                }
            }, {
                type: 'Numeric',
                position: 'bottom',
                fields: ['interval'],
                title: false,
                label: {
                    font: '11px Arial',
                    renderer: Ext.util.Format.numberRenderer('0,0'),                    
                }
            }],
            series: [{
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'core'+i+'_user',
		title: 'User',
		smooth: true,
		fill: true,
		highlight: {
                    size: 1,
                    radius: 1
                },
                style: {
                    fill: '#38B8BF',
                    stroke: '#38B8BF',
                    'stroke-width': 1
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: '#38B8BF',
                    stroke: '#38B8BF'
                }
            },
	    {
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'core'+i+'_sys',
		smooth: true,
		title  : 'Sys',
		fill: true,
		highlight: {
                    size: 1,
                    radius: 1
                },
                style: {
                    fill: 'red',
                    stroke: 'red',
                    'stroke-width': 1
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'red',
                    stroke: 'red'
                }
            }]
        }
    });
    
  };
   


    graph_initialized = true;
};

