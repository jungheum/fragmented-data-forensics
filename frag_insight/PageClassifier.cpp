/**
 *	@file	PageClassifier.cpp
 *	@date	2012/04/07
 *	@author	Jungheum Park

 *	@brief	Fragmented Page Classifier
 */

/**
	@mainpage	Fragmented Page Classifier

	@section	MODIFYINFO	History

		- Jungheum Park / 2011.06.25
			- Initial Version
		- Jungheum Park / 2012.04.07
			- Enhance classification algorithms
 */



#include "StdAfx.h"
#include "global_defines.h"
#include "PageClassifier.h"
#include "FileSignatures/FileSignatures.h"
#include "RandomTest/RTest.h"
#include "SQLiteRecover/SQLiteDataStructure.h"
#include "SQLiteRecover/RecordPageExtractor.h"


CPageClassifier::CPageClassifier()
{
	pageSizeBase = 4096;
	htmlCounts = 0;
	xmlCounts = 0;
	pdfCounts = 0;
	oleCounts = 0;
	asciiCounts = 0;
	utf8Counts = 0;
}

CPageClassifier::CPageClassifier(UINT base)
{
	pageSizeBase = base;
	htmlCounts = 0;
	xmlCounts = 0;
	pdfCounts = 0;
	oleCounts = 0;
	asciiCounts = 0;
	utf8Counts = 0;
}

CPageClassifier::~CPageClassifier(void)
{
}

UINT CPageClassifier::RunPageClassifier(CString srcPath1, CString srtPath2, CString dstPath)
{
	CString strPath;

#ifdef ON_CLASSIFIER

	// 1. Hash-based Classification
	if(PathFileExists(srcPath1))
	{
		printf("\n [PROCESS] : Hash-based Classification.");
		HashBasedClassifier(srcPath1, dstPath);
		printf(" [COMPLETE]\n");
	}

	// 2. Meta Page Classification
	strPath.Format(_T("%s\\%s\\ClassifiedPagesA.bin"), dstPath, STR_CLASS_HASH);

	if(PathFileExists(strPath))
	{
		printf("\n [PROCESS] : Meta Page Classification.");
		MetaPageClassifier(strPath, dstPath);
		printf(" [COMPLETE]\n");
	}
	
	// 3. Statistical Classification
	strPath.Format(_T("%s\\%s\\ClassifiedPagesB.bin"), dstPath, STR_CLASS_META);

	if(PathFileExists(strPath))
	{
		printf("\n [PROCESS] : Statistical Classification.");
		StatisticalClassifier(strPath, srtPath2, dstPath);
		printf(" [COMPLETE]\n");
	}

	// 4. File Format Classification
	strPath.Format(_T("%s\\%s\\NonRandomPages.bin"), dstPath, STR_CLASS_RND);

	if(PathFileExists(strPath))
	{
		printf("\n [PROCESS] : File Format Classification.");
		FileFormatClassifier(strPath, srtPath2, dstPath);
		printf(" [COMPLETE]\n");
	}

#endif

	return TRUE;
}

