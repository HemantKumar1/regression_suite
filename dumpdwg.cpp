//
//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2014 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form. 
//
//////////////////////////////////////////////////////////////////////////////
//
// DUMPDWG.CPP
//
// DESCRIPTION:
//
// This console application prints the values of entity data
// members.  The 's' version is linked with acisstub.dll.
// It uses the dumpcore.cpp module, like many of these samples,
// for walking through the DWG file. 

#include "afxwin.h"
#include "shlwapi.h"
#include "urlmon.h"
#include "Wininet.h"
#include "adesk.h"
#include "dbsymtb.h"
#include "dbents.h"
#include "dbelipse.h"
#include "dbspline.h"
#include "dblead.h"
#include "dbray.h"
#include "dbxline.h"
#include "dbmline.h"
#include "dbbody.h"
#include "dbregion.h"
#include "dbsol3d.h"
#include "acgi.h"
#include "myacgi.h"
#include "acestext.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "dbmstyle.h"
#include "rxregsvc.h"

#include "dbapserv.h"
#include "dir.h"
#include "acdbxref.h"
#include "dbacis.h"

#include <shellapi.h>
#include <WinUser.h>
#include <Windows.h>
#include <string>
#include <list>
#include <iostream>
#include <Cstring>


#define PI ((double)3.14159265358979323846)
#define PIOVER180 ((double)PI/180)

extern "C" Acad::ErrorStatus acdbGetDimBlock(AcDbDimension *pDim,AcDbObjectId  &id);

void readDrawing(const TCHAR *pFileName);
void dumpDatabase(AcDbDatabase *pDatabase, const TCHAR *xmlname);

void entInfo(AcDbEntity *pEnt, int size);
void dumpEntInfo();
void dumpDwgHeaderVars(AcDbDatabase *pDatabase);

int Copy = 0;
int Header = 0;
int Tables = 0;
int doExtents = 0;
int showErased = 0;
int resolveXrefs = 0;
bool bCorruptDrawing = false;


static const TCHAR acIsmObjDbxFile[] = _T("acIsmObj20.dbx");

class DumpDwgHostApp : public AcDbHostApplicationServices
{
public:
	~DumpDwgHostApp();
	Acad::ErrorStatus findFile(TCHAR* pcFullPathOut, int nBufferLength,
		const TCHAR* pcFilename, AcDbDatabase* pDb = NULL,
		AcDbHostApplicationServices::FindFileHint = kDefault);
	// These two functions return the full path to the root folder where roamable/local 
	// customizable files were installed. Note that the user may have reconfigured 
	// the location of some the customizable files using the Options Dialog 
	// therefore these functions should not be used to locate customizable files. 
	// To locate customizable files either use the findFile function or the 
	// appropriate system variable for the given file type. 
	//
	Acad::ErrorStatus getLocalRootFolder(const TCHAR*& folder);

	// make sure you implement getAlternateFontName. In case your findFile implementation
	// fails to find a font you should return a font name here that is guaranteed to exist.
	virtual const TCHAR * getAlternateFontName() const override
	{
		return _T("txt.shx"); //findFile will be called again with this name
	}

	void searchDirectory(std::wstring& folderPath, std::list<std::wstring>& filesList);
#ifdef REALDWGMSI //for AutoCAD Realdwg test install only
	const ACHAR * getMachineRegistryProductRootKey ();
#endif

private:
	mutable CMapStringToString m_localToUrl;
};

#ifdef REALDWGMSI //for AutoCAD Realdwg test install only
CString registryRoot = _T(/*MSG0*/"Software\\<MyProduct>\\<VersionX.Y>"); 
const ACHAR * DumpDwgHostApp::getMachineRegistryProductRootKey ()
{
	return registryRoot;
}
#endif

DumpDwgHostApp::~DumpDwgHostApp()
{
	CString local, url;
	for (POSITION pos = m_localToUrl.GetStartPosition(); pos != NULL;)
	{
		m_localToUrl.GetNextAssoc(pos, local, url);
		DeleteUrlCacheEntry(url);
	}
}

// Return the Install directory for customizable files
Acad::ErrorStatus
DumpDwgHostApp::getLocalRootFolder(const TCHAR*& folder)
{
	Acad::ErrorStatus ret = Acad::eOk;
	static TCHAR buf[MAX_PATH] = _T("\0"); //MDI SAFE
	if (buf[0] == 0)
		if (GetModuleFileName(NULL, buf, MAX_PATH) != 0)
			ret = Acad::eRegistryAccessError;
	folder = buf;
	return ret;
}

Acad::ErrorStatus
DumpDwgHostApp::findFile(TCHAR* pcFullPathOut, int nBufferLength,
const TCHAR* pcFilename, AcDbDatabase* pDb,
AcDbHostApplicationServices::FindFileHint hint)
{
	TCHAR pExtension[5];
	switch (hint)
	{
	case kCompiledShapeFile:
		_tcscpy(pExtension, _T(".shx"));
		break;
	case kTrueTypeFontFile:
		_tcscpy(pExtension, _T(".ttf"));
		break;
	case kPatternFile:
		_tcscpy(pExtension, _T(".pat"));
		break;
	case kARXApplication:
		_tcscpy(pExtension, _T(".dbx"));
		break;
	case kFontMapFile:
		_tcscpy(pExtension, _T(".fmp"));
		break;
	case kXRefDrawing:
		_tcscpy(pExtension, _T(".dwg"));
		break;
	case kFontFile:                // Fall through.  These could have
	case kEmbeddedImageFile:       // various extensions
	default:
		pExtension[0] = _T('\0');
		break;
	}
	TCHAR* filePart;
	DWORD result;
	result = SearchPath(NULL, pcFilename, pExtension, nBufferLength,
		pcFullPathOut, &filePart);
	if (result && result < (DWORD)nBufferLength)
		return Acad::eOk;
	else
		return Acad::eFileNotFound;
}

