#include "comparethread.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QDebug>

Q_DECLARE_METATYPE(QList<int>)

CompareThread::CompareThread(QObject *parent) :
    QObject(parent)
{
    m_iDuplicatesCount = 0;
    m_iDuplicateIndexes.clear();

    qRegisterMetaType<QList<int> >("QList<int>");

    m_bSuspendValue = false;
}

void CompareThread::setParameters(QString sFolderPath, QString sFilter)
{
    m_sFolderPath = sFolderPath;
    m_sFilter = sFilter;
}

void CompareThread::dirFunction()
{
    QDir folder(m_sFolderPath);
    m_sFolderPath = folder.absolutePath();
    QString sFilterMOD = "*" + m_sFilter;
    folder.setNameFilters(QStringList()<< sFilterMOD);
    fileList = folder.entryList();

    fileCount = fileList.count();

    compareFiles();
}

void CompareThread::compareFiles()
{
    m_iDuplicatesCount = 0;
    m_iDuplicateIndexes.clear();

    emit progress(0);

    for(int i = 0; i < fileCount; i++)
    {
        QString sFileToCompare(m_sFolderPath + "/" + fileList[i]);

        QCryptographicHash hashToCompare(QCryptographicHash::Sha1);
        QFile fileToCompare(sFileToCompare);

        if(fileToCompare.open(QIODevice::ReadOnly))
        {
            hashToCompare.addData(fileToCompare.readAll());
        }
        else
        {
            // Handle "cannot open file" error
        }

        // Retrieve the SHA1 signature of the file
        QByteArray signatureToCompare = hashToCompare.result();

        for(int j = i; j < fileCount; j++)
        {
            QString sFileCompared(m_sFolderPath + "/" + fileList[j]);

            QCryptographicHash hashCompared(QCryptographicHash::Sha1);
            QFile fileCompared(sFileCompared);

            if(fileCompared.open(QIODevice::ReadOnly))
            {
                hashCompared.addData(fileCompared.readAll());
            }
            else
            {
                // Handle "cannot open file" error
            }

            // Retrieve the SHA1 signature of the file
            QByteArray signatureCompared = hashCompared.result();

            if((i != j) && (signatureToCompare == signatureCompared))
            {
                m_iDuplicatesCount++;

                m_iDuplicateIndexes << j;

                emit duplicatesFound(fileList[i], fileList[j], i, j);
            }

            fileCompared.close();

            if(m_bSuspendValue == true)
            {
                emit workFinished();
                emit progress(0);
                return;
            }
        }

        float var = ((float)i+1.0)/(float)fileCount;
//        qDebug() << var;
        emit progress(100.0*(2.0*var-(var*var)));

        fileToCompare.close();
    }

    emit compareDone(m_iDuplicateIndexes, m_iDuplicatesCount);
    emit workFinished();
}
