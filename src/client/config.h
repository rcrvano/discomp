// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef CONFIG_H
#define CONFIG_H

#define CLIENT 1
#define SOCKET_CLIENT 1
#define VERSION "2.0b"

#define MAX_CPU_COUNT 32

#ifdef Q_OS_UNIX
#define PRIVATE_CONFIG_FILE "~/.discomp/client.conf"
#define CONFIG_FILE         "/etc/discomp/client.conf"
#else
#define PRIVATE_CONFIG_FILE "~/.discomp/client.conf"
#define CONFIG_FILE "client.conf"
#endif


#define MODULES_DIR "modules"
#define MODULE_CONFIG_FILE "module.xml"

#define LOG_FILE "client"
#ifdef Q_OS_UNIX
#define LOG_FILE_EXT "log"
#else
#define LOG_FILE_EXT "txt"
#endif


#endif

// vim: set fenc=utf-8 tabstop=8 :
