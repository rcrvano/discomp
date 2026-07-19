function initStore(node_stats)
    {
    	var fields_array = new Array('interval');

	for ( var i=1; i<=node_stats.cputemp.core.length; i++ ) {
		fields_array[fields_array.length] = "coretemp"+i+"_used";
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
	    for ( var i=1; i<=node_stats.cputemp.core.length; i++ ) {
		data["cputemp"+i+"_used"] = parseInt(node_stats.cputemp.core[i-1]);
		//data["cputemp"+i+"_sys"] = parseInt(node_stats.cputemp[i-1]["@attributes"].sys_p)
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
 for ( var i=1; i<=node_stats.cputemp.core.length; i++ ) {
 var cputemp = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Core '+i+' temperature',
        renderTo: 'graph-area',
        layout: 'fit',
        items: {
            xtype: 'chart',
            animate: false,
            store: monitoringStore,
            insetPadding: 10,
            axes: [{
                type: 'Numeric',
                minimum: 0,
                maximum: 100,
                position: 'left',
                fields: ['cputemp'+i+'_used'],
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
                yField: 'cputemp'+i+'_used',
		smooth: true,
		fill: true,
		highlight: {
                    size: 7,
                    radius: 7
                },
                style: {
                    fill: '#38B8BF',
                    stroke: '#38B8BF',
                    'stroke-width': 3
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: '#38B8BF',
                    stroke: '#38B8BF'
                }
            }]
	    }
	});
    };
   


    graph_initialized = true;
};

