#ifndef CHROMEPASS_H
#define CHROMEPASS_H

#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QDebug>

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
    QString path;
};

#endif // CHROMEPASS_H
