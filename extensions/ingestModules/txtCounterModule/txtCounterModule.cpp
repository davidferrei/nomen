#include "txtCounterModule.h"

void TxtCounterModule::init()
{
	filesCount = 0;
	dirsCount = 0;
	txtFilesCount = 0;

	// Create Folder
	if (ingestResultDB)
	{
        resultFolderId = ingestResultDB->createFolderItem(getModuleName());
		filesFolderId = ingestResultDB->createFolderItem(resultFolderId, QStringLiteral("Files"));
		if (filesFolderId < 0) filesFolderId = resultFolderId;
	}
}

void TxtCounterModule::processFile(const FileContext &fileContext)
{
	if (fileContext.hasMetadata && fileContext.tsk_metadata && fileContext.tsk_metadata->type == TSK_FS_META_TYPE_DIR) 
	{
		++dirsCount;
		return;
	}

	++filesCount;

	const bool isTxt = fileContext.filename.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive);
	if (isTxt) 
	{
		++txtFilesCount;

		if (fileContext.hasMetadata && fileContext.tsk_metadata && fileContext.tsk_metadata->type == TSK_FS_META_TYPE_REG) 
		{
			QByteArray contentByteArray;
			contentByteArray.resize(10);
			const ssize_t n = fileContext.readFileContent(0, contentByteArray.data(), 10);
			if (n > 0) 
			{
				if (n < 10) contentByteArray.resize(static_cast<int>(n));

				QByteArray hexByteArray = contentByteArray.toHex(' ').toUpper();
				QString asciiString;
				asciiString.reserve(contentByteArray.size());
				for (char ch : contentByteArray) 
				{
					//Convert to ASCII Safe
					unsigned char uch = static_cast<unsigned char>(ch);
					asciiString.append(std::isprint(uch) ? QChar(static_cast<char>(uch)) : QChar('.'));
				}
				// qInfo() << "[TxtCounter] First10Bytes for" << fileContext.filePath << ": HEX=" << hexByteArray.constData() << ", ASCII=" << asciiString;
			}
		}
	}
}

void TxtCounterModule::finishWalk()
{
	qInfo() << "[TxtCounter] Results: Files:" << filesCount << "Directories:" << dirsCount << "TXT Files:" << txtFilesCount;
	
	if (ingestResultDB)
	{
		ingestResultDB->createTextItem(resultFolderId, QStringLiteral("Files"), QString::number(filesCount));
		ingestResultDB->createTextItem(resultFolderId, QStringLiteral("Directories"), QString::number(dirsCount));
		ingestResultDB->createTextItem(resultFolderId, QStringLiteral("TXT Files"), QString::number(txtFilesCount));

		qint64 idPasswd = ingestResultDB->createFileItem(1, 2, 0, QStringLiteral("2:0:/etc/passwd"), QStringLiteral("This is a note for the passwd file record"), filesFolderId);
		if (idPasswd < 0) qWarning() << "[TxtCounter] Failed to Create File Record for /etc/passwd";
		qint64 idShadow = ingestResultDB->createFileItem(1, 2, 0, QStringLiteral("2:0:/etc/shadow"), QString(), filesFolderId);
		if (idShadow < 0) qWarning() << "[TxtCounter] Failed to Create File Record for /etc/shadow";

		idPasswd = ingestResultDB->createFileItem(1, 2, 0, QStringLiteral("2:0:/etc/passwd"), QString(), filesFolderId);
		if (idPasswd < 0) qWarning() << "[TxtCounter] Failed to Create File Record for /etc/passwd";
		idShadow = ingestResultDB->createFileItem(1, 2, 0, QStringLiteral("2:0:/etc/shadow"), QString(), filesFolderId);
		if (idShadow < 0) qWarning() << "[TxtCounter] Failed to Create File Record for /etc/shadow";

		idPasswd = ingestResultDB->createFileItem(1, 2, 0, QStringLiteral("2:0:/etc/passwd"), QString(), filesFolderId);
		if (idPasswd < 0) qWarning() << "[TxtCounter] Failed to Create File Record for /etc/passwd";
		idShadow = ingestResultDB->createFileItem(1, 2, 0, QStringLiteral("2:0:/etc/shadow"), QStringLiteral("This is a note for the shadow file record"), filesFolderId);
		if (idShadow < 0) qWarning() << "[TxtCounter] Failed to Create File Record for /etc/shadow";

		// Load Test
		// qInfo() << "[TxtCounter] Creating 500 Folders...";
		// for (int i = 1; i <= 500; ++i)
		// {
		// 	QString name = QStringLiteral("PerfFolder_%1").arg(i);
		// 	qint64 newId = ingestResultDB->createFolderItem(resultFolderId, name);
		// 	if (newId < 0)
		// 	{
		// 		qWarning() << "[TxtCounter] Failed to Create Folder:" << name << "at Iteration" << i;
		// 		break;
		// 	}

		// 	for (int t = 1; t <= 500; ++t)
		// 	{
		// 		if (ingestResultDB->createTextItem(newId, QString(), QString::number(t)) < 0)
		// 		{
		// 			qWarning() << "[TxtCounter] Failed to Create Text Item" << t << "in Folder" << name;
		// 			break;
		// 		}
		// 	}
		// }
	}
}

// 1935.14 ms (10 bytes Content Read)
// 1514.97 ms (No Content Read)

