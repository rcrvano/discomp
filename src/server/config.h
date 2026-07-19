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

#define SERVER 1
#define SOCKET_SERVER 1
#define VERSION "2.0b"
#define SUPERUSER "admin"

#ifdef Q_OS_UNIX
#define PRIVATE_CONFIG_FILE "~/.discomp/server.conf"
#define CONFIG_FILE         "/etc/discomp/server.conf"
#else
#define PRIVATE_CONFIG_FILE "~/.discomp/server.conf"
#define CONFIG_FILE         "server.conf"
#endif



#define LOG_FILE "server"
#ifdef Q_OS_UNIX
#define LOG_FILE_EXT "log"
#else
#define LOG_FILE_EXT "txt"
#endif


#define PACKAGES_DIR "packages"
#define PACKAGE_PARAMETERS_FILE "parameters.xml"

#define MODULES_DIR "modules"
#define MODULE_CONFIG_FILE "module.xml"

#define PROCESSES_DIR "schemes"
#define SCHEME_CONFIG_FILE "process.xml"
#define SCHEME_JS_CONFIG_FILE "process.js"
#define SCHEME_JS_TYPE "EDP"

#define PARAMETERS_DIR "parameters"

#endif

// vim: set fenc=utf-8 tabstop=8 :
