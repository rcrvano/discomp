    function initStore(node_stats)
    {
    	var fields_array = new Array('interval');

	for ( var i=1; i<=node_stats.network.interface.length; i++ ) {
		fields_array[fields_array.length] = "network"+i+"_rx_bytes_psec";
		fields_array[fields_array.length] = "network"+i+"_tx_bytes_psec";
	};

	monitoringStore = Ext4.create('store.json', { fields: fields_array });
	initDefaultStore(node_stats);
    };


    var store_initialized = false;
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
	    for ( var i=1; i<=node_stats.network.interface.length; i++ ) {
		//data["network"+i+"_tx_bytes_psec"] = parseInt(node_stats.network.interface[i-1]["@attributes"].tx_bytes_psec)
		//data["network"+i+"_rx_bytes_psec"] = parseInt(node_stats.network.interface[i-1]["@attributes"].rx_bytes_psec)

	    	if ( node_stats.network.interface[i-1]["@attributes"].rx_bytes_psec  === undefined ) {
			data["network"+i+"_rx_bytes_psec"] = 0;
		} else {
			data["network"+i+"_rx_bytes_psec"] = parseInt(node_stats.network.interface[i-1]["@attributes"].rx_bytes_psec);
		};
	    	if ( node_stats.network.interface[i-1]["@attributes"].tx_bytes_psec === undefined ) {
			data["network"+i+"_tx_bytes_psec"] = 0;
		} else {
			data["network"+i+"_tx_bytes_psec"] = parseInt(node_stats.network.interface[i-1]["@attributes"].tx_bytes_psec);
		};


		if ( data["network"+i+"_tx_bytes_psec"] == 0 ) {
			data["network"+i+"_tx_bytes_psec"] = 0.001;
		};
		if ( data["network"+i+"_rx_bytes_psec"] == 0 ) {
			data["network"+i+"_rx_bytes_psec"] = 0.001;
		};

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
 for ( var i=1; i<=node_stats.network.interface.length; i++ ) {
// if ( node_stats.network.interface[i-1]["@attributes"].name  != "eth0" ) {
// 	continue;
// };
 var network_bytes_psec = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Network interface '+node_stats.network.interface[i-1]["@attributes"].name + " ("+node_stats.network.interface[i-1]["@attributes"].type+")",
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
		adjustMaximumByMajorUnit :1,
                position: 'left',
                fields: ['network'+i+'_tx_bytes_psec', 'network'+i+'_rx_bytes_psec'],
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
		title: 'Sent bytes',
                xField: 'interval',
                yField: 'network'+i+'_tx_bytes_psec',
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
		title: 'Received bytes',
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'network'+i+'_rx_bytes_psec',
		smooth: true,
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

