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
	}
}

// 1935.14 ms (10 bytes Content Read)
// 1514.97 ms (No Content Read)

