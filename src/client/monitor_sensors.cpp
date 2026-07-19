/****************************************************************************
** Copyright (C) 2010 Frédéric Logier
** Contact: Frédéric Logier <frederic@logier.org>
**
** http://gitorious.org/nodecast/nodecast-gui
**
** This file is part of Nodecast.
**
** Nodecast is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Nodecast is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Nodecast.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

// check for lib sigar usage : http://www.hypertable.org/doxygen/_system_info_8cc_source.html

#include <QFile>

#include "monitor_sensors.h"
#include "../lib/common.h"
#include "../lib/log.h"
/*
extern "C" {
#include <poll.h>
#include <curses.h>
#include <term.h>
#include <sigar-1.6.3/include/sigar.h>
#include <sigar-1.6.3/include/sigar_format.h>
}
*/


DiscompMonitorSensors::DiscompMonitorSensors()
{
    m_net_prev_stat_time = 0;
    m_fs_prev_stat_time = 0;



    //Sensors API
    m_script_engine = new QScriptEngine( );
    m_discomp_monitor_api = new DiscompMonitorJSAPI();
    QScriptValue discomp_monitor_api_sv = m_script_engine->newQObject(m_discomp_monitor_api);
    m_script_engine->globalObject().setProperty("MonitorAPI", discomp_monitor_api_sv);


    QString fileName = Config::variables.value("MonitorSensorPath");
    QFile scriptFile(fileName);
    if (scriptFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&scriptFile);
        QString contents = stream.readAll();
        scriptFile.close();

        m_script_engine->evaluate(contents, fileName);
    };
/*
    if (myEngine.hasUncaughtException()) {
        int line = myEngine.uncaughtExceptionLineNumber();
        qDebug() << "uncaught exception at line" << line << ":" << result.toString();
    }
*/

};


QString DiscompMonitorSensors::getSysInfo()
{
    QString xml_data = "";

    int status;
    sigar_t *sigar;
    sigar_sys_info_t sysinfo;

    sigar_open(&sigar);
    status = sigar_sys_info_get(sigar, &sysinfo);

    xml_data += "<sysinfo ";
    if (status == SIGAR_OK) {
        xml_data += QString("name='%1' ").arg(sysinfo.name);
        xml_data += QString("version='%1' ").arg(sysinfo.version);
        xml_data += QString("arch='%1' ").arg(sysinfo.arch);
//        xml_data += QString("machine='%1' ").arg(sysinfo.machine);
        xml_data += QString("description='%1' ").arg(sysinfo.description);
//        xml_data += QString("patch_level='%1' ").arg(sysinfo.patch_level);
//        xml_data += QString("vendor='%1' ").arg(sysinfo.vendor);
//        xml_data += QString("vendor_version='%1' ").arg(sysinfo.vendor_version);
//        xml_data += QString("vendor_name='%1' ").arg(sysinfo.vendor_name);
//        xml_data += QString("vendor_code_name='%1'").arg(sysinfo.vendor_code_name);
    };


    sigar_net_info_t ni;
    status = sigar_net_info_get(sigar, &ni);    
    if (status == SIGAR_OK) {
        if ( !QString(ni.host_name).isEmpty() )
            xml_data += QString ("hostname='%1' ").arg(ni.host_name);
/*
        if ( !QString(ni.default_gateway).isEmpty() )
            xml_data += QString ("\t<gateway>%1</gateway>\n").arg(ni.default_gateway);
        if ( !QString(ni.domain_name).isEmpty() && QString(ni.domain_name) != "(none)" )
            xml_data += QString ("\t<domain>%1</domain>\n").arg(ni.domain_name);
        if ( !QString(ni.primary_dns).isEmpty() )
            xml_data += QString ("\t<dns1>%1</dns1>\n").arg(ni.primary_dns);
        if ( !QString(ni.secondary_dns).isEmpty() )
            xml_data += QString ("\t<dns2>%1</dns2>\n").arg(ni.secondary_dns);
*/
    }


    xml_data += "/>\n";


    sigar_close(sigar);

    return xml_data;
}