void searchDirectory(CString& folderPath, std::list<CString>& filesList)
{
	CFileFind finder;
	CString strWildcard(folderPath);
	strWildcard += _T("\\*.*");
	BOOL bWorking = finder.FindFile(strWildcard);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots())
			continue;

		if (finder.IsDirectory())
		{
			CString str = finder.GetFilePath();

			int sub = str.Find(L"test");
			if (sub > 0)
				filesList.push_back(finder.GetFilePath());
			else
				searchDirectory(str, filesList);
		}

	}

	finder.Close();
}

void searchDirectory2(CString& folderPath, std::list<CString>& filesList2)
{
	for (auto& file : filesList2)
	{
		CFileFind finder;
		CString strWildcard(file);
		strWildcard += _T("\\*.dxf");
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			CString str = finder.GetFilePath();
			file = str;
			

			//if (finder.IsDots())
			//	continue;

			//if (finder.IsDirectory())
			//{
			//	CString str = finder.GetFilePath();

			//	file = str;

			//	//int sub = str.Find(L"test");
			//	//if (sub>0)
			//	//filesList.push_back(finder.GetFilePath());
			//	//else
			//	//searchDirectory(str, filesList);
			//}
			//bWorking = finder.FindNextFile();
		}

		finder.Close();
	}
}
//
//void searchDirectory2(CString& folderPath, std::list<CString>& filesList2)
//{
//	CFileFind finder;
//	CString strWildcard(folderPath);
//	strWildcard += _T("\\*.*");
//	BOOL bWorking = finder.FindFile(strWildcard);
//
//	while (bWorking)
//	{
//		bWorking = finder.FindNextFile();
//
//		if (finder.IsDots())
//			continue;
//
//		if (finder.IsDirectory())
//		{
//			CString str = finder.GetFilePath();
//
//			int sub = str.Find(L"dxf");
//			if (sub>0)
//				filesList2.push_back(finder.GetFilePath());
//			else
//				searchDirectory2(str, filesList2);
//		}
//
//	}
//
//	finder.Close();
//}

DumpDwgHostApp gDumpDwgHostApp;

int _tmain(int argc, TCHAR *argv[])
{
	acdbSetHostApplicationServices(&gDumpDwgHostApp);
	acdbValidateSetup(0x00000409); //English

	// Load the ISM dll
	acrxLoadModule(acIsmObjDbxFile, 0);

	std::list<CString> filesList;
	CString folderPath = "E:\\TestData";
	searchDirectory(folderPath, filesList);
	//searchDirectory2(folderPath, filesList2);




	// loop
	for (std::list<CString>::iterator it = filesList.begin(); it != filesList.end(); ++it)
	{
		//zumen call 
		ShellExecute(NULL, L"open", L"E:\\Projects\\MiSUMi\\TestRun\\zumen.exe", *it, NULL, SW_SHOWNORMAL);

		// dxf will be generated

		// generate xml
		//readDrawing(L"C:\\Users\\Chetan\\Desktop\\Test Suite\\test_pts.dxf");
	}
	std::list<CString> filesList2;
	searchDirectory(folderPath, filesList2);
	searchDirectory2(folderPath, filesList2);
	// loop
	for (std::list<CString>::iterator it2 = filesList2.begin(); it2 != filesList2.end(); ++it2)
	{
		std::wstring filename = it2->GetString();
		// generate xml
		readDrawing(filename.c_str());
	}
	//std::list<CString> filesList3;
	CString folderPath2 = "E:\\Benchmark_Data";
	std::list<CString> filesList4;
	searchDirectory(folderPath2, filesList4);
	searchDirectory2(folderPath2, filesList4);
	for (std::list<CString>::iterator it3 = filesList4.begin(); it3 != filesList4.end(); ++it3)
	{
		std::wstring filename2 = it3->GetString();
		
		// generate xml
		readDrawing(filename2.c_str());
	}

	// Unload the ISM dll
	acrxUnloadModule(acIsmObjDbxFile);

	//Host app shutdown before exiting
	acdbCleanUp();
	//{
	//	ShellExecute(NULL, L"open", L"E:\\Benchmark\\BatchTestingTool\\bin\\Debug\\BatchTestingTool.exe", L"\"E:\\TestData\" \"E:\\Benchmark_Data\"  \"E:\\Comparison_Report\" \"E:\\Comparison_Report\\html_files\"", NULL, SW_SHOWNORMAL);
	//}
	return 0;
}

void readDrawing(const TCHAR *pFileName)
{
	AcDbDatabase *pDb = new AcDbDatabase(Adesk::kFalse);
	if (pDb == NULL)
		return;

	// Read the drawing  or dxf file.
	if (Acad::eOk == pDb->dxfIn(pFileName))
	{
		acdbHostApplicationServices()->setWorkingDatabase(pDb);

		dumpDatabase(pDb,pFileName);
	}
	else
		_tprintf(_T("Can't open %s\n"), pFileName);
	
	delete pDb;

	_tprintf(_T("\n"));
}
