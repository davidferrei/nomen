#include "tskFileReader.h"

tskFileReader::tskFileReader(QObject* parent) : QObject(parent) {}

tskFileReader::~tskFileReader() {}

QByteArray tskFileReader::readFileContentFromImage(const QString& diskImagePath, const QString& fsPath, QString& errorStringOutput)
{

	QByteArray outputBArray;
	outputBArray.append(diskImagePath.toUtf8());
	outputBArray.append('\n');
	outputBArray.append(fsPath.toUtf8());

	// qDebug() << "[TSKFileReader] Returning Payload:" << outputBArray;

	return outputBArray;
}
