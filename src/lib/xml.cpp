// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2008 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Author: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>      |
// +--------------------------------------------------------------------+


#include <QString>

#include "xml.h"


/**
 * @brief Additional functions for works with XML document
 *
 * This namespace contain functions for make a simple interface for works with XML documents.
 * Many of these functions included to the standart implemetation of the DOM model,
 * but QT not supported these methods. Maybe it will be supported later.
 */
namespace XML 
{
	/**
	 * This function extract data for node \a tagName from tree of \a xmlNode
	 *
	 * @par Example
	 * @code 
	 * QString xml_data = "<?xml?><doc><info><subnode>some text</subnode></info></doc>";
	 * //processing xml document
	 * ...
	 *
	 * QDomNode infoNode = rootNode.firstChild();
	 * QString subnode_text = XML::getTextNodeByTag("subnode", infoNode);
	 * //subnode_text now contain "some text"
	 * @endcode
	 *
	 *
	 * @param tagName - name of searching tag
	 * @param xmlNode - node in which will be searched \a tagName
	 * @return data from node (tagName)
	 */
	QString getTextNodeByTag ( const QString &tagName, const QDomNode &xmlNode ) 
	{
		QDomNode xml_node = xmlNode.firstChild();
		while ( !xml_node.isNull() ) {
			if ( xml_node.toElement().tagName() == tagName ) {
				QDomNode childNode = xml_node.firstChild();
				while (!childNode.isNull()) {
					if (childNode.nodeType() == QDomNode::TextNode || childNode.nodeType() == QDomNode::CDATASectionNode) {
						return childNode.toText().data();
					};
					childNode = childNode.nextSibling();
				};
			};
			xml_node = xml_node.nextSibling();
		};
		return "";
	};
	
	
	/**
	 * Search node by tag in the xmlNode tree
	 *
	 * @param tagName - searching tagName
	 * @param xmlNode - node in whcih we search \a tagName
	 * @return pointer to the found Node or on the empty node if node not found
	 */
	QDomNode getChildNodeByTag ( const QString &tagName, const QDomNode &xmlNode ) 
	{
		QDomNode xml_node = xmlNode.firstChild();
		while ( !xml_node.isNull() ) {
			if ( xml_node.toElement().tagName() == tagName ) {
				return xml_node;
			};
			xml_node = xml_node.nextSibling();
		};
	
		xml_node.clear();
		return xml_node; 
	};

	/**
	 *
	 */
	QString getNodeAttribute ( const QString& attribute, const QDomNode &xmlNode ) 
	{
		if ( xmlNode.toElement().attribute(attribute) != "" ) {
			return xmlNode.toElement().attribute(attribute);
		} else {
			return xmlNode.toElement().attribute(attribute.toLower());
		};
	}
};
// vim: set fenc=utf-8 tabstop=8 :