QString DiscompMonitorSensors::getUptime()
{
    int status;
    sigar_t *sigar;
    QString xml_data = "";


    sigar_open(&sigar);

    //uptime
    sigar_uptime_t sigar_uptime;
    status = sigar_uptime_get ( sigar, &sigar_uptime );
    if (status == SIGAR_OK) {

    //QString uptime_days = ""
        char b[256];
    int status2 = sigar_uptime_string(sigar, &sigar_uptime, b, sizeof(b));
    //if ( status2 == SIGAR_OK ) { uptime_days = "" };
        xml_data = QString("<uptime sec='%1' readable='%2'/>\n").arg(sigar_uptime.uptime, 0, 'f',0).arg( ( status2 == SIGAR_OK ) ? b : "" );

    }
    sigar_close(sigar);

    return xml_data;
}

QString DiscompMonitorSensors::getLoadAVG()
{
    int status;
    sigar_t *sigar;
    QString xml_data;


    sigar_open(&sigar);

    //load avg
    sigar_loadavg_t sigar_loadavg;
    status = sigar_loadavg_get ( sigar, &sigar_loadavg );
    if (status == SIGAR_OK) {
        xml_data = QString("<loadavg avg1='%1' avg2='%2' avg3='%3'/>\n").arg((float)sigar_loadavg.loadavg[0]).arg(sigar_loadavg.loadavg[1]).arg(sigar_loadavg.loadavg[2]);
    };
    sigar_close(sigar);

    return xml_data;

};




QString DiscompMonitorSensors::getCPUStat()
{
    int status1, status2;
    sigar_t *sigar;
    QString xml_data ="";

    sigar_open(&sigar);

    sigar_cpu_info_list_t cpuinfolist;
    status1 = sigar_cpu_info_list_get(sigar, &cpuinfolist);

    sigar_cpu_list_t cpulist;
    status2 = sigar_cpu_list_get(sigar, &cpulist);


    sigar_cpu_info_t cpu = cpuinfolist.data[0];

    int counter = 0;
    int common_wait_p = 0;
    int common_nice_p = 0;
    int common_idle_p = 0;
    int common_user_p = 0;
    int common_sys_p = 0;
    if (status1 == SIGAR_OK && status2 == SIGAR_OK) {

        for ( uint i=0; i<cpulist.number; i++) {
            sigar_cpu_t cpustats = cpulist.data[i];

            if ( this->m_cpu_prev_stats.size() < (int)i+1 ) {
               this->m_cpu_prev_stats.push_back(cpustats);
               return "";
            };

            quint64 total_diff = cpustats.total - this->m_cpu_prev_stats.at(i).total;


            int wait_p = (int)(((float)(cpustats.wait - this->m_cpu_prev_stats.at(i).wait)/total_diff)*100);
            if (wait_p < 0 ) { wait_p = 0; };
            int nice_p = (int)(((float)(cpustats.nice - this->m_cpu_prev_stats.at(i).nice)/total_diff)*100);
            if (nice_p < 0 ) { nice_p = 0; };
            int user_p = (int)(((float)(cpustats.user - this->m_cpu_prev_stats.at(i).user)/total_diff)*100);
            if (user_p < 0 ) { user_p = 0; };
            int sys_p = (int)(((float)(cpustats.sys - this->m_cpu_prev_stats.at(i).sys)/total_diff)*100);
            if ( sys_p < 0 ) { sys_p = 0; };
            int idle_p = (int)(((float)(cpustats.idle - this->m_cpu_prev_stats.at(i).idle)/total_diff)*100);
            if ( idle_p < 0 ) { idle_p = 0; };

            common_wait_p += wait_p;
            common_nice_p += nice_p;
            common_idle_p += idle_p;
            common_user_p += user_p;
            common_sys_p  += sys_p;
            counter++; 

            xml_data += QString("\t<core num='%1' user_p='%2' sys_p='%3' wait_p='%4' nice_p='%5' used_p='%6' idle_p='%7' />\n")
                .arg(i)
                .arg( user_p )
                .arg( sys_p )
                .arg( wait_p )
                .arg( nice_p )
                .arg( 100 - idle_p ) 
                .arg( idle_p )
                ;

            this->m_cpu_prev_stats[i] = cpustats;
        };
    };
    if (  counter == 0 ) { counter = 1; };

    xml_data = QString("<cpu vendor='%1' model='%2' mhz='%3' cache_size='%4' total_sockets='%5' total_cores='%6' cores_per_socket='%7' wait_p='%8' nice_p='%9' idle_p='%10' user_p='%11' sys_p='%12' used_p='%13'>\n"+xml_data+"</cpu>\n")
	.arg(cpu.vendor)
	.arg(cpu.model)
	.arg(cpu.mhz)
	.arg(cpu.cache_size)
	.arg(cpu.total_sockets)
	.arg(cpu.total_cores)
	.arg(cpu.cores_per_socket)
        .arg( common_wait_p/counter )
        .arg( common_nice_p/counter )
        .arg( common_idle_p/counter )
        .arg( common_user_p/counter )
        .arg( common_sys_p/counter )
        .arg( 100 - common_idle_p/counter ) 
 	;


    sigar_cpu_info_list_destroy(sigar, &cpuinfolist);
    sigar_cpu_list_destroy(sigar, &cpulist);
    sigar_close(sigar);

    return xml_data;
}


