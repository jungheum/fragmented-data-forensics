/**
 *	@file	PageClassifier.h
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


#pragma once
#include "atlcoll.h"
#include "Utils/Utils.h"

#define MB				1024*1024
#define BUF_SIZE		5*MB
#define safe_free(p)	{ if(p) { free(p); (p) = NULL; } }

enum 
{
	FS_UNKNOWN = 0x00,
	FS_YAFFS = 0x01,
	FS_EXT4  = 0X02,
};

class CPageClassifier
{
public:
	CPageClassifier();

	CPageClassifier(UINT base);

	~CPageClassifier(void);


public:
	UINT pageSizeBase;

	UINT htmlCounts;
	UINT xmlCounts;
	UINT oleCounts;
	UINT pdfCounts;
	UINT asciiCounts;
	UINT utf8Counts;

	CUtils utils;

public:
	// Public Function
	UINT RunPageClassifier(CString srcPath1, CString srtPath2, CString dstPath);


private:
	// Page Size Detection (Not yet)
	UINT DetectPageSize(CString srcPath1, CString srtPath2);


private:
	// Meta Page Classification
	UINT MetaPageClassifier(CString srcPath, CString dstPath);
	BOOL IsMetaPage(PBYTE buffer, UINT pageSize);
	BOOL IsYaffsMetaPage(PBYTE buffer, UINT pageSize);
	BOOL IsExt4MetaPage(PBYTE buffer, UINT pageSize);


private:
	// Hash-based Classification
	UINT HashBasedClassifier(CString srcPath, CString dstPath);


private:
	// Statistical Classification
	UINT StatisticalClassifier(CString srcPath1, CString srtPath2, CString dstPath);
	

private:
	// File Format Classification
	UINT FileFormatClassifier(CString srcPath1, CString srtPath2, CString dstPath);
	UINT VerifyPageSignature(PBYTE buffer, UINT pageSize, BOOL bOpt = FALSE);
	UINT ClassifyPage(PBYTE buffer, UINT readSize, UINT pageSignature, CString dstPath);
	BOOL ClassifySQLitePage(PBYTE buffer, UINT pageSize, UINT pageSignature, CString dstPath);


private:
	// Random page Classification
	UINT RandomPageClassifier(CString srcPath1, CString srtPath2, CString dstPath);
};


