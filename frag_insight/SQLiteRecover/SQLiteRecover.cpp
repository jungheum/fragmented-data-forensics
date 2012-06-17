/**
 *	@file	SQLiteRecover.cpp
 *	@date	2011/06/17
 *	@author	Sangjun Jeon (heros86@korea.ac.kr)

 *	@brief	SQLite Recover
 */

/**
	@mainpage	SQLite Recover

	@section	MODIFYINFO	History

		- Sangjun Jeon / 2011.06.17
			- Initial Version
		- Jungheum Park / 2011.06.27
			- Add & Modify some functions for supporting DFRWS Challenges
 */

#include "stdafx.h"
#include "SQLiteRecover.h"


// SQLiteRecover

SQLiteRecover::SQLiteRecover()
{
}


SQLiteRecover::~SQLiteRecover()
{
	for(int i = 0; i < arrTableData.GetSize(); i++)
	{
		delete arrTableData[i].FieldArray;
		delete arrTableData[i].NorRowArray;
		delete arrTableData[i].DelRowArray;
	}
}


UINT SQLiteRecover::GetDataFromPages(CString PageFileName, UINT PageSize_parm, UINT EncodedType_parm)
{
	CFile PageFile;
	if (PageFile.Open(PageFileName, CFile::modeRead | CFile::shareDenyNone) != TRUE)
	{
		return PROCESS_FAIL;
	}

	this->PageSize = PageSize_parm;

	UINT8 *PageBuff;
	PageBuff = (UINT8 *)malloc(this->PageSize);

	UINT totalPageCount = 0;
	UINT classifiedPageCount = 0;
	UINT pageSignature = 0;

	UINT   readSize = 0;
	UINT64 totalSize = 0;
	UINT64 srcFileSize = PageFile.GetLength();

	while(srcFileSize > totalSize)
	{
		if(srcFileSize-totalSize < this->PageSize)
			readSize = (UINT)(srcFileSize-totalSize);
		else
			readSize = this->PageSize;

		if(PageFile.Read(PageBuff, this->PageSize) != readSize)
		{
			break;
		}
		
		//-------------------------------------------------------------------------------

		RecordPageExtractor PageExtractModule(this->PageSize, EncodedType_parm);

		PageExtractModule.GetNormalRecordSet(PageBuff);
		
		UINT type = 0;
		TableData tTableData;
		tTableData.pageOffset = totalSize;
		tTableData.FieldArray = new ARRFieldArray;
		tTableData.NorRowArray = new ARRRowArray;
		tTableData.DelRowArray = new ARRRowArray; 

		// NormalTable_InPage
		for(int jdx = 0 ; jdx < PageExtractModule.NormalTable_InPage.GetCount() ; jdx++)
		{
			CRowArray RowArr;
			RowArr = PageExtractModule.NormalTable_InPage.GetAt(jdx);
			tTableData.NorRowArray->Add(RowArr);

			ARRField arrField;
			CTypeArray TypeArr;
			TypeArr = PageExtractModule.Types_InPage.GetAt(jdx);

			for(int i = 0 ; i < TypeArr.GetCount() ; i++)
			{
				type = TypeArr.GetAt(i);

				Field field;
				field.cid.Format(_T("%d"), i);

				switch(type)
				{
				case TYPE_NULL:
					field.type = _T("NULL");
					field.length = -1;	
					break;
				case TYPE_INTEGER0:
					field.type = _T("INTEGER");
					field.length = 0;
					break;
				case TYPE_INTEGER1:
					field.type = _T("INTEGER");
					field.length = 1;
					break;
				case TYPE_INTEGER2:
					field.type = _T("INTEGER");
					field.length = 2;
					break;
				case TYPE_INTEGER3:
					field.type = _T("INTEGER");
					field.length = 3;
					break;
				case TYPE_INTEGER4:
					field.type = _T("INTEGER");
					field.length = 4;
					break;
				case TYPE_INTEGER6:
					field.type = _T("INTEGER");
					field.length = 6;
					break;
				case TYPE_INTEGER8:
					field.type = _T("INTEGER");
					field.length = 8;
					break;
				case TYPE_NUMERIC:
					field.type = _T("IEEE754");
					field.length = 9;
					break;
				case TYPE_BLOB:
					field.type = _T("BLOB");
					field.length = 12;	// variable length
					break;
				case TYPE_TEXT:
					field.type = _T("TEXT");
					field.length = 13;	// variable length
					break;
				}

				arrField.Add(field);
			}

			tTableData.FieldArray->Add(arrField);
		}

		arrTableData.Add(tTableData);
		
		//-------------------------------------------------------------------------------

		ZeroMemory(PageBuff, this->PageSize);

		totalSize += readSize;
		totalPageCount++;
	}

	safe_free(PageBuff);
	PageFile.Close();
	return TRUE;
}

