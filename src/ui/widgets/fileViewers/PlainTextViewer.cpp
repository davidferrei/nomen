#include "PlainTextViewer.h"
#include <QDebug>


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

void PlainTextViewer::fileSelected(const QString& path)
{
    if (!txtEditorWidgetInstance) 
    {
        qWarning() << "[PlainTextViewer]: Cannot Display File, Editor Instance is NULL";
        return;
    }   
    
    txtEditorWidgetInstance->setPlainText(path);
}