UINT CPageClassifier::MetaPageClassifier(CString srcPath, CString dstPath)
{
	// Make output directory
	dstPath.AppendFormat(_T("\\%s\\"), STR_CLASS_META);
	SHCreateDirectoryEx(NULL, dstPath, NULL);
	

	CFile srcFile;
	if(srcFile.Open(srcPath, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}


	// classify meta page
	PBYTE buffer = (PBYTE)malloc(pageSizeBase);
	PBYTE yaffsMetaBuffer = (PBYTE)malloc(BUF_SIZE);
	PBYTE ext4MetaBuffer = (PBYTE)malloc(BUF_SIZE);
	PBYTE othersBuffer = (PBYTE)malloc(BUF_SIZE);

	ZeroMemory(buffer, pageSizeBase);
	ZeroMemory(yaffsMetaBuffer, BUF_SIZE);
	ZeroMemory(ext4MetaBuffer, BUF_SIZE);
	ZeroMemory(othersBuffer, BUF_SIZE);

	UINT totalPageCount = 0;
	UINT metaPageCount = 0;
	UINT yaffsMetaSize = 0, ext4MetaSize = 0, othersSize = 0;

	UINT   readSize = 0;
	UINT64 totalSize = 0;
	UINT64 srcFileSize = srcFile.GetLength();

	while(srcFileSize > totalSize)
	{
		if(srcFileSize-totalSize < pageSizeBase)
			readSize = (UINT)(srcFileSize-totalSize);
		else
			readSize = pageSizeBase;

		if(srcFile.Read(buffer, readSize) != readSize)
		{
			break;
		}

		// verify filesystem meta page
		if(IsYaffsMetaPage(buffer, readSize) == TRUE)
		{
			if (yaffsMetaSize+readSize > BUF_SIZE)
			{
				CString dstPathBak = dstPath;
				dstPathBak.AppendFormat( _T("\\MetaPages_of_%s.bin"), STR_FS_YAFFS);

				utils.ExportToFile(dstPathBak, yaffsMetaBuffer, yaffsMetaSize);

				ZeroMemory(yaffsMetaBuffer, BUF_SIZE);
				yaffsMetaSize = 0;
			}

			memcpy(yaffsMetaBuffer+yaffsMetaSize, buffer, readSize);
			yaffsMetaSize += readSize;

			metaPageCount++;
		}
		else if(IsExt4MetaPage(buffer, readSize) == TRUE)
		{
			if (ext4MetaSize+readSize > BUF_SIZE)
			{
				CString dstPathBak = dstPath;
				dstPathBak.AppendFormat( _T("\\MetaPages_of_%s.bin"), STR_FS_EXT4);

				utils.ExportToFile(dstPathBak, ext4MetaBuffer, ext4MetaSize);

				ZeroMemory(ext4MetaBuffer, BUF_SIZE);
				ext4MetaSize = 0;
			}

			memcpy(ext4MetaBuffer+ext4MetaSize, buffer, readSize);
			ext4MetaSize += readSize;

			metaPageCount++;
		}
		else
		{
			if (othersSize+readSize > BUF_SIZE)
			{
				CString dstPathBak = dstPath;
				dstPathBak.AppendFormat( _T("\\ClassifiedPagesB.bin"), STR_FS_EXT4);

				utils.ExportToFile(dstPathBak, othersBuffer, othersSize);

				ZeroMemory(othersBuffer, BUF_SIZE);
				othersSize = 0;
			}

			memcpy(othersBuffer+othersSize, buffer, readSize);
			othersSize += readSize;
		}

		ZeroMemory(buffer, pageSizeBase);

		totalSize += readSize;
		totalPageCount++;
	}

	if (yaffsMetaSize > 0)
	{
		CString dstPathBak = dstPath;
		dstPathBak.AppendFormat( _T("\\MetaPages_of_%s.bin"), STR_FS_YAFFS);

		utils.ExportToFile(dstPathBak, yaffsMetaBuffer, yaffsMetaSize);
	}

	if (ext4MetaSize > 0)
	{
		CString dstPathBak = dstPath;
		dstPathBak.AppendFormat( _T("\\MetaPages_of_%s.bin"), STR_FS_EXT4);

		utils.ExportToFile(dstPathBak, ext4MetaBuffer, ext4MetaSize);
	}

	if (othersSize > 0)
	{
		CString dstPathBak = dstPath;
		dstPathBak.AppendFormat( _T("\\ClassifiedPagesB.bin"), STR_FS_EXT4);

		utils.ExportToFile(dstPathBak, othersBuffer, othersSize);
	}

	//printf("\n Total number of pages: %d\n", totalPageCount);
	//printf("\n Total number of meta pages: %d\n", metaPageCount);

	safe_free(buffer);
	safe_free(yaffsMetaBuffer);
	safe_free(ext4MetaBuffer);
	safe_free(othersBuffer);
	srcFile.Close();
	return TRUE;
}

BOOL CPageClassifier::IsYaffsMetaPage(PBYTE buffer, UINT pageSize)
{
	// 128 ~ 144 = 0x00
	BYTE byXor = 0;

	for(int i = 0; i < 16; i++)
	{
		byXor = buffer[128+i] ^ byXor;
	}

	if(byXor != 0x00)
		return FALSE;

	// 1024 ~ 2048 = 0xFF
	byXor = 0;

	for(UINT i = 0; i < pageSize/2; i++)
	{
		if(buffer[pageSize/2+i] != 0xFF)
			return FALSE;
	}

	return TRUE;
}

BOOL CPageClassifier::IsExt4MetaPage(PBYTE buffer, UINT pageSize)
{
	// Directory Entry of EXT4
	const BYTE sig_EXT4_DE1[8] = {0x0C, 0x00, 0x01, 0x02, 0x2E, 0x00, 0x00, 0x00};
	const BYTE sig_EXT4_DE2[6] = {0x02, 0x02, 0x2E, 0x2E, 0x00, 0x00};

	if( !memcmp(sig_EXT4_DE1, buffer+4, sizeof(sig_EXT4_DE1)) &&
		!memcmp(sig_EXT4_DE2, buffer+18, sizeof(sig_EXT4_DE2)) )
	{
		return TRUE;
	}

	return FALSE;
}

UINT CPageClassifier::HashBasedClassifier(CString srcPath, CString dstPath)
{
	// Make output directory
	dstPath.AppendFormat(_T("\\%s\\"), STR_CLASS_HASH);
	SHCreateDirectoryEx(NULL, dstPath, NULL);
	dstPath += _T("\\ClassifiedPagesA.bin");

	CFile srcFile;
	if(srcFile.Open(srcPath, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}


	ATL::CAtlMap<CAtlStringA, CStringA> hashMap;

	CStringA srtHash;
	CStringA strValue;

	PBYTE buffer = (PBYTE)malloc(pageSizeBase);
	PBYTE outBuffer = (PBYTE)malloc(BUF_SIZE);

	ZeroMemory(buffer, pageSizeBase);
	ZeroMemory(outBuffer, BUF_SIZE);

	UINT totalPageCount = 0;
	UINT duplicatePageCount = 0;
	UINT outSize = 0;

	UINT   readSize = 0;
	UINT64 totalSize = 0;
	UINT64 srcFileSize = srcFile.GetLength();

	while(srcFileSize > totalSize)
	{
		if(srcFileSize-totalSize < pageSizeBase)
			readSize = (UINT)(srcFileSize-totalSize);
		else
			readSize = pageSizeBase;

		if(srcFile.Read(buffer, readSize) != readSize)
		{
			break;
		}

		srtHash = utils.DoSHA1(buffer, pageSizeBase);

		// Insert hash value into AtlMap
		if(hashMap.Lookup(srtHash, strValue))
		{// duplicate page does exist
			duplicatePageCount++;
		}
		else
		{
			hashMap.SetAt(srtHash, srtHash);

			// Write the page to dstFile
			if (outSize+readSize > BUF_SIZE)
			{
				utils.ExportToFile(dstPath, outBuffer, outSize);

				ZeroMemory(outBuffer, BUF_SIZE);
				outSize = 0;
			}

			memcpy(outBuffer+outSize, buffer, readSize);
			outSize += readSize;
		}

		ZeroMemory(buffer, pageSizeBase);

		totalSize += readSize;
		totalPageCount++;
	}

	if (outSize > 0)
	{
		utils.ExportToFile(dstPath, outBuffer, outSize);
	}

	//printf("\n Total number of pages: %d\n", totalPageCount);
	//printf("\n Total number of duplicate pages: %d\n", duplicatePageCount);

	safe_free(buffer);
	safe_free(outBuffer);
	srcFile.Close();
	return TRUE;
}

UINT CPageClassifier::StatisticalClassifier(CString srcPath1, CString srtPath2, CString dstPath)
{
	// Make output directory
	SHCreateDirectoryEx(NULL, dstPath, NULL);

	CFile srcFile;
	if(srcFile.Open(srcPath1, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}

	BYTE *buffer = (PBYTE)malloc(pageSizeBase);
	BYTE *randomBuffer = (PBYTE)malloc(BUF_SIZE);
	BYTE *nonRandomBuffer = (PBYTE)malloc(BUF_SIZE);

	ZeroMemory(buffer, pageSizeBase);
	ZeroMemory(randomBuffer, BUF_SIZE);
	ZeroMemory(nonRandomBuffer, BUF_SIZE);

	UINT totalPageCount = 0;
	UINT classifiedPageCount = 0, classifiedPageCount2 = 0;
	UINT randomSize = 0, nonRandomSize = 0;
	UINT pageSignature = 0;

	UINT   readSize = 0;
	UINT64 totalSize = 0;
	UINT64 srcFileSize = srcFile.GetLength();

	while(srcFileSize > totalSize)
	{
		if(srcFileSize-totalSize < pageSizeBase)
			readSize = (UINT)(srcFileSize-totalSize);
		else
			readSize = pageSizeBase;

		if(srcFile.Read(buffer, readSize) != readSize)
		{
			break;
		}

		// Verify page signature
		int pageSignature = VerifyPageSignature(buffer, readSize, TRUE);

		if(pageSignature == FORMAT_JPEG_EXIF || pageSignature == FORMAT_JPEG_JFIF 
			|| pageSignature == FORMAT_GIF || pageSignature == FORMAT_PNG
			|| pageSignature == FORMAT_TIFF || pageSignature == FORMAT_AVI || pageSignature == FORMAT_WAV
			|| pageSignature == FORMAT_MP4 || pageSignature == FORMAT_MOV || pageSignature == FORMAT_SWF
			|| pageSignature == FORMAT_MP3 || pageSignature == FORMAT_ZIP || pageSignature == FORMAT_GZIP)
		{
			pageSignature = RANDOM_BLOCK;
			classifiedPageCount++;
		}
		else
		{
			// Randomness Test
			pageSignature = Randomness_Test((PCHAR)buffer, readSize);
			
			if(pageSignature == RANDOM_BLOCK)
				classifiedPageCount++;
			else if(pageSignature == NON_RANDOM_BLOCK)
				classifiedPageCount2++;
		}
		
		if (pageSignature == RANDOM_BLOCK)
		{
			if (randomSize+readSize > BUF_SIZE)
			{
				CString dstPathBak = dstPath;
				dstPathBak.AppendFormat(_T("\\%s\\"), STR_CLASS_RND);
				SHCreateDirectoryEx(NULL, dstPathBak, NULL);
				dstPathBak.AppendFormat( _T("RandomPages.bin"));

				utils.ExportToFile(dstPathBak, randomBuffer, randomSize);

				ZeroMemory(randomBuffer, BUF_SIZE);
				randomSize = 0;
			}

			memcpy(randomBuffer+randomSize, buffer, readSize);
			randomSize += readSize;
		}
		else if (pageSignature == NON_RANDOM_BLOCK)
		{
			if (nonRandomSize+readSize > BUF_SIZE)
			{
				CString dstPathBak = dstPath;
				dstPathBak.AppendFormat(_T("\\%s\\"), STR_CLASS_RND);
				SHCreateDirectoryEx(NULL, dstPathBak, NULL);
				dstPathBak.AppendFormat( _T("NonRandomPages.bin"));

				utils.ExportToFile(dstPathBak, nonRandomBuffer, nonRandomSize);

				ZeroMemory(nonRandomBuffer, BUF_SIZE);
				nonRandomSize = 0;
			}

			memcpy(nonRandomBuffer+nonRandomSize, buffer, readSize);
			nonRandomSize += readSize;
		}
		
		ZeroMemory(buffer, pageSizeBase);

		totalSize += readSize;
		totalPageCount++;
	}

	if (randomSize > 0)
	{
		CString dstPathBak = dstPath;
		dstPathBak.AppendFormat(_T("\\%s\\"), STR_CLASS_RND);
		SHCreateDirectoryEx(NULL, dstPathBak, NULL);
		dstPathBak.AppendFormat( _T("RandomPages.bin"));

		utils.ExportToFile(dstPathBak, randomBuffer, randomSize);
	}

	if (nonRandomSize > 0)
	{
		CString dstPathBak = dstPath;
		dstPathBak.AppendFormat(_T("\\%s\\"), STR_CLASS_RND);
		SHCreateDirectoryEx(NULL, dstPathBak, NULL);
		dstPathBak.AppendFormat( _T("NonRandomPages.bin"));

		utils.ExportToFile(dstPathBak, nonRandomBuffer, nonRandomSize);
	}

// 	printf("\n Total number of pages: %d\n", totalPageCount);
// 	printf("\n Total number of classified random pages: %d, %d\n", classifiedPageCount, classifiedPageCount2);

	safe_free(buffer);
	safe_free(randomBuffer);
	safe_free(nonRandomBuffer);
	srcFile.Close();
	return TRUE;
}

UINT CPageClassifier::FileFormatClassifier(CString srcPath1, CString srtPath2, CString dstPath)
{
	// Make output directory
	SHCreateDirectoryEx(NULL, dstPath, NULL);

	CFile srcFile;
	if(srcFile.Open(srcPath1, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}

		
	PBYTE buffer = (PBYTE)malloc(pageSizeBase);
	PBYTE sqliteBuffer = (PBYTE)malloc(BUF_SIZE);
	PBYTE unknownBuffer = (PBYTE)malloc(BUF_SIZE);

	ZeroMemory(buffer, pageSizeBase);
	ZeroMemory(sqliteBuffer, BUF_SIZE);
	ZeroMemory(unknownBuffer, BUF_SIZE);

	int	 ret = 0;
	UINT totalPageCount = 0;
	UINT sqliteSize = 0, unknownSize = 0;
	UINT classifiedPageCount1 = 0;
	UINT classifiedPageCount2 = 0;
	UINT classifiedPageCount3 = 0;
	UINT classifiedPageCount4 = 0;
	UINT pageSignature = 0;

	UINT   readSize = 0;
	UINT64 totalSize = 0;
	UINT64 srcFileSize = srcFile.GetLength();

	while(srcFileSize > totalSize)
	{
		if(srcFileSize-totalSize < pageSizeBase)
			readSize = (UINT)(srcFileSize-totalSize);
		else
			readSize = pageSizeBase;

		if(srcFile.Read(buffer, readSize) != readSize)
		{
			break;
		}

		// Verify page signature
		pageSignature = VerifyPageSignature(buffer, readSize);

		if(pageSignature >= FORMAT_SQLITE && pageSignature <= FORMAT_SQLITE_II_PAGE)
		{
			pageSignature = ClassifySQLitePage(buffer, readSize, pageSignature, dstPath);
			if(pageSignature != FORMAT_UNKNOWN)
			{
				if (sqliteSize+readSize > BUF_SIZE)
				{
					CString dstPathBak = dstPath;
					dstPathBak.AppendFormat(_T("\\%s\\SQLite\\"), STR_CLASS_FORMAT);
					SHCreateDirectoryEx(NULL, dstPathBak, NULL);
					dstPathBak.AppendFormat( _T("SQLitePages.bin"));

					utils.ExportToFile(dstPathBak, sqliteBuffer, sqliteSize);

					ZeroMemory(sqliteBuffer, BUF_SIZE);
					sqliteSize = 0;
				}

				memcpy(sqliteBuffer+sqliteSize, buffer, readSize);
				sqliteSize += readSize;

				classifiedPageCount2++;
				continue;
			}
		}
		
		if (pageSignature == FORMAT_UNKNOWN)
		{
			if (unknownSize+readSize > BUF_SIZE)
			{
				CString dstPathBak = dstPath;
				dstPathBak.AppendFormat(_T("\\e. UNKNOWN\\"));
				SHCreateDirectoryEx(NULL, dstPathBak, NULL);
				dstPathBak.AppendFormat( _T("UnknownPages.bin"));

				utils.ExportToFile(dstPathBak, unknownBuffer, unknownSize);

				ZeroMemory(unknownBuffer, BUF_SIZE);
				unknownSize = 0;
			}

			memcpy(unknownBuffer+unknownSize, buffer, readSize);
			unknownSize += readSize;
		}
		else
		{
			ClassifyPage(buffer, readSize, pageSignature, dstPath);
			classifiedPageCount3++;
		}
				
		ZeroMemory(buffer, pageSizeBase);

		totalSize += readSize;
		totalPageCount++;
	}

	if (sqliteSize > 0)
	{
		CString dstPathBak = dstPath;
		dstPathBak.AppendFormat(_T("\\%s\\SQLite\\"), STR_CLASS_FORMAT);
		SHCreateDirectoryEx(NULL, dstPathBak, NULL);
		dstPathBak.AppendFormat( _T("SQLitePages.bin"));

		utils.ExportToFile(dstPathBak, sqliteBuffer, sqliteSize);
	}

	if (unknownSize > 0)
	{
		CString dstPathBak = dstPath;
		dstPathBak.AppendFormat(_T("\\e. UNKNOWN\\"));
		SHCreateDirectoryEx(NULL, dstPathBak, NULL);
		dstPathBak.AppendFormat( _T("UnknownPages.bin"));

		utils.ExportToFile(dstPathBak, unknownBuffer, unknownSize);
	}

	//printf("\n Total number of pages: %d\n", totalPageCount);
	//printf("\n Total number of classified SQLite pages: %d, %d, %d, %d\n", classifiedPageCount1, classifiedPageCount2, classifiedPageCount4, classifiedPageCount3);

	safe_free(buffer);
	safe_free(sqliteBuffer);
	safe_free(unknownBuffer);
	srcFile.Close();
	return TRUE;
}

UINT CPageClassifier::ClassifyPage(PBYTE buffer, UINT readSize, UINT pageSignature, CString dstPath)
{
	if(pageSignature > 0x0000 && pageSignature <= 0x0FFF)
	{
		if(pageSignature == FORMAT_HTML)
		{
			dstPath.AppendFormat(_T("\\%s\\HTML\\"), STR_CLASS_FORMAT);
			SHCreateDirectoryEx(NULL, dstPath, NULL);
			
			htmlCounts++;
			dstPath.AppendFormat( _T("%04d.htm"), htmlCounts);
		}
		else if(pageSignature == FORMAT_XML)
		{
			dstPath.AppendFormat(_T("\\%s\\XML\\"), STR_CLASS_FORMAT);
			SHCreateDirectoryEx(NULL, dstPath, NULL);

			xmlCounts++;
			dstPath.AppendFormat( _T("%04d.xml"), xmlCounts);
		}
		else if(pageSignature == FORMAT_PDF)
		{
			dstPath.AppendFormat(_T("\\%s\\PDF\\"), STR_CLASS_FORMAT);
			SHCreateDirectoryEx(NULL, dstPath, NULL);

			pdfCounts++;
			dstPath.AppendFormat( _T("%04d.pdf"), pdfCounts);
		}
		else if(pageSignature == FORMAT_COMPOUND)
		{
			dstPath.AppendFormat(_T("\\%s\\OLE\\"), STR_CLASS_FORMAT);
			SHCreateDirectoryEx(NULL, dstPath, NULL);

			oleCounts++;
			dstPath.AppendFormat( _T("%04d.ole"), oleCounts);
		}
		else if(pageSignature == FORMAT_ASCII)
		{
			dstPath.AppendFormat(_T("\\%s\\TEXT_ASCII\\"), STR_CLASS_FORMAT);
			SHCreateDirectoryEx(NULL, dstPath, NULL);

			asciiCounts++;
			dstPath.AppendFormat( _T("ascii.%04d.txt"), asciiCounts);
		}
		else if(pageSignature == FORMAT_UTF8)
		{
			dstPath.AppendFormat(_T("\\%s\\TEXT_UTF8\\"), STR_CLASS_FORMAT);
			SHCreateDirectoryEx(NULL, dstPath, NULL);

			utf8Counts++;
			dstPath.AppendFormat( _T("utf8.%04d.txt"), utf8Counts);
		}
		else
		{
			dstPath += _T("\\e. UNKNOWN\\");
			SHCreateDirectoryEx(NULL, dstPath, NULL);
			dstPath.AppendFormat( _T("UnknownPages.bin"));
		}
	}

	utils.ExportToFile(dstPath, buffer, readSize);

	return TRUE;
}

BOOL CPageClassifier::ClassifySQLitePage(PBYTE buffer, UINT pageSize, UINT pageSignature, CString dstPath)
{
	if(pageSignature >= FORMAT_SQLITE_TL_PAGE && pageSignature <= FORMAT_SQLITE_II_PAGE)
	{
		USHORT offset = 0;
		USHORT numOfcells = 0;

		// 1~2 : Byte offset into the page of the first free bloc
		offset = big_2bytetoint(&buffer[1]);

		// 3~4 : Number of cells on this page 
		numOfcells = big_2bytetoint(&buffer[3]);

		if(numOfcells > 0x00FF)
		{
			return FORMAT_UNKNOWN;
		}

		// 5~6 : Offset to the first byte of the cell content area.
		offset = big_2bytetoint(&buffer[5]);

		if(offset > pageSizeBase)
		{
			return FORMAT_UNKNOWN;
		}
	}

	if(pageSignature == FORMAT_SQLITE_TI_PAGE || pageSignature == FORMAT_SQLITE_II_PAGE)
	{// Internal page
		USHORT offset = 0;
		USHORT numOfcells = 0;

		// 3~4 : Number of cells on this page 
		numOfcells = big_2bytetoint(&buffer[3]);

		// 12~13 : Byte offset of the first record
		offset = big_2bytetoint(&buffer[12]);

		if(offset > pageSizeBase)
		{
			return FORMAT_UNKNOWN;
		}

		if(pageSignature == FORMAT_SQLITE_II_PAGE)
		{// 0x02 : Index Internal
			UINT pageNumber = 0;

			// 1~4 : Parent Page Number
			pageNumber = big_4bytetoint(&buffer[1]);

			if(pageNumber == 0)
				return FORMAT_UNKNOWN;
		}
		else
		{// 0x05 : Table Internal
			if(numOfcells == 0 || offset == 0)
			{// Free page로 table leaf만 모으기 위함
				return FORMAT_UNKNOWN;
			}
		}
	}

	if(pageSignature == FORMAT_SQLITE_TL_PAGE || pageSignature == FORMAT_SQLITE_IL_PAGE)
	{// Leaf page
		USHORT offset = 0;
		USHORT numOfcells = 0;

		// 3~4 : Number of cells on this page 
		numOfcells = big_2bytetoint(&buffer[3]);

		// 8~9 : Byte offset of the first record
		offset = big_2bytetoint(&buffer[8]);

		if(offset > pageSizeBase)
		{
			return FORMAT_UNKNOWN;
		}

		if(numOfcells == 0 || offset == 0)
		{// Free page로 table leaf만 모으기 위함
			if(pageSignature == FORMAT_SQLITE_TL_PAGE)
			{
				pageSignature = FORMAT_SQLITE_FREE_PAGE;
			}
			else
			{
				return FORMAT_UNKNOWN;
			}
		}
	}

	if(pageSignature == FORMAT_SQLITE)
	{// Main Header page

		// 16~17 : page size
		USHORT pageSize = 0;

		pageSize = big_2bytetoint(&buffer[16]);

		if (pageSize != 1 && pageSize%512 != 0)
		{
			return FORMAT_UNKNOWN;
		}
	}

	return pageSignature;
}

UINT CPageClassifier::VerifyPageSignature(PBYTE pvBuffer, UINT bufSize, BOOL bOpt)
{
	//---------------------------
	// archive
	if(!memcmp(ZIP_sigHEADER, pvBuffer, sizeof(ZIP_sigHEADER))) 
	{
		return FORMAT_ZIP;
	}

	if(!memcmp(GZIP_sigHEADER, pvBuffer, sizeof(GZIP_sigHEADER))) 
	{
		return FORMAT_GZIP;
	}

	//---------------------------
	// graphic
	if(!memcmp(JPEG_EXIF_sigHEADER, pvBuffer, sizeof(JPEG_EXIF_sigHEADER)))
	{
		return FORMAT_JPEG_EXIF;
	}

	if(!memcmp(JPEG_JFIF_sigHEADER, pvBuffer, sizeof(JPEG_JFIF_sigHEADER))) 
	{
		return FORMAT_JPEG_JFIF;
	}

	if( !memcmp(GIF_sigHEADER1, pvBuffer, sizeof(GIF_sigHEADER1)) || 
		!memcmp(GIF_sigHEADER2, pvBuffer, sizeof(GIF_sigHEADER2))) 
	{
		return FORMAT_GIF;
	}

	if(!memcmp(PNG_sigHEADER, pvBuffer, sizeof(PNG_sigHEADER)))
	{
		return FORMAT_PNG;
	}

	if(!memcmp(BMP_sigHEADER, pvBuffer, sizeof(BMP_sigHEADER)))
	{
		return FORMAT_BMP;
	}

	if (!memcmp(TIFF_sigHEADER1, pvBuffer, sizeof(TIFF_sigHEADER1)) ||
		!memcmp(TIFF_sigHEADER2, pvBuffer, sizeof(TIFF_sigHEADER2))) 
	{
		return FORMAT_TIFF;
	}

	//---------------------------
	// multimedia
	if(!memcmp(AVI_sigHEADER, pvBuffer, sizeof(AVI_sigHEADER)))
	{
		return FORMAT_AVI;
	}

	if(!memcmp(WAV_sigHEADER, pvBuffer, sizeof(WAV_sigHEADER)))
	{
		return FORMAT_WAV;
	}

	if(!memcmp(MP4_sigHEADER1, pvBuffer+4, sizeof(MP4_sigHEADER1)))
	{
		return FORMAT_MP4;
	}

	if (!memcmp(MOV_sigHEADER1, pvBuffer+4, sizeof(MOV_sigHEADER1)) ||
		!memcmp(MOV_sigHEADER2, pvBuffer+4, sizeof(MOV_sigHEADER2)) ||
		!memcmp(MOV_sigHEADER3, pvBuffer+4, sizeof(MOV_sigHEADER3)) ||
		!memcmp(MOV_sigHEADER4, pvBuffer+4, sizeof(MOV_sigHEADER4))) 
	{
		return FORMAT_MOV;
	}

	if (!memcmp(SWF_sigHEADER1, pvBuffer, sizeof(SWF_sigHEADER1)) ||
		!memcmp(SWF_sigHEADER2, pvBuffer, sizeof(SWF_sigHEADER2)) ||
		!memcmp(SWF_sigHEADER3, pvBuffer, sizeof(SWF_sigHEADER3))) 
	{
		return FORMAT_SWF;
	}

	if(!memcmp(MP3_sigHEADER, pvBuffer, sizeof(MP3_sigHEADER)))
	{
		return FORMAT_MP3;
	}
	
	if (bOpt == TRUE)
		return FORMAT_UNKNOWN;

	//---------------------------
	// document
	if(!memcmp(PDF_sigHEADER, pvBuffer, sizeof(PDF_sigHEADER))) 
	{
		return FORMAT_PDF;
	}

	if(!memcmp(COMPOUND_sigHEADER, pvBuffer, sizeof(COMPOUND_sigHEADER))) 
	{
		return FORMAT_COMPOUND;
	}

	for(int idx = 0; idx < 64; idx++)
	{
		if (!memcmp(HTML_sigHEADER1, pvBuffer+idx, strlen(HTML_sigHEADER1)) ||
			!memcmp(HTML_sigHEADER2, pvBuffer+idx, strlen(HTML_sigHEADER2)) ||
			!memcmp(HTML_sigHEADER3, pvBuffer+idx, strlen(HTML_sigHEADER3)) ||
			!memcmp(HTML_sigHEADER4, pvBuffer+idx, strlen(HTML_sigHEADER4))) 
		{
			return FORMAT_HTML;
		}

		if (!memcmp(XML_sigHEADER1, pvBuffer+idx, strlen(XML_sigHEADER1)) ||
			!memcmp(XML_sigHEADER2, pvBuffer+idx, strlen(XML_sigHEADER2))) 
		{
			return FORMAT_XML;
		}
	}

	//---------------------------
	// SQLite
	if(!memcmp(SQLITE_sigHeader, pvBuffer, sizeof(SQLITE_sigHeader))) 
	{
		return FORMAT_SQLITE;
	}

	const BYTE	SQLITE_II_PAGE_sigHEADER[1] = {0x02}; // "SQLite Index B-Tree Internal Page"
	if(!memcmp(SQLITE_II_PAGE_sigHEADER, pvBuffer, sizeof(SQLITE_II_PAGE_sigHEADER))) 
	{
		return FORMAT_SQLITE_II_PAGE;
	}

	const BYTE	SQLITE_IL_PAGE_sigHEADER[1] = {0x0A}; // "SQLite Index B-Tree Leaf Page"
	if(!memcmp(SQLITE_IL_PAGE_sigHEADER, pvBuffer, sizeof(SQLITE_IL_PAGE_sigHEADER))) 
	{
		return FORMAT_SQLITE_IL_PAGE;
	}

	const BYTE	SQLITE_TI_PAGE_sigHEADER[1] = {0x05}; // "SQLite Table B-Tree Internal Page"
	if(!memcmp(SQLITE_TI_PAGE_sigHEADER, pvBuffer, sizeof(SQLITE_TI_PAGE_sigHEADER))) 
	{
		return FORMAT_SQLITE_TI_PAGE;
	}

	const BYTE	SQLITE_TL_PAGE_sigHEADER[1] = {0x0D}; // "SQLite Table B-Tree Leaf Page"
	if(!memcmp(SQLITE_TL_PAGE_sigHEADER, pvBuffer, sizeof(SQLITE_TL_PAGE_sigHEADER))) 
	{
		return FORMAT_SQLITE_TL_PAGE;
	}

	//---------------------------
	// TEXT : ASCII only, UTF8...
	if(1)
	{
		BOOL isUTF8 = TRUE, isASCII = TRUE;

		for(UINT i = 3 ; i < bufSize /* == 512 */; )
		{
			if((utils.bigEn_3bytetoint(pvBuffer+i) >= LowerBound_UTF8_kor) && (utils.bigEn_3bytetoint(pvBuffer+i) <= UpperBound_UTF8_kor))
			{
				i += 3;
				continue;
			}
			else if((*(pvBuffer+i) >= LowerBound_ASCII && *(pvBuffer+i) <= UpperBound_ASCII) || 
					*(pvBuffer+i) == 0x0d ||
					*(pvBuffer+i) == 0x0a ||
					*(pvBuffer+i) == 0x09 )
			{
				i++;
				continue;
			}
			else
			{
				isUTF8 = FALSE; 
				break;
			}
		}

		for(UINT i = 2 ; i < bufSize /* == 512 */; i++)
		{
			//ASCII
			if(!((*(pvBuffer+i) >= LowerBound_ASCII && *(pvBuffer+i) <= UpperBound_ASCII) || 
				*(pvBuffer+i) == 0x0d ||
				*(pvBuffer+i) == 0x0a ||
				*(pvBuffer+i) == 0x09 ) && isASCII != FALSE)
			{
				i = 0;
				isASCII = FALSE;
			}

			if(isASCII == FALSE)
				break;
		}

		if(isUTF8 == TRUE && isASCII == TRUE)	
			return FORMAT_UNKNOWN;
		else if(isUTF8 == TRUE)					
			return FORMAT_UTF8;
		else if(isASCII == TRUE)				
			return FORMAT_ASCII;
	}

	return FORMAT_UNKNOWN;
}

// Not yet
UINT CPageClassifier::RandomPageClassifier(CString srcPath1, CString srtPath2, CString dstPath)
{
	return TRUE;
}

// Not yet
UINT CPageClassifier::DetectPageSize(CString srcPath1, CString srtPath2)
{
	return TRUE;
}

