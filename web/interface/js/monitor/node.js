
    requestCurrentNodeStats();

    function initStore(node_stats)
    {
    	var fields_array = new Array(
		'interval',
		'cpu_user',
		'cpu_sys',
		'cputemp_max', 
		'cputemp_min', 
		'memory_used',
		'swap_used',
		'fs_used',
		'fs_read_psec',
		'fs_write_psec',
//		'fs_max_psec',
//		'network_max_bytes_psec',
		'network_tx_bytes_psec',
		'network_rx_bytes_psec');

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
/*
	    var fs_max_bytes_psec = 0;
	    if ( parseInt(node_stats.filesystems["@attributes"].write_bytes_psec) > parseInt(node_stats.filesystems["@attributes"].read_bytes_psec) ) {
	    	fs_max_bytes_psec = parseInt(node_stats.filesystems["@attributes"].write_bytes_psec);
            } else {
	    	fs_max_bytes_psec = parseInt(node_stats.filesystems["@attributes"].read_bytes_psec);
	    };


	    var network_max_bytes_psec = 0;
	    if ( parseInt(node_stats.network["@attributes"].tx_bytes_psec) > parseInt(node_stats.network["@attributes"].rx_bytes_psec) ) {
	    	network_max_bytes_psec = parseInt(node_stats.network["@attributes"].tx_bytes_psec) ;
            } else {
	    	network_max_bytes_psec = parseInt(node_stats.network["@attributes"].rx_bytes_psec) ;
	    };
*/
            monitoringStore.loadData([
            {
                interval: lastData.interval + 1,
                //core1_used: parseInt(node_stats.cpu.core[0]["@attributes"].used_p),
                //core1_used: parseInt(node_stats.cpu.core[1]["@attributes"].used_p),
		cpu_user: parseInt(node_stats.cpu["@attributes"].user_p),
		cpu_sys: parseInt(node_stats.cpu["@attributes"].sys_p),
                memory_used: parseInt(node_stats.memory["@attributes"].actual_used_p),
                swap_used: parseInt(node_stats.swap["@attributes"].used_p),
		fs_used: parseInt(node_stats.filesystems["@attributes"].used_p),
                fs_write_psec: parseInt(node_stats.filesystems["@attributes"].write_bytes_psec),
                fs_read_psec: parseInt(node_stats.filesystems["@attributes"].read_bytes_psec),
//		fs_max_psec: fs_max_bytes_psec,
		network_tx_bytes_psec: parseInt(node_stats.network["@attributes"].tx_bytes_psec),
		network_rx_bytes_psec: parseInt(node_stats.network["@attributes"].rx_bytes_psec),
//		network_max_bytes_psec: network_max_bytes_psec,
		cputemp_max: parseInt(node_stats.cputemp["@attributes"].max),
		cputemp_min: parseInt(node_stats.cputemp["@attributes"].min),
            }], true);


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
    var memory = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Memory used',
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
                fields: ['memory_used'],
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
                yField: 'memory_used',
		smooth: true,
		fill: true,
		highlight: {
                    size: 7,
                    radius: 7
                },
                style: {
                    fill: 'green',
                    stroke: 'green',
                    'stroke-width': 3
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'green',
                    stroke: 'green'
                }
            }]
        }
    });
    
    
    
 var cputemp = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'CPUTemp used',
        renderTo: 'graph-area',
        layout: 'fit',
        items: {
            xtype: 'chart',
            animate: false,
            store: monitoringStore,
            insetPadding: 10,
	    legend: {
		position: 'bottom',
		padding:1,
		margin:1,
		    font: {
		    family: 'Tahoma',
		    size:8
		    }
		}, 

            axes: [{
                type: 'Numeric',
                minimum: 0,
                maximum: 100,
                position: 'left',
                fields: ['cputemp_min', 'cputemp_max'],
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
	        title: 'Min',
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'cputemp_min',
		smooth: true,
		fill: true,
		highlight: {
                    size: 1,
                    radius: 1
                },
                style: {
                    fill: 'green',
                    stroke: 'green',
                    'stroke-width': 1
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'green',
                    stroke: 'green'
                }
            },
	    {
	    	title : 'Max',
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'cputemp_max',
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
            }
	    
	    ]
        }
    });
    
    
    
    
    
 var fs_used = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Fs used',
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
                fields: ['fs_used'],
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
                yField: 'fs_used',
		smooth: true,
		fill: true,
		highlight: {
                    size: 7,
                    radius: 7
                },
                style: {
                    fill: 'green',
                    stroke: 'green',
                    'stroke-width': 3
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'green',
                    stroke: 'green'
                }
            }]
        }
    });
    
    

 var fs_rw_bytes_psec = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Filesystem read/write bytes per second',
        renderTo: 'graph-area',
        layout: 'fit',
        items: {
            xtype: 'chart',
            animate: false,
            store: monitoringStore,
            insetPadding: 10,
	    legend: {
		position: 'bottom',
		padding:1,
		margin:1,
		    font: {
		    family: 'Tahoma',
		    size:8
		    }
		}, 
            axes: [{
                type: 'Numeric',
                position: 'left',
                fields: ['fs_read_psec','fs_write_psec'],
                title: false,
                grid: true,
		adjustMaximumByMajorUnit :1,
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
                yField: 'fs_write_psec',
		title: 'Write bytes',
		smooth: true,
		fill: true,
		highlight: {
                    size: 1,
                    radius: 1
                },
                style: {
                    fill: 'green',
                    stroke: 'green',
                    'stroke-width': 1
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'green',
                    stroke: 'green'
                }
            },
		{
                title: 'Read bytes',
		type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'fs_read_psec',
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
                    stroke: 'red',
                }
            }
	    ]
        }
    });



        
    

    var swap = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Swap used',
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
                fields: ['swap_used'],
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
                yField: 'swap_used',
		smooth: true,
		fill: true,
		highlight: {
                    size: 7,
                    radius: 7
                },
                style: {
                    fill: 'green',
                    stroke: 'green',
                    'stroke-width': 3
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'green',
                    stroke: 'green'
                }
            }]
        }
    });
    
    
       
       
    
    
     var cpu_used = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'CPU used',
        renderTo: 'graph-area',
        layout: 'fit',
        items: {
            xtype: 'chart',
            animate: false,
            store: monitoringStore,
            insetPadding: 10,
	    legend: {
		position: 'bottom',
		padding:1,
		margin:1,
		    font: {
		    family: 'Tahoma',
		    size:8
		    }
		}, 


            axes: [{
                type: 'Numeric',
                minimum: 0,
                maximum: 100,
                position: 'left',
                fields: ['cpu_user', 'cpu_sys'],
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
	        title: 'User',
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'cpu_user',
		smooth: true,
		fill: true,
		highlight: {
                    size: 1,
                    radius: 1
                },
                style: {
                    fill: 'green',
                    stroke: 'green',
                    'stroke-width': 1
                },
                markerConfig: {
                    type: 'circle',
                    size: 0,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'green',
                    stroke: 'green'
                }
            },
	    {
	        title: 'Sys',
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'cpu_sys',
		smooth: true,
		fill: true,
		highlight: {
                    size: 1,
                    radius: 1
                },
                style: {
                    fill: "red",
                    stroke: "red",
                    'stroke-width': 1
                },
                markerConfig: {
                    type: 'circle',
                    size: 0,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'red',
                    stroke: 'red',
                }
            }
	    
	    ]
        }
    });
   

 
    
    
 var network_bytes_psec = Ext.create('widget.panel', {
        width: 500,
        height: 300,
        title: 'Network bytes per second',
        renderTo: 'graph-area',
        layout: 'fit',
        items: {
            xtype: 'chart',
            animate: false,
            store: monitoringStore,
            insetPadding: 10,
	    legend: {
		position: 'bottom',
		padding:1,
		margin:1,
		    font: {
		    family: 'Tahoma',
		    size:8
		    }
		}, 

            axes: [{
                type: 'Numeric',
                position: 'left',
                fields: ['network_tx_bytes_psec', 'network_rx_bytes_psec'],
                title: false,
                grid: true,
		minimum:0,
		adjustMaximumByMajorUnit :1,
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
	    	title: "Send bytes",
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'network_tx_bytes_psec',
		smooth: true,
		fill: true,
		highlight: {
                    size: 1,
                    radius: 1
                },
                style: {
                    fill: 'green',
                    stroke: 'green',
                    'stroke-width': 1
                },
                markerConfig: {
                    type: 'circle',
                    size: 1,
                    radius: 1,
                    'stroke-width': 0,
                    fill: 'green',
                    stroke: 'green'
                }
            },
		{
		title: "Read bytes",
                type: 'line',
                axis: 'left',
                xField: 'interval',
                yField: 'network_rx_bytes_psec',
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
                    stroke: 'red',
                }
            }
	    ]
        }
    });













    graph_initialized = true;
};


