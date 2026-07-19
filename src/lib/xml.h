// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#ifndef XML_H
#define XML_H

#include <QtXml>

namespace XML {
	QString   getTextNodeByTag          ( const QString &tagName,   const QDomNode &xml_node );
	QDomNode  getChildNodeByTag         ( const QString &tagName,   const QDomNode &xml_node );
	QString   getNodeAttribute          ( const QString &attribute, const QDomNode &xml_node ); 
}

#endif

// vim: set fenc=utf-8 tabstop=8 :
