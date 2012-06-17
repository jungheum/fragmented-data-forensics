/**
	@mainpage		frag_insight
	@date			2012/04/11
	@author			Jungheum Park (junghmi@gmail.com)
	@affiliation	Digital Forensic Research Center @ Korea University

	@section	MODIFYINFO	History

				- 0.1.0 / Jungheum Park / 2011.06.05
					- Initial Version for DFRWS 2011 Forensics Challenge
				- 0.1.1 / Jungheum Park / 2011.06.25
					- Add classification functions
				- 0.1.2 / Jungheum Park / 2011.07.07
					- Add ananysis functions
				- 0.2.0 / Jungheum Park / 2011.07.20
					- Modify some functions
				- 0.3.0 / Jungheum Park / 2011.12.18
					- Revision for fragmented data forensics
					- Enhance classification algorithms
				- 0.3.1 / Jungheum Park / 2012.01.11
					- Enhance analysis algorithms
				- 0.3.2 / Jungheum Park / 2012.04.11
					- Enhance analysis algorithms

	@license

		License: DFRC@KU

		Copyright (c) 2011-2012, Jungheum Park (junghmi@gmail.com),
		DFRC(Digital Forensic Research Center). All rights reserved.

		Refer to AUTHORS for acknowledgements.

		Redistribution and use in source and binary forms, with or without modification, 
		are permitted for any non-commercial purpose provided that the following conditions are met:

		- Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
		- Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.
		- All advertising materials mentioning features or use of this software
		must acknowledge the contribution by people stated in the acknowledgements.

		You may not use or distribute this Software or any derivative works in any form 
		for commercial purposes. Examples of commercial purposes would be running business 
		operations, licensing, leasing, or selling the Software, distributing the Software 
		for use with commercial products, using the Software in the creation or use of 
		commercial products or any other activity which purpose is to procure a commercial 
		gain to you or others. 

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
		EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
		OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
		SHALL JUERGEN GALL AND CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
		SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
		OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
		HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
		SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */


#include "stdafx.h"
#include "frag_insight.h"

#include "PageClassifier.h"
#include "PageAnalyzer.h"

#define FI_VERSION "0.3.2"

int Usage(void);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	if(argc != 4)
	{
		return Usage();
	}

	UINT pageSize = _ttoi(argv[2]);
	if (pageSize%512 != 0)
	{
		printf("Invalid pagesize.\n");
		printf("Pagesize should be a multiple of 512.\n");
		return 0;
	}

	CString srcPath1 = argv[1];
	CString strPath2 = _T("");
	CString dstPath  = _T("");
	CString dstPathBase = argv[3];
		
	WCHAR full[_MAX_PATH] = {0,0};
	_wfullpath(full, srcPath1, _MAX_PATH);
	srcPath1 = full;

	ZeroMemory(full, sizeof(full));
	_wfullpath(full, dstPathBase, _MAX_PATH);
	dstPathBase = full;

		
	dstPath.Format(_T("%s\\1st. CLASSIFICATION_RESULT"), dstPathBase);
	printf("\n--- Page Classification ---\n");

	CPageClassifier pageClassifier(pageSize);
	pageClassifier.RunPageClassifier(srcPath1, strPath2, dstPath);

	srcPath1 = dstPath;
	
	dstPath.Format(_T("%s\\2nd. ANALYSIS_RESULT"), dstPathBase);
	printf("\n--- Page Analysis ---\n");

	CPageAnalyzer pageAnalyzer(pageSize);
	pageAnalyzer.RunPageAnalyzer(srcPath1, strPath2, dstPath);


	printf("\n");
	return TRUE;
}

int Usage(void)
{
	CStringA strMsg = "";
	strMsg.Format("  frag_insight %s for fragmented data forensics\n", FI_VERSION);

	printf("----------------------------------------------------------------\n\n");
	printf(strMsg);
	printf("  (https://github.com/jungheum/fragmented-data-forensics)\n\n");
	printf("----------------------------------------------------------------\n");
	printf("\n");
	printf("  Usage : frag_insight  <target image>  <pagesize>  <output path>\n\n");
	printf("  ex #1 > frag_insight  c:\\image.dd     2048        d:\\output\n");
	printf("  ex #2 > frag_insight  imageUnlloca     4096        outputPath\n");
	printf("\n");
	return 0;
}