UINT SQLiteRecover::GetRecordsFromPageBuffer(PBYTE buffer, UINT bufSize, UINT offsetForDebug, UINT encoding)
{
	UINT totalPageCount = 0;
	UINT classifiedPageCount = 0;
	UINT pageSignature = 0;

// 	if (offsetForDebug == 1953792)
// 		Sleep(1);

	this->PageSize = AssumeSQLitePageSize(buffer, bufSize);
	if (this->PageSize == 0)
		return FALSE;

	RecordPageExtractor PageExtractModule(this->PageSize, encoding);
	PageExtractModule.GetNormalRecordSet(buffer);

	UINT type = 0;
	TableData tTableData;
	tTableData.pageOffset = offsetForDebug;
	tTableData.FieldArray = new ARRFieldArray;
	tTableData.NorRowArray = new ARRRowArray;
	tTableData.DelRowArray = new ARRRowArray;

	// NormalTable_InPage
	for(int jdx = 0; jdx < PageExtractModule.NormalTable_InPage.GetCount(); jdx++)
	{
		CRowArray RowArr;
		RowArr = PageExtractModule.NormalTable_InPage.GetAt(jdx);
		tTableData.NorRowArray->Add(RowArr);

		ARRField arrField;
		CTypeArray TypeArr;
		TypeArr = PageExtractModule.Types_InPage.GetAt(jdx);

		for(int i = 0 ; i < TypeArr.GetCount() ; i++)
		{
			type = TypeArr.GetAt(i);

			Field field;
			field.cid.Format(_T("%d"), i);

			switch(type)
			{
			case TYPE_NULL:
				field.type = _T("NULL");
				field.length = -1;	
				break;
			case TYPE_INTEGER0:
				field.type = _T("INTEGER");
				field.length = 0;
				break;
			case TYPE_INTEGER1:
				field.type = _T("INTEGER");
				field.length = 1;
				break;
			case TYPE_INTEGER2:
				field.type = _T("INTEGER");
				field.length = 2;
				break;
			case TYPE_INTEGER3:
				field.type = _T("INTEGER");
				field.length = 3;
				break;
			case TYPE_INTEGER4:
				field.type = _T("INTEGER");
				field.length = 4;
				break;
			case TYPE_INTEGER6:
				field.type = _T("INTEGER");
				field.length = 6;
				break;
			case TYPE_INTEGER8:
				field.type = _T("INTEGER");
				field.length = 8;
				break;
			case TYPE_NUMERIC:
				field.type = _T("IEEE754");
				field.length = 9;
				break;
			case TYPE_BLOB:
				field.type = _T("BLOB");
				field.length = 12;	// variable length
				break;
			case TYPE_TEXT:
				field.type = _T("TEXT");
				field.length = 13;	// variable length
				break;
			}

			arrField.Add(field);
		}

		tTableData.FieldArray->Add(arrField);
	}

	arrTableData.Add(tTableData);
	return TRUE;
}

// Assume sqlite page size
UINT SQLiteRecover::AssumeSQLitePageSize(PBYTE buffer, UINT bufSize)
{
	UINT	sqlitePageSize = 0;
	USHORT	offset = 0;
	USHORT	numOfcells = 0;
	UINT16	tmpOffset = 0;
	UINT64	cellSize = 0;

	// 3~4 : Number of cells on this page 
	numOfcells = big_2bytetoint(&buffer[3]);

	for(UINT idx = 0 ; idx < numOfcells; idx++)
	{
		if(8+(idx*2) + 1 < bufSize)									
		{
			tmpOffset = big_2bytetoint(&buffer[8 + (idx*2)]);

			if (offset < tmpOffset)
				offset = tmpOffset;
		}
	}

	if (offset > bufSize-9)
	{
		while (sqlitePageSize > bufSize)
		{
			sqlitePageSize = sqlitePageSize/2;
		}

		return sqlitePageSize;
	}

	if (offset != 0)
	{
		RecordPageExtractor PageExtractModule;
		PageExtractModule.CalcValriableLengthInteger(buffer+offset, &cellSize);

		tmpOffset = offset+(UINT16)cellSize;

		if (tmpOffset >= 0x0000 && tmpOffset < 0x0200)
		{// 512
			if (sqlitePageSize < 512)
				sqlitePageSize = 512;
		}
		else if (tmpOffset >= 0x0200 && tmpOffset < 0x0400)
		{// 1024
			if (sqlitePageSize < 1024)
				sqlitePageSize = 1024;
		}
		else if (tmpOffset >= 0x0400 && tmpOffset < 0x0800)
		{// 2048
			if (sqlitePageSize < 2048)
				sqlitePageSize = 2048;
		}
		else if (tmpOffset >= 0x0800 && tmpOffset < 0x1000)
		{// 4096
			if (sqlitePageSize < 4096)
				sqlitePageSize = 4096;
		}
		else if (tmpOffset >= 0x1000 && tmpOffset < 0x2000)
		{// 8192
			if (sqlitePageSize < 8192)
				sqlitePageSize = 8192;
		}
		else if (tmpOffset >= 0x2000 && tmpOffset < 0x4000)
		{// 16384
			if (sqlitePageSize < 16384)
				sqlitePageSize = 16384;
		}
		else if (tmpOffset >= 0x4000 && tmpOffset < 0x8000)
		{// 32768
			if (sqlitePageSize < 32768)
				sqlitePageSize = 32768;
		}
		else if (tmpOffset >= 0x8000 && tmpOffset < 0x10000)
		{// 65536
			if (sqlitePageSize < 65536)
				sqlitePageSize = 65536;
		}

		while (sqlitePageSize > bufSize)
		{
			sqlitePageSize = sqlitePageSize/2;
		}

// 		if (sqlitePageSize != 4096)
// 		{
// 			sqlitePageSize = 0;
// 		}
	}

	return sqlitePageSize;
}

