// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#ifndef SYSINFO_H
#define SYSINFO_H

#include <QString>

class DiscompSysInfo
{
public:
	 DiscompSysInfo ( );
	~DiscompSysInfo ( );

	bool    Init    ( );
	bool	Update  ( );

	QString getXML  ( );
	bool    parseXML( const QString & ); 

	QString sys_name;
	QString sys_version;
	QString sys_machine;
	QString sys_description;
	QString sys_vendor;
	qint64  cpu_mhz;
	QString cpu_vendor;
	QString cpu_model;
	qint64  cpu_cores;
	qint64  cpu_sockets;
	qint64  cpu_cache_size;
	qint64  cpu_idle;
	qint64  cpu_total;
	qint64  mem_total;
	qint64  mem_free;
	qint64  mem_used;
	double  mem_free_pct;
	double  mem_used_pct;
	double  stat_uptime;
	double  stat_loadavg1;
	double  stat_loadavg2;
	double  stat_loadavg3;

private:
	bool    initialized;
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
