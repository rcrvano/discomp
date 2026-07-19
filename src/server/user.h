// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+
// | Application      : Non-GUI Discomp Server                          |
// | File description : Connected Discomp Node class header             |
// | File $Id: user.h,v 1.3 2009/06/23 12:54:18 discomp Exp $          |
// +--------------------------------------------------------------------+


#ifndef DISCOMPUSER_H
#define DISCOMPUSER_H

#include <QString>
#include <QStringList>

namespace DiscompUser {
	QStringList getUsersList ( ); 
	bool authorize ( const QString &login, const QString &password_md5, QString &user_type );
	void create    ( const QString &login, const QString &password_md5, const QString &user_type );
	void changePassword ( const QString &login, const QString &password_md5, const QString& user_type );
};

#endif
// vim: set fenc=utf-8 tabstop=8 :
