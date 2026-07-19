// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+

#include <QDir>
#include <QString>

#include "user.h"
#include "../lib/common.h"
#include "../lib/file.h"


/***********************************************************************/
/*-----------------  Constructor & Destructor  ------------------------*/
/***********************************************************************/

/* 
 * Constructor 
 * */
namespace DiscompUser 
{
	QStringList getUsersList ( ) 
	{
		QStringList return_users_list;

		QString users_file = Config::variables["BaseDirectory"] + QDir::separator() + "etc" + QDir::separator() + "users";
		
		QString file_content;
		File::read ( users_file, file_content );
		QStringList users_lines = file_content.split("\n");
		for ( int i = 0; i < users_lines.size(); ++i ) {
			return_users_list <<  users_lines.at(i).trimmed();
		};
		return return_users_list;
	};

	bool authorize ( const QString &login, const QString &password_md5, QString &user_type )
	{
		QString users_file = Config::variables["BaseDirectory"] + QDir::separator() + "etc" + QDir::separator() + "users";
		
		QString file_content;
		File::read ( users_file, file_content );
		QStringList users_lines = file_content.split("\n");
		for ( int i = 0; i < users_lines.size(); ++i ) {
			QStringList user_info = users_lines.at(i).trimmed().split(":");
			if ( user_info.size() > 2 ) {
				if ( user_info.at(0) == login && user_info.at(1).mid(0,16) == password_md5.mid(0,16) ) {
					user_type = user_info.at(2);
					return true;
				};
			};
		};
		return false;
	};
	
	void create ( const QString &login, const QString &password_md5, const QString &user_type )
	{
		QString users_file = Config::variables["BaseDirectory"] + QDir::separator() + "etc" + QDir::separator() + "users";

		QString file_content;
		File::read ( users_file, file_content );
	
		//trying to find user with this login in users file;
		bool flag = false;
		QStringList users_lines = file_content.split("\n");
		for ( int i = 0; i < users_lines.size(); ++i ) {
			QStringList user_info = users_lines[i].trimmed().split(":");
			if ( user_info.size() > 1 ) {
				if ( user_info[0] == login ) {
					flag = true;
				};
			};
			users_lines[i] = user_info.join(":");
		};
		file_content = users_lines.join("\n");

		//user not found. just add it to the end of file
		if ( !flag ) {
			file_content += login + ":" + password_md5 + ":" + user_type + "\n";
		};

		File::save ( users_file, file_content );
	}
	
	void changePassword ( const QString &login, const QString &password_md5, const QString &user_type ) 
	{
		QString users_file = Config::variables["BaseDirectory"] + QDir::separator() + "etc" + QDir::separator() + "users";

		QString file_content;
		File::read ( users_file, file_content );
	
		//trying to find user with this login in users file;
		bool flag = false;
		QStringList users_lines = file_content.split("\n");
		for ( int i = 0; i < users_lines.size(); ++i ) {
			QStringList user_info = users_lines[i].trimmed().split(":");
			if ( user_info.size() > 1 ) {
				if ( user_info[0] == login ) {
					user_info[1] = password_md5.mid(0,16);
					if ( user_type != "" && user_info.size() > 2 ) {
						user_info[2] = user_type;
					};
					flag = true;
				};
			};
			users_lines[i] = user_info.join(":");
		};
		file_content = users_lines.join("\n");

		File::save ( users_file, file_content );

	};

}


// vim: set fenc=utf-8 tabstop=8 :
