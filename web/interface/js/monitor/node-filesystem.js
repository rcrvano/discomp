    function initStore(node_stats)
    {
    	var fields_array = new Array('interval');

	for ( var i=1; i<=node_stats.filesystems.device.length; i++ ) {
		fields_array[fields_array.length] = "fs"+i+"_read_bytes_psec";
		fields_array[fields_array.length] = "fs"+i+"_write_bytes_psec";
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
	    for ( var i=1; i<=node_stats.filesystems.device.length; i++ ) {
	    	if ( node_stats.filesystems.device[i-1]["@attributes"].read_bytes_psec  === undefined ) {
			data["fs"+i+"_read_bytes_psec"] = 0;
		} else {
			data["fs"+i+"_read_bytes_psec"] = parseInt(node_stats.filesystems.device[i-1]["@attributes"].read_bytes_psec);
		};
	    	if ( node_stats.filesystems.device[i-1]["@attributes"].write_bytes_psec === undefined ) {
			data["fs"+i+"_write_bytes_psec"] = 0;
		} else {
			data["fs"+i+"_write_bytes_psec"] = parseInt(node_stats.filesystems.device[i-1]["@attributes"].write_bytes_psec);
		};


		if ( data["fs"+i+"_write_bytes_psec"] == 0 ) {
			data["fs"+i+"_write_bytes_psec"] = 0.001;
		};
		if ( data["fs"+i+"_read_bytes_psec"] == 0 ) {
			data["fs"+i+"_read_bytes_psec"] = 0.001;
		};

            };
	    
	    //alert(data.core1_used);
            monitoringStore.loadData([data], true);

            if ( !graph_initialized ) {
                initGraphs(node_stats);
            };
};



function initGraphs (node_stats)
{
 for ( var i=1; i<=node_stats.filesystems.device.length; i++ ) {
 var fs_used = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: "Filesystem: " + node_stats.filesystems.device[i-1]["@attributes"].path + " (" + node_stats.filesystems.device[i-1]["@attributes"].fs + ", "+node_stats.filesystems.device[i-1]["@attributes"].type+")",
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
		adjustMaximumByMajorUnit:1,
		minimum:0,
                position: 'left',
                fields: ['fs'+i+'_write_bytes_psec', 'fs'+i+'_read_bytes_psec'],
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
                yField: 'fs'+i+'_read_bytes_psec',
		title: 'Read bytes',
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
                yField: 'fs'+i+'_write_bytes_psec',
		title: 'Write bytes',
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

