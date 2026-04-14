#ifndef PLAINTEXTVIEWER_H
#define PLAINTEXTVIEWER_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "core/forensic/tsk/tskFileReader.h"

class PlainTextViewer : public QWidget
{
    Q_OBJECT
public:
    explicit PlainTextViewer(QPlainTextEdit* existingEditor = nullptr, QWidget* parent = nullptr);

public slots:
    void fileSelected(const QString& diskImagePath, const QString& fsPath);
    void prevPage();
    void nextPage();

signals:
    void prevPageEnabled(bool enabled);
    void nextPageEnabled(bool enabled);
    void pageLabelChanged(const QString& text);

private slots:
    void loadCurrentPage();

private:
    static constexpr qint64 PAGE_SIZE = 512 * 1024; // 512 KB

    QPlainTextEdit* txtEditorWidgetInstance = nullptr;

    QString selectedDiskImagePath;
    QString selectedFsPath;
    qint64  selectedFileSize    = 0;
    qint64  currentPageNumber = 0;

    qint64 totalPages() const;
    void   updateNavBar();
};

#endif // PLAINTEXTVIEWER_H
