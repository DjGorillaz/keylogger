#ifndef CHROMEPASS_H
#define CHROMEPASS_H

#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QTimer>

#include <windows.h>

class PassReader : public QObject
{
    Q_OBJECT
public:
    explicit PassReader(QObject* parent = 0, const QString& defaultPath = QDir::currentPath());
    ~PassReader();

public slots:
    bool readPass();

signals:
    void passSaved(QString filePath);

private:
    QTimer* timer;
    QString path;
    QSqlDatabase* db;
};

#endif // CHROMEPASS_H
