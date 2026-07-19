// +--------------------------------------------------------------------+
// | Discomp : Distributed Computing System of Modular Programming      |
// +--------------------------------------------------------------------+
// | Copyright (C) 2004-2011 ISDCT                                      |
// | Institute of System Dynamics and Control Theory SB RAS             |
// | Unauthorized reproduction is not allowed                           |
// +--------------------------------------------------------------------+
// | Authors: Sidorov Ivan <ivan@sidorov.net> <ivan.sidorov@icc.ru>     |
// +--------------------------------------------------------------------+


#ifndef DISCOMPMONITORRRD_H
#define DISCOMPMONITORRRD_H


#include <QObject>
#include <QtXml>



class DiscompXMLRRD 
{
public:
     DiscompXMLRRD          (  );
    ~DiscompXMLRRD          (                     );



    void  Init ();
    void  setData ( const QString & );
    void  setSubdirPath ( const QString & path );

private:

    QDomDocument Agregate ( int, int );
    void attachStats( QDomDocument *agregateDoc, QDomDocument *currentDoc);
    void mergeDocs( QDomNode agregateNode, QDomNode targetNode);
    void divideByCounter( QDomNode agregateNode, int counter );
    void dumpArchiveToDisk ( int rra_num ); 

    QVector < QPair<int,int> > m_rra_conf;
    QVector < QVector<QDomDocument>* > m_rra_data;
    quint64 m_rra_cycle_step;

    QString m_subdir_path;
signals:

public slots:

};

#endif // DISCOMPMONITOR_H
