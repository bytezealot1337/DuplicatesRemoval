#ifndef COMPARETHREAD_H
#define COMPARETHREAD_H

#include <QObject>
#include <QStringList>

class CompareThread : public QObject
{
    Q_OBJECT
public:
    explicit CompareThread(QObject *parent = 0);

    QString m_sFolderPath;
    QString m_sFilter;

    QStringList fileList;
    int fileCount;

    int m_iDuplicatesCount;
    QList<int> m_iDuplicateIndexes;

    bool m_bSuspendValue;

signals:
    void workFinished();
    void duplicatesFound(QString sFileToCompare, QString sFileCompared, int i, int j);
    void compareDone(QList<int> iDuplicateIndexes, int iDuplicatesCount);
    void progress(int progressValue);

public slots:
    void dirFunction();
    void compareFiles();
    void setParameters(QString sFolderPath, QString sFilter);

};

#endif // COMPARETHREAD_H