QString DiscompMonitorSensors::getMemory ()
{
    int status;
    sigar_t *sigar;
    sigar_mem_t mem;

    sigar_open(&sigar);
    status = sigar_mem_get(sigar, &mem);


    QString xml_string = "<memory ";
    if (status == SIGAR_OK) {

        xml_string += QString("total='%1' ").arg(mem.total );
        xml_string += QString("used='%1' ").arg(mem.used );
//        xml_string += QString("free='%1' ").arg(mem.free / MiB);
        if ( mem.total != 0 ) {
            xml_string += QString("used_p='%1' ").arg((int) (mem.used*100/mem.total));
            xml_string += QString("free_p='%1' ").arg(100-(int) (mem.used*100/mem.total));
        };
        xml_string += QString("actual_used='%1' ").arg(mem.actual_used );
        if ( mem.total != 0 ) {
            xml_string += QString("actual_used_p='%1' ").arg((int)(mem.actual_used*100/mem.total));
        };
    /*
        actual_free.setNum(mem.actual_free / MiB);
        actual_used.setNum(mem.actual_used / MiB);
    */
    };
    xml_string += "/>\n";
    sigar_close(sigar);

    return xml_string;
};


QString DiscompMonitorSensors::getSwap ()
{
    int status;
    sigar_t *sigar;
    sigar_swap_t swap;

    sigar_open(&sigar);
    status = sigar_swap_get(sigar, &swap);

    QString xml_string = "<swap ";
    if (status == SIGAR_OK) {
        xml_string += QString("total='%1' ").arg(swap.total);
        xml_string += QString("used='%1' ").arg(swap.used);
//        xml_string += QString("free='%1' ").arg(swap.free / MiB);
        //xml_string += QString("page_in='%1' ").arg(swap.page_in);
        //xml_string += QString("page_out='%1'").arg(swap.page_out);
        if ( swap.total != 0 ) {
            xml_string += QString("used_p='%1' ").arg((int)(swap.used*100/swap.total));
            xml_string += QString("free_p='%1'").arg(100-(int)(swap.used*100/swap.total));
        };
    }
    xml_string += "/>\n";

    sigar_close(sigar);
    return xml_string;
};





/*
* FileSystem CLASS
*/


