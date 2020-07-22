#ifndef DUPLICATES_H
#define DUPLICATES_H

#include <QMainWindow>
#include <QThread>
#include "comparethread.h"

namespace Ui {
class Duplicates;
}

class Duplicates : public QMainWindow
{
    Q_OBJECT

public:
    explicit Duplicates(QWidget *parent = 0);
    ~Duplicates();

    QString folderString;
    QStringList fileList;
    int fileCount;

    QString m_sFilter;

    int m_iDuplicatesCount;
    QList<int> m_iDuplicateIndexes;

    QThread *thread;
    CompareThread *compareThread;

    bool compareState;

private slots:
    void on_browseButton_clicked();
    void dirFunction();
    void on_filterLineEdit_textChanged(const QString &arg1);
    void on_compareButton_clicked();

    void onDuplicatesFound(QString sFileToCompare, QString sFileCompared, int i, int j);
    void updateSuspendButtonState();
    void onCompareDone(QList<int> iDuplicateIndexes, int iDuplicatesCount);
    void onProgress(int iProgress);
    void on_resetButton_clicked();
    void aboutFunction();

    void on_suspendButton_clicked();

private:
    Ui::Duplicates *ui;
};

#endif // DUPLICATES_H
