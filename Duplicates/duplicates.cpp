#include "duplicates.h"
#include "ui_duplicates.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QDebug>

Q_DECLARE_METATYPE(QList<int>)

Duplicates::Duplicates(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Duplicates)
{
    ui->setupUi(this);

    qRegisterMetaType<QList<int> >("QList<int>");

    m_sFilter = ".jpg";
    this->ui->filterLineEdit->setText(m_sFilter);

    m_iDuplicatesCount = 0;

    compareState = false;
    this->ui->suspendButton->setEnabled(false);

    this->ui->compareButton->setEnabled(false);
    this->ui->actionCompare->setEnabled(false);

    connect(this->ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutFunction()));
    connect(this->ui->actionBrowse, SIGNAL(triggered()), this, SLOT(on_browseButton_clicked()));
    connect(this->ui->actionCompare, SIGNAL(triggered()), this, SLOT(on_compareButton_clicked()));
    connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(this->ui->actionReset, SIGNAL(triggered()), this, SLOT(on_resetButton_clicked()));
}

Duplicates::~Duplicates()
{
    delete ui;
}

void Duplicates::on_browseButton_clicked()
{
    QFileDialog *fileDialog = new QFileDialog;
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setOption(QFileDialog::ShowDirsOnly);
    fileDialog->setViewMode(QFileDialog::Detail);

    int result = fileDialog->exec();

    if(result)
    {
        folderString = fileDialog->selectedFiles()[0];
        dirFunction();
    }
    else
    {
        fileDialog->close();
    }
}

void Duplicates::dirFunction()
{
    QDir folder(folderString);
    folderString = folder.absolutePath();
    QString sFilterMOD = "*" + m_sFilter;
    folder.setNameFilters(QStringList()<< sFilterMOD);
    fileList = folder.entryList();

    fileCount = fileList.count();

    if(fileCount == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("No files with this extension in folder...");
        msgBox.exec();

        return;
    }
    else if(fileCount >= 1000000)
    {
        QMessageBox msgBox;
        msgBox.setText("There is too many files to compare...");
        msgBox.exec();

        return;
    }
    else if(fileCount > 0 && fileCount < 1000000)
    {
        this->ui->duplicatesTextEdit->clear();

        this->ui->filterLineEdit->setEnabled(false);
        this->ui->browseButton->setEnabled(false);
        this->ui->actionBrowse->setEnabled(false);
        this->ui->compareButton->setEnabled(true);
        this->ui->actionCompare->setEnabled(true);

        this->ui->duplicatesTextEdit->append("FOLDER : " + folderString);
        this->ui->duplicatesTextEdit->append("-------------------------------------------------------------");

        this->ui->detailsLabel->setText("FileCount :  " + QString::number(fileCount));
    }
}

void Duplicates::on_filterLineEdit_textChanged(const QString &arg1)
{
    m_sFilter = QString(arg1);
}