QString DiscompMonitorSensors::getFileSystem()
{
    int status;
    sigar_t *sigar;
    sigar_file_system_list_t fslist;


    int counter=0;
    int common_used_p=0;
    int common_free_p=0;
    quint64 common_total=0;
    quint64 common_used=0;
    quint64 common_read_bytes_psec=0;
    quint64 common_write_bytes_psec=0;
  //  qint64 start_microtime = QDateTime::currentMSecsSinceEpoch ();

    sigar_open(&sigar);
    status = sigar_file_system_list_get(sigar, &fslist);


    QString prev_dir_name;
    QString xml_data = "";
    if (status == SIGAR_OK) {
        for ( uint i=0; i < fslist.number; i++ ) {
//    xml_data += QString("<m%2>%1</m%2>\n").arg(QDateTime::currentMSecsSinceEpoch () - start_microtime).arg(i);
            QString dir_name = QString("%1").arg(fslist.data[i].dir_name);
            if ( dir_name.isNull() ) {
                    break;
            };

            if ( dir_name.startsWith("/proc/")||
                 dir_name == "/proc" ||
                 dir_name.startsWith("/sys/")||
                 dir_name == "/sys" ||
                 dir_name.startsWith("/dev/")||
                 dir_name == "/dev" ||
                 dir_name.startsWith("/lib/")||
                 dir_name == "/lib" ||
                 dir_name.startsWith("/lib32/")||
                 dir_name == "/lib32" ||
                 dir_name.startsWith("/lib64/")||
                 dir_name == "/lib64" ||
                 dir_name.startsWith("/run/")||
                 dir_name == "/run" ||
                 dir_name == prev_dir_name ||
                 QString("%1").arg(fslist.data[i].type_name) == "none"
            ) {
                continue;
            };
            prev_dir_name = dir_name;



            sigar_file_system_usage_t fsusage;
            int status2 = sigar_file_system_usage_get(sigar, fslist.data[i].dir_name, &fsusage);
            if (status2 == SIGAR_OK && fsusage.total != 0) {
            
                xml_data += "\t<device ";
                xml_data += QString("path='%1' ").arg(fslist.data[i].dir_name);
                xml_data += QString("type='%1' ").arg(fslist.data[i].type_name);
                xml_data += QString("fs='%1' ").arg(fslist.data[i].sys_type_name);
        


                xml_data += QString("total='%1' ").arg(fsusage.total);
                xml_data += QString("used='%1' ").arg(fsusage.used);
                if ( fsusage.total != 0 ) {
                    xml_data += QString("used_p='%1' ").arg((int)(fsusage.used*100/fsusage.total));
                    xml_data += QString("free_p='%1' ").arg(100-(int)(fsusage.used*100/fsusage.total));
                };

                //get disk info2
                sigar_disk_usage_t disk;
                int status3 = sigar_disk_usage_get(sigar, fslist.data[i].dir_name, &disk);
                if (status3 == SIGAR_OK) {
                    xml_data += QString("reads='%1' ").arg(disk.reads);
                    xml_data += QString("writes='%1' ").arg(disk.writes);
                    xml_data += QString("rtime='%1' ").arg(disk.rtime);
                    xml_data += QString("wtime='%1' ").arg(disk.wtime);
                    xml_data += QString("write_bytes='%1' ").arg(disk.write_bytes);
                    xml_data += QString("read_bytes='%1' ").arg(disk.read_bytes);
                    //xml_data += QString("queue='%1</queue>\n").arg(disk.queue);

                    if ( m_fs_prev_stat_time != 0 ) { 
                        quint64 time_diff = (QDateTime::currentMSecsSinceEpoch () - m_fs_prev_stat_time ) /1000;
                        if ( m_fs_prev_stats.contains(dir_name) && time_diff > 0  ) {
                            xml_data += QString("reads_psec='%1' ").arg( (disk.reads - m_fs_prev_stats[dir_name].reads)/time_diff );
                            xml_data += QString("writes_psec='%1' ").arg( (disk.writes - m_fs_prev_stats[dir_name].writes)/time_diff );
                            xml_data += QString("read_bytes_psec='%1' ").arg( (disk.read_bytes - m_fs_prev_stats[dir_name].read_bytes)/time_diff );
                            xml_data += QString("write_bytes_psec='%1' ").arg( (disk.write_bytes - m_fs_prev_stats[dir_name].write_bytes)/time_diff );

                            common_read_bytes_psec+=(disk.read_bytes - m_fs_prev_stats[dir_name].read_bytes)/time_diff;
                            common_write_bytes_psec+=(disk.write_bytes - m_fs_prev_stats[dir_name].write_bytes)/time_diff;
                        };
                    };

                    m_fs_prev_stats[dir_name] = disk;
                }
                xml_data += "/>\n";

               counter++;
               common_used_p+= (int)(fsusage.used*100/fsusage.total) ;
               common_free_p+= 100 - (int)(fsusage.used*100/fsusage.total);
               common_total+=fsusage.total;
               common_used+=fsusage.used;
            }; //if sigar ok
            
        }
    }
    if ( counter == 0 ) { counter=1; };
    xml_data = QString("<filesystems used_p='%1' free_p='%2' total='%3' used='%4' read_bytes_psec='%5' write_bytes_psec='%6'>\n"+xml_data+"</filesystems>\n")
                .arg(common_used_p/counter)
                .arg(common_free_p/counter)
                .arg(common_total)
                .arg(common_used)
                .arg(common_read_bytes_psec)
                .arg(common_write_bytes_psec)
                ;

    m_fs_prev_stat_time = QDateTime::currentMSecsSinceEpoch ();

    sigar_file_system_list_destroy(sigar, &fslist);
    sigar_close(sigar);
    return xml_data;
}





