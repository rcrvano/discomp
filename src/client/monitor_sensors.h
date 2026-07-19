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

#ifndef MONITOR_DATA_H
#define MONITOR_DATA_H

#include <QTime>
//#include <QTest>
#include <QSysInfo>
#include <QString>
#include <QDebug>
//#include <QDialog>
#include <QUuid>
#include <QDomDocument>
#include <QStack>
#include <QStringList>
#include <QScriptEngine>

#include "monitor_jsapi.h"

extern "C" {
#include "sigar.h"
#include "sigar_format.h"
}

//#include <iostream>


#ifdef WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif





const double KiB = 1024.;
const double MiB = KiB * 1024;
const double GiB = MiB * 1024;


const int DEFAULT_NET_STAT_FLAGS = SIGAR_NETCONN_CLIENT|SIGAR_NETCONN_SERVER|SIGAR_NETCONN_TCP;



class DiscompMonitorSensors
{
public:
    DiscompMonitorSensors();

    QString getSysInfo();
    QString getUptime();
    QString getLoadAVG();
    QString getCPUStat();
    QString getMemory();
    QString getSwap();
    QString getNetStat();
    QString getFileSystem();

    QString getProcList();
    QString getUsersList();

    QString getExternalSensors();

private:
    QScriptEngine* m_script_engine;
    DiscompMonitorJSAPI* m_discomp_monitor_api;

    QVector<sigar_cpu_t> m_cpu_prev_stats;
    QHash <QString, sigar_net_interface_stat_t> m_net_prev_stats;
    quint64 m_net_prev_stat_time;
    QHash <QString, sigar_disk_usage_t> m_fs_prev_stats;
    quint64 m_fs_prev_stat_time;
};





/*
class NetStat
{
public:
    NetStat(QString ifname);    
    ~NetStat();
    QString tcp_established;
    QString tcp_listen;
    QString tcp_time_wait;
    QString tcp_close_wait;
    QString tcp_idle;
    QString rx_rate;
    QString tx_rate;
private:
    QTime m_time;
};

*/


#endif // MONITOR_DATA_H
