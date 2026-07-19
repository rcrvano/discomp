// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#include "sysinfo.h"
#include "sigar.h"
#include "sigar_fileinfo.h"
#include "sigar_format.h"



DiscompSysInfo::DiscompSysInfo ( )
{
	this->initialized = false;
}

DiscompSysInfo::~DiscompSysInfo () 
{
}

bool DiscompSysInfo::Init ()
{
	sigar_t *sigar;
	int status = sigar_open(&sigar);
	if (status != SIGAR_OK) {
		return false;
	};
	
	//system info
	sigar_sys_info_t sysinfo;
	status = sigar_sys_info_get(sigar, &sysinfo);
	if (status != SIGAR_OK) { return false; };
	this->sys_name    = QString("%1").arg(sysinfo.name);
	this->sys_version = QString("%1").arg(sysinfo.version);
	this->sys_machine = QString("%1").arg(sysinfo.machine);
	this->sys_description = QString("%1").arg(sysinfo.description);
	this->sys_vendor  = QString("%1").arg(sysinfo.vendor);

	//cpu
	sigar_cpu_info_list_t cpu_infos;
	status = sigar_cpu_info_list_get(sigar, &cpu_infos);
	if (status != SIGAR_OK) { return false; };
	
	this->cpu_mhz    = (qint64) cpu_infos.data->mhz;
	this->cpu_vendor = QString("%1").arg(cpu_infos.data->vendor);
	this->cpu_model  = QString("%1").arg(cpu_infos.data->model);
	this->cpu_cores  = (qint64) cpu_infos.data->total_cores;
	this->cpu_sockets= (qint64) cpu_infos.data->total_sockets;
	this->cpu_cache_size = (qint64) cpu_infos.data->cache_size;
	sigar_cpu_info_list_destroy(sigar, &cpu_infos);


	this->Update();

	this->initialized = true;
};

bool DiscompSysInfo::Update ()
{
	sigar_t *sigar;
	int status = sigar_open(&sigar);
	if (status != SIGAR_OK) {
		return false;
	};

	//cpu
	sigar_cpu_t sigar_cpu;
	status = sigar_cpu_get (sigar, &sigar_cpu);
	if (status != SIGAR_OK) { return false; };
	this->cpu_idle   = (qint64)sigar_cpu.idle;
	this->cpu_total  = (qint64)sigar_cpu.total;


	//memory
	sigar_mem_t mem_info;
	status = sigar_mem_get ( sigar, &mem_info );
	if (status != SIGAR_OK) { return false; };
	this->mem_total    = (quint64) mem_info.total;
	this->mem_free     = (quint64) mem_info.free;
	this->mem_used     = (quint64) mem_info.used;
	this->mem_free_pct = mem_info.free_percent;
	this->mem_used_pct = mem_info.used_percent;

	//uptime
	sigar_uptime_t sigar_uptime;
	status = sigar_uptime_get ( sigar, &sigar_uptime ); 
	if (status != SIGAR_OK) { return false; };
	this->stat_uptime  = sigar_uptime.uptime;

	//load avg
	sigar_loadavg_t sigar_loadavg;
	status = sigar_loadavg_get ( sigar, &sigar_loadavg ); 
	if (status != SIGAR_OK) { return false; };
	this->stat_loadavg1 = sigar_loadavg.loadavg[0];
	this->stat_loadavg2 = sigar_loadavg.loadavg[1];
	this->stat_loadavg3 = sigar_loadavg.loadavg[2];


	sigar_close ( sigar );
	
	//processes
	//filesystem
	//network
	//logged in users
	return true;
}


QString DiscompSysInfo::getXML ()
{
	if ( !this->initialized ) { return false; };

	QString xml;
	xml = "<sysinfo><![CDATA[";
	xml += QString("<sys_name>%1</sys_name>").arg(this->sys_name);
	xml += QString("<sys_version>%1</sys_version>").arg(this->sys_version);
	xml += QString("<sys_machine>%1</sys_machine>").arg(this->sys_machine);
	xml += QString("<sys_description>%1</sys_description>").arg(this->sys_description);
	xml += QString("<sys_vendor>%1</sys_vendor>").arg(this->sys_vendor);
	xml += QString("<cpu_mhz>%1</cpu_mhz>").arg(this->cpu_mhz);
	xml += QString("<cpu_vendor>%1</cpu_vendor>").arg(this->cpu_vendor);
	xml += QString("<cpu_model>%1</cpu_model>").arg(this->cpu_model);
	xml += QString("<cpu_cores>%1</cpu_cores>").arg(this->cpu_cores);
	xml += QString("<cpu_sockets>%1</cpu_sockets>").arg(this->cpu_sockets);
	xml += QString("<cpu_cache_size>%1</cpu_cache_size>").arg(this->cpu_cache_size);
	xml += QString("<cpu_idle>%1</cpu_idle>").arg(this->cpu_idle);
	xml += QString("<cpu_total>%1</cpu_total>").arg(this->cpu_total);
	xml += QString("<mem_total>%1</mem_total>").arg(this->mem_total);
	xml += QString("<mem_free>%1</mem_free>").arg(this->mem_free);
	xml += QString("<mem_used>%1</mem_used>").arg(this->mem_used);
	xml += QString("<mem_free_pct>%1</mem_free_pct>").arg(this->mem_free_pct);
	xml += QString("<mem_used_pct>%1</mem_used_pct>").arg(this->mem_used_pct);
	xml += QString("<stat_uptime>%1</stat_uptime>").arg(this->stat_uptime);
	xml += QString("<stat_loadavg1>%1</stat_loadavg1>").arg(this->stat_loadavg1);
	xml += QString("<stat_loadavg2>%1</stat_loadavg2>").arg(this->stat_loadavg2);
	xml += QString("<stat_loadavg3>%1</stat_loadavg3>").arg(this->stat_loadavg3);
	xml += "]]></sysinfo>";
	return xml;
}


bool DiscompSysInfo::parseXML (const QString &xml)
{
	if ( !this->initialized ) { return false; };
	
	return true;
}

// vim: set fenc=utf-8 tabstop=8 :