QString DiscompMonitorSensors::getNetStat()
{    
    int status;
    QString tmp;
    sigar_t *sigar;
    sigar_net_stat_t s;
    QString primary_ifname = "";

    sigar_open(&sigar);
  


    //get primary interface name
    sigar_net_interface_config_t ifc;
    if ( sigar_net_interface_config_primary_get(sigar, &ifc) == SIGAR_OK) {
        primary_ifname = ifc.name;
    };

    QString xml_data = "";

    int counter = 0;
    int common_rx_bytes_psec = 0;
    int common_tx_bytes_psec = 0;

    //netinfo
//    status = sigar_net_stat_get(sigar, &s, DEFAULT_NET_STAT_FLAGS);
    sigar_net_interface_list_t iflist;
    status = sigar_net_interface_list_get (sigar, &iflist);
    if ( status == SIGAR_OK ) {

        for ( uint i=0; i < iflist.number; i++ ) {
//            Log::addMessage(LOG_COMMON, "monitor", QString ("%1"). arg(i) );
            QString ifname = QString("%1").arg(iflist.data[i]);
            if ( ifname.isEmpty() ) {
                break;
            };
            
            xml_data += QString("\t<interface name='%1'%2 ").arg(ifname)
                    .arg( ( ifname == primary_ifname ) ? " primary='1'" : "" );

            sigar_net_interface_config_t ifconfig;
            int status3 = sigar_net_interface_config_get(sigar,iflist.data[i], &ifconfig);
            if ( status3 == SIGAR_OK ) {
                xml_data += QString("type='%1' ").arg(ifconfig.type);
//                xml_data += QString("description='%1' ").arg(ifconfig.description);
                xml_data += QString("mtu='%1' ").arg(ifconfig.mtu);
                xml_data += QString("metric='%1' ").arg(ifconfig.metric);

                char addrbuf[SIGAR_INET6_ADDRSTRLEN];
                sigar_net_address_to_string(sigar, &ifconfig.address, addrbuf);
                xml_data += QString("ip='%1' ").arg(addrbuf);
                sigar_net_address_to_string(sigar, &ifconfig.hwaddr, addrbuf);
                xml_data += QString("hwaddr='%1' ").arg(addrbuf);

//                xml_data += "/>\n";
            };

            sigar_net_interface_stat_t ifstat;
            int status2 = sigar_net_interface_stat_get(sigar, iflist.data[i], &ifstat);
            if ( status2 == SIGAR_OK ) {
//                xml_data += "\t\t<stats ";
//                xml_data += QString("speed='%1' ").arg(ifstat.speed);
                xml_data += QString("rx_bytes='%1' ").arg(ifstat.rx_bytes);
                xml_data += QString("rx_dropped='%1' ").arg(ifstat.rx_dropped);
                xml_data += QString("rx_errors='%1' ").arg(ifstat.rx_errors);
                xml_data += QString("rx_frame='%1' ").arg(ifstat.rx_frame);
                xml_data += QString("rx_overruns='%1' ").arg(ifstat.rx_overruns);
                xml_data += QString("rx_packets='%1' ").arg(ifstat.rx_packets);
                xml_data += QString("tx_bytes='%1' ").arg(ifstat.tx_bytes);
                xml_data += QString("tx_dropped='%1' ").arg(ifstat.tx_dropped);
                xml_data += QString("tx_errors='%1' ").arg(ifstat.tx_errors);
                xml_data += QString("tx_overruns='%1' ").arg(ifstat.tx_overruns);
                xml_data += QString("tx_packets='%1' ").arg(ifstat.tx_packets);
                xml_data += QString("tx_collisions='%1' ").arg(ifstat.tx_collisions);
                xml_data += QString("tx_carrier='%1' ").arg(ifstat.tx_carrier);
                
                if ( m_net_prev_stat_time != 0 ) { 
                    quint64 time_diff = (QDateTime::currentMSecsSinceEpoch () - m_net_prev_stat_time ) /1000;
                    if ( m_net_prev_stats.contains(ifname) && time_diff > 0  ) {
                        xml_data += QString("rx_bytes_psec='%1' ").arg((ifstat.rx_bytes-m_net_prev_stats[ifname].rx_bytes)/time_diff);
                        xml_data += QString("tx_bytes_psec='%1' ").arg((ifstat.tx_bytes-m_net_prev_stats[ifname].tx_bytes)/time_diff);
			common_rx_bytes_psec += (ifstat.rx_bytes-m_net_prev_stats[ifname].rx_bytes)/time_diff;
			common_tx_bytes_psec += (ifstat.tx_bytes-m_net_prev_stats[ifname].tx_bytes)/time_diff;
                    };
                }; 
//            xml_data += "/>\n";
            };
            m_net_prev_stats[ifname] = ifstat; 
            
            xml_data += "/>\n";
     };
    };
    xml_data = QString("<network rx_bytes_psec='%1' tx_bytes_psec='%2'>\n"+xml_data+"</network>\n").arg(common_rx_bytes_psec).arg(common_tx_bytes_psec);

    sigar_net_interface_list_destroy (sigar, &iflist);
    sigar_close(sigar);

    
    m_net_prev_stat_time = QDateTime::currentMSecsSinceEpoch ();

    return xml_data;
};