void Duplicates::on_compareButton_clicked()
{
    compareState = true;
    this->ui->suspendButton->setEnabled(true);

    this->ui->compareButton->setEnabled(false);
    this->ui->actionCompare->setEnabled(false);

    this->ui->browseButton->setEnabled(false);
    this->ui->actionBrowse->setEnabled(false);

    this->ui->resetButton->setEnabled(false);
    this->ui->actionReset->setEnabled(false);

    this->ui->duplicatesTextEdit->clear();
    this->ui->duplicatesTextEdit->append("FOLDER : " + folderString);
    this->ui->duplicatesTextEdit->append("-------------------------------------------------------------");

    thread = new QThread(this);
    compareThread = new CompareThread();

    compareThread->setParameters(folderString, m_sFilter);

    compareThread->moveToThread(thread);

    connect(thread, SIGNAL(started()), compareThread, SLOT(dirFunction()));
    connect(compareThread, SIGNAL(duplicatesFound(QString,QString,int,int)), this, SLOT(onDuplicatesFound(QString,QString,int,int)));
    connect(compareThread, SIGNAL(compareDone(QList<int>, int)), this, SLOT(onCompareDone(QList<int>, int)));
    connect(compareThread, SIGNAL(progress(int)), this, SLOT(onProgress(int)));
    connect(compareThread, SIGNAL(workFinished()), this, SLOT(updateSuspendButtonState()));
    connect(compareThread, SIGNAL(workFinished()), thread, SLOT(quit()));

    // Automatically delete thread and task object when work is done:
    connect(thread, SIGNAL(finished()), compareThread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}

void Duplicates::onDuplicatesFound(QString sFileToCompare, QString sFileCompared, int i, int j)
{
    this->ui->duplicatesTextEdit->append(sFileToCompare + " @ " + QString::number(i));
    this->ui->duplicatesTextEdit->append(sFileCompared + " @ " + QString::number(j));
    this->ui->duplicatesTextEdit->append("-------------------------------------------------------------");
}

void Duplicates::updateSuspendButtonState()
{
    compareState = false;
    this->ui->suspendButton->setEnabled(false);
}

void Duplicates::onCompareDone(QList<int> iDuplicateIndexes, int iDuplicatesCount)
{
//    qDebug() << iDuplicateIndexes;

    QString sDuplcates(QString::number(iDuplicatesCount));
    QString sMessage;

    if(iDuplicatesCount == 1)
    {
        sMessage = sDuplcates + " Duplicate Found...";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Duplicates"), sMessage + "\nDo you want to delete duplicate?", QMessageBox::Yes|QMessageBox::No);

        if(reply == QMessageBox::Yes)
        {
            for(int n = 0; n < iDuplicatesCount; n++)
            {
                int index = iDuplicateIndexes[n];
                QString sFilePath(folderString + "/" + fileList[index]);

                QFile file(sFilePath);
                file.remove();
            }
        }
    }
    else if(iDuplicatesCount > 1)
    {
        sMessage = sDuplcates + " Duplicates Found...";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Duplicates"), sMessage + "\nDo you want to delete duplicates?", QMessageBox::Yes|QMessageBox::No);

        if(reply == QMessageBox::Yes)
        {
            for(int n = 0; n < iDuplicatesCount; n++)
            {
                int index = iDuplicateIndexes[n];
                QString sFilePath(folderString + "/" + fileList[index]);

                QFile file(sFilePath);
                file.remove();
            }
        }
    }
    else if(iDuplicatesCount == 0)
    {
        sMessage = "No Duplicates Found...";

        QMessageBox msgBox;
        msgBox.setText(sMessage);
        msgBox.exec();
    }

    this->ui->compareButton->setEnabled(true);
    this->ui->actionCompare->setEnabled(true);

    this->ui->resetButton->setEnabled(true);
    this->ui->actionReset->setEnabled(true);
}

void Duplicates::onProgress(int iProgress)
{
    this->ui->progressBar->setValue(iProgress);
}

void Duplicates::on_resetButton_clicked()
{
    this->ui->filterLineEdit->setEnabled(true);
    this->ui->compareButton->setEnabled(false);

    this->ui->browseButton->setEnabled(true);
    this->ui->actionBrowse->setEnabled(true);

    this->ui->compareButton->setEnabled(false);
    this->ui->actionCompare->setEnabled(false);

    this->ui->detailsLabel->setText("Details");

    this->ui->progressBar->setValue(0);

    this->ui->duplicatesTextEdit->clear();

    m_sFilter = ".jpg";
    this->ui->filterLineEdit->setText(m_sFilter);

    folderString.clear();

    m_iDuplicatesCount = 0;
    m_iDuplicateIndexes.clear();
}

void Duplicates::aboutFunction()
{
    QMessageBox::about(this, tr("Duplicates Cleaner"), "Duplicates Cleaner V1.0 - CopyRight © Apps Brothers 2014 - All Rights Reserved.");
}

void Duplicates::on_suspendButton_clicked()
{
    if(compareState == true)
    {
        compareThread->m_bSuspendValue = true;

        this->ui->compareButton->setEnabled(true);
        this->ui->actionCompare->setEnabled(true);

        this->ui->browseButton->setEnabled(true);
        this->ui->actionBrowse->setEnabled(true);

        this->ui->resetButton->setEnabled(true);
        this->ui->actionReset->setEnabled(true);
    }
}
