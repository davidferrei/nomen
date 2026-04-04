#ifndef TSKFILEREADER_H
#define TSKFILEREADER_H

#include <tsk/libtsk.h>

#include <unordered_map>

#include <QObject>
#include <QByteArray>
#include <QString>

class tskFileReader : public QObject
{
    Q_OBJECT
public:
    explicit tskFileReader(QObject* parent = nullptr);
    ~tskFileReader();

    QByteArray readFileContentFromImage(const QString& diskImagePath, const QString& fsPath, QString& errorStringOutput);

};

#endif // TSKFILEREADER_H