QString DiscompMonitorSensors::getUsersList()
{
    int status;
    sigar_t *sigar;
    QString xml_data;

    sigar_open(&sigar);

    xml_data = "<users>\n";
    sigar_who_list_t wholist;
    status = sigar_who_list_get ( sigar, &wholist );
    if ( status == SIGAR_OK ) {
        for ( uint i=0; i < wholist.number; i++ ) {
        xml_data += "\t<user ";
        xml_data += QString("login='%1' ").arg(wholist.data[i].user);
        xml_data += QString("device='%1' ").arg(wholist.data[i].device);
        xml_data += QString("host='%1'").arg(wholist.data[i].host);
        xml_data += "/>\n";
    };
    };
    xml_data += "</users>\n";


    sigar_who_list_destroy ( sigar, &wholist );
    sigar_close(sigar);

    return xml_data;

};











QString DiscompMonitorSensors::getProcList () 
{
    int status;
    sigar_t *sigar;
    sigar_proc_list_t proclist;
    unsigned long i;

    sigar_open(&sigar);
    status = sigar_proc_list_get(sigar, &proclist);

    if (status != SIGAR_OK) {
        return "";
    };

    QString xml_string = "<processes ";

    sigar_proc_stat_t procstat;
    int status2 = sigar_proc_stat_get(sigar, &procstat);
    if ( status2 == SIGAR_OK ) {
        xml_string += QString ("total='%1' sleeping='%2' running='%3' zombie='%4' stopped='%5' idle='%6' threads='%7'").arg(procstat.total).arg(procstat.sleeping).arg(procstat.running).arg(procstat.zombie).arg(procstat.stopped).arg(procstat.idle).arg(procstat.threads);
    };
    xml_string += ">\n";



    for (i=0; i<proclist.number; i++) 
    {

        xml_string += "\t<process>\n";

        sigar_pid_t pid = proclist.data[i];        
        sigar_proc_state_t proc_state;
        status = sigar_proc_state_get(sigar, pid, &proc_state);
        if (status == SIGAR_OK) {
            xml_string += "\t\t<info ";
            xml_string += QString("name='%1' ").arg(proc_state.name);
            xml_string += QString("pid='%1' ").arg((long)pid);
            xml_string += QString("ppid='%1' ").arg(proc_state.ppid);
            
            sigar_proc_cred_name_t proccredname;
            int status2 = sigar_proc_cred_name_get(sigar, pid, &proccredname);
            if (status2 == SIGAR_OK) {
                xml_string += QString("user='%1' ").arg(proccredname.user);
                xml_string += QString("group='%1' ").arg(proccredname.group);
            }
            xml_string += "/>\n";

            xml_string += "\t\t<status ";
            xml_string += QString("state='%1' ").arg(proc_state.state);
            xml_string += QString("priority='%1' ").arg(proc_state.priority);
            xml_string += QString("nice='%1' ").arg(proc_state.nice);
            xml_string += QString("cpu='%1' ").arg(proc_state.processor);
            xml_string += QString("threads='%1' ").arg(proc_state.threads);
            //xml_string += QString("tty='%1' ").arg(proc_state.tty);
            sigar_proc_fd_t proc_fd;
            int status4 = sigar_proc_fd_get(sigar, pid, &proc_fd);
            if (status4 == SIGAR_OK) {
                xml_string += QString("filedesc='%1' ").arg(proc_fd.total);
            }
            xml_string += "/>\n";

        };


//        sigar_proc_time_t proc_time;
//        status = sigar_proc_time_get(sigar, pid, &proc_time);
//        if (status == SIGAR_OK) {
//        xml_string += QString("time_start_time='%1' ").arg(proc_time.start_time);
//        xml_string += QString("time_user='%1' ").arg(proc_time.user);
//        xml_string += QString("time_sys='%1' ").arg(proc_time.sys);
//        xml_string += QString("time_total='%1' ").arg(proc_time.total);
//        }

        sigar_proc_cpu_t proc_cpu;
        status = sigar_proc_cpu_get(sigar, pid, &proc_cpu);
        if (status == SIGAR_OK) {
            xml_string += "\t\t<cpu "; 
            xml_string += QString("start_time='%1' ").arg(proc_cpu.start_time);
            xml_string += QString("last_time='%1' ").arg(proc_cpu.last_time);
            xml_string += QString("user='%1' ").arg(proc_cpu.user);
            xml_string += QString("sys='%1' ").arg(proc_cpu.sys);
            xml_string += QString("total='%1' ").arg(proc_cpu.total);
            xml_string += QString("percent='%1' ").arg( (int)(proc_cpu.percent*10000));
            xml_string += "/>\n";
        }


        sigar_proc_mem_t proc_mem;
        status = sigar_proc_mem_get(sigar, pid, &proc_mem);
        if (status == SIGAR_OK) {
            xml_string += "\t\t<memory "; 
            xml_string += QString("size='%1' ").arg(proc_mem.size);
            xml_string += QString("resident='%1' ").arg(proc_mem.resident);
            xml_string += QString("share='%1' ").arg(proc_mem.share);
            xml_string += QString("minor_faults='%1' ").arg(proc_mem.minor_faults);
            xml_string += QString("major_faults='%1' ").arg(proc_mem.major_faults);
            xml_string += QString("page_faults='%1' ").arg(proc_mem.page_faults);
            xml_string += "/>\n";
        }


        xml_string += "\t</process>\n";


    }

    xml_string += "</processes>\n";


    sigar_proc_list_destroy(sigar, &proclist);

    sigar_close(sigar);


    return xml_string;
};


QString DiscompMonitorSensors::getExternalSensors()
{
    QScriptValue out = m_script_engine->evaluate("getData()");
/*
    if (myEngine.hasUncaughtException()) {
        int line = myEngine.uncaughtExceptionLineNumber();
        qDebug() << "uncaught exception at line" << line << ":" << result.toString();
    }
*/
    return out.toString();
};


