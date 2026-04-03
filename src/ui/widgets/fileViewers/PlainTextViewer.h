#ifndef PLAINTEXTVIEWER_H
#define PLAINTEXTVIEWER_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QSizePolicy>

class PlainTextViewer : public QWidget
{
    Q_OBJECT
public:
    explicit PlainTextViewer(QPlainTextEdit* existingEditor = nullptr, QWidget* parent = nullptr);

public slots:
    void fileSelected(const QString& path);

private:
    QPlainTextEdit* txtEditorWidgetInstance = nullptr;
};

#endif // PLAINTEXTVIEWER_H
