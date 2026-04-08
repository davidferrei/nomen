#include "PlainTextViewer.h"


PlainTextViewer::PlainTextViewer(QPlainTextEdit* existingEditor, QWidget* parent) : QWidget(parent), txtEditorWidgetInstance(existingEditor)
{
    if (!txtEditorWidgetInstance)
    {
        qWarning() << "[PlainTextViewer]: Editor instance is NULL";
        Q_ASSERT(txtEditorWidgetInstance);
        return;
    }

}

//################################################################################################
// Signals

qint64 PlainTextViewer::totalPages() const
{
    if (selectedFileSize == 0) return 1;

    return (selectedFileSize + PAGE_SIZE - 1) / PAGE_SIZE;
}

void PlainTextViewer::updateNavBar()
{
    const qint64 total = totalPages();
    emit prevPageEnabled(currentPageNumber > 0);
    emit nextPageEnabled(currentPageNumber < total - 1);
    emit pageLabelChanged(QString("Page %1 / %2").arg(currentPageNumber + 1).arg(total));
}

void PlainTextViewer::prevPage()
{
    if (currentPageNumber > 0)
    {
        --currentPageNumber;
        loadCurrentPage();
    }
}

void PlainTextViewer::nextPage()
{
    if (currentPageNumber < totalPages() - 1)
    {
        ++currentPageNumber;
        loadCurrentPage();
    }
}

void PlainTextViewer::fileSelected(const QString& diskImagePath, const QString& fsPath)
{
    if (!txtEditorWidgetInstance)
    {
        qWarning() << "[PlainTextViewer]: Cannot Display File, Editor Instance is NULL";
        return;
    }

    selectedDiskImagePath = diskImagePath;
    selectedFsPath = fsPath;
    currentPageNumber   = 0;
    selectedFileSize = 0;

    loadCurrentPage();
}

void PlainTextViewer::loadCurrentPage()
{
    if (!txtEditorWidgetInstance) return;

    const qint64 offset = currentPageNumber * PAGE_SIZE;

    QString errorString;
    tskFileReader reader;
    QByteArray rawBArray = reader.readFileContentFromImage(
        selectedDiskImagePath, selectedFsPath,
        offset, PAGE_SIZE,
        selectedFileSize, errorString);

    if (!errorString.isEmpty())
    {
        qWarning() << "[PlainTextViewer]: Error Reading File:" << errorString;
        txtEditorWidgetInstance->setPlainText(QStringLiteral("Error: %1").arg(errorString));
        updateNavBar();
        return;
    }

    QString text;
    if (rawBArray.isEmpty())
    {
        text = QStringLiteral("<Empty File>");
    }
    else
    {
        QString utf8 = QString::fromUtf8(rawBArray);
        text = utf8.contains(QChar::ReplacementCharacter) ? QString::fromLatin1(rawBArray) : utf8;
    }

    txtEditorWidgetInstance->setPlainText(text);
    txtEditorWidgetInstance->moveCursor(QTextCursor::Start);

    updateNavBar();
}
