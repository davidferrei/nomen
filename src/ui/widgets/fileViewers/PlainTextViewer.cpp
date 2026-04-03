#include "PlainTextViewer.h"


PlainTextViewer::PlainTextViewer(QPlainTextEdit* existingEditor, QWidget* parent): QWidget(parent), txtEditorWidgetInstance(existingEditor)
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

void PlainTextViewer::fileSelected(const QString& diskImagePath, const QString& fsPath)
{

    if (!txtEditorWidgetInstance) 
    {
        qWarning() << "[PlainTextViewer]: Cannot Display File, Editor Instance is NULL";
        return;
    }

    QString errorString;
    tskFileReader reader;
    QByteArray rawBArray = reader.readFileContentFromImage(diskImagePath, fsPath, errorString);

    if (!errorString.isEmpty())
    {
        qWarning() << "[PlainTextViewer]: Error Reading File:" << errorString;
        txtEditorWidgetInstance->setPlainText(QStringLiteral("Error: %1").arg(errorString));
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
        if (!utf8.contains(QChar::ReplacementCharacter))
        {
            text = utf8;
        }
        else
        {
            text = QString::fromLatin1(rawBArray);
        }
    }

    txtEditorWidgetInstance->setPlainText(text);
}
