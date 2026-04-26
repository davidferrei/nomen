#ifndef INGESTMODULEINTERFACE_H
#define INGESTMODULEINTERFACE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonObject>

class AbstractIngestModule : public QObject
{
    Q_OBJECT
public:
    explicit AbstractIngestModule(QObject *parent = nullptr) : QObject(parent) {}
    explicit AbstractIngestModule(const QJsonObject &metadata, QObject *parent = nullptr) : QObject(parent), m_metadata(metadata) {}
    ~AbstractIngestModule() override = default;

    virtual void init() = 0;


    virtual QString id() const
    {
        const QString id = m_metadata.value("id").toString();

        if (id.isEmpty())
            qWarning() << "[AbstractIngestModule] Missing Metadata Key: id";
        return id;
    }

    virtual QString name() const
    {
        const QString name = m_metadata.value("name").toString();
        if (name.isEmpty())
            qWarning() << "[AbstractIngestModule] Missing Metadata Key: name";
        return name;
    }

    virtual QString version() const
    {
        const QString version = m_metadata.value("version").toString();
        if (version.isEmpty())
            qWarning() << "[AbstractIngestModule] Missing Metadata Key: version";
        return version;
    }

    virtual QString getModuleName() const { return name(); }

    virtual void process() = 0;

    

protected:
    QJsonObject m_metadata;

// signals:
//     void showMessage(const QString &message, int timeout = 8000);
//     void finished();
};

class IngestModuleInterface
{
public:
    virtual ~IngestModuleInterface() = default;

    virtual AbstractIngestModule *module(const QJsonObject &metadata) const = 0;
    virtual QString moduleName() const = 0;
};

Q_DECLARE_INTERFACE(IngestModuleInterface, "org.nomen.IngestModuleInterface")

#endif // INGESTMODULEINTERFACE_H
