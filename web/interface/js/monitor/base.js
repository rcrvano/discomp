/*

This file is part of Ext JS 4

Copyright (c) 2011 Sencha Inc

Contact:  http://www.sencha.com/contact

GNU General Public License Usage
This file may be used under the terms of the GNU General Public License version 3.0 as published by the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.  Please review the following information to ensure the GNU General Public License version 3.0 requirements will be met: http://www.gnu.org/copyleft/gpl.html.

If you are unsure which license is appropriate for your use, please contact the sales department at http://www.sencha.com/contact.

*/

Ext.require('Ext.chart.*');
Ext.require('Ext.layout.container.Fit');



var monitoringStore;
/*
Ext.onReady(function() 
{
*/
    var json_stats; 
    var statsLoadTimer;
    function requestCurrentNodeStats() {
        clearTimeout(statsLoadTimer);
        statsLoadTimer = setTimeout(function() {
		Ext.Ajax.request({
                        waitMsg: 'Please Wait',
                        url: "/index.php?component=monitor&action=get_current_data",
                        success:function(response,options)
			{
				json_stats = Ext.decode(response.responseText);
				processCurrentNodeStats(json_stats);
                        }
                    });
		requestCurrentNodeStats();
        }, 1000);
    }
    


    function initDefaultStore(json_stats)
    {
    	    var data=[];

            data.push({
                interval:0
            });

            for (var i = 1; i < 100; i++) {
                data.push({
                    interval: i,
                });
            };

            monitoringStore.loadData(data);
    };

//});
//
/*
monitoringStore = Ext4.create('store.json', { fields: [
    	'interval',
	'cpu_user',
	'cpu_sys',
	'cputemp', 
	'memory_used',
	'swap_used',
	'fs_used',
	'fs_read_psec',
	'fs_write_psec',
	'network_tx_bytes_psec',
	'network_rx_bytes_psec',
	'network1_rx_bytes_psec',
	'network2_rx_bytes_psec',
	'network3_rx_bytes_psec',
	'network4_rx_bytes_psec',
	'network5_rx_bytes_psec',
	'network6_rx_bytes_psec',
	'fs1_read_bytes_psec',
	'fs2_read_bytes_psec',
	'fs3_read_bytes_psec',
	'fs4_read_bytes_psec',
	'fs5_read_bytes_psec',
	'fs6_read_bytes_psec',
	'fs1_write_bytes_psec',
	'fs2_write_bytes_psec',
	'fs3_write_bytes_psec',
	'fs4_write_bytes_psec',
	'fs5_write_bytes_psec',
	'fs6_write_bytes_psec',

	'core1_used',
	] });
    initCurrentNodeStats();
    requestCurrentNodeStats();

*/


    requestCurrentNodeStats();


