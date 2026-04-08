#ifndef TSKFILEREADER_H
#define TSKFILEREADER_H

#include <tsk/libtsk.h>

#include <unordered_map>

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDebug>

class tskFileReader : public QObject
{
    Q_OBJECT
public:
    explicit tskFileReader(QObject* parent = nullptr);
    ~tskFileReader();

    QByteArray readFileContentFromImage(const QString& diskImagePath, const QString& fsPath, qint64 offset, qint64 length, qint64& fileSize, QString& errorStringOutput);

};

#endif // TSKFILEREADER_H
