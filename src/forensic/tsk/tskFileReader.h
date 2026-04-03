#ifndef TSKFILEREADER_H
#define TSKFILEREADER_H

#include <QObject>

class tskFileReader : public QObject
{
    Q_OBJECT
public:
    explicit tskFileReader(QObject* parent = nullptr);
    ~tskFileReader();

};

#endif // TSKFILEREADER_H
