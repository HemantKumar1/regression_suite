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


// DUMPCORE.CPP
using namespace std;
#include<vector>
#include "windows.h"
#include "adesk.h"
#include "dbsymtb.h"
#include "dbents.h"
#include "dbelipse.h"
#include "dbspline.h"
#include "dbhatch.h"
#include "dblead.h"
#include "dbray.h"
#include "dbxline.h"
#include "dbmline.h"
#include "dbbody.h"
#include "dbregion.h"
#include "dbsol3d.h"
#include "dbfcf.h"
#include "stdlib.h"

#include "string.h"
#include "tchar.h"

#include "tinyxml.h"
#include "entitydumper.h"

#define PI ((double)3.14159265358979323846)
#define PIOVER180 ((double)PI/180)

void dumpDatabase(AcDbDatabase *pDatabase, const TCHAR *xmlname);
void dumpBlockTable(AcDbBlockTable *pBlockTable, TiXmlElement* element);
void dumpLayerTable(AcDbLayerTable *pBlockTable, TiXmlElement* element);
void dumpViewTable(AcDbViewTable *pViewTable, TiXmlElement* element);
void dumpEntity(AcDbEntity *pEnt, TiXmlElement* element);

void dumpAcDb2dPolyline(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDb3dPolyline(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbArc(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbCircle(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbEllipse(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbLeader(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbLine(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbPolyline(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbRay(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbSpline(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbXline(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDb2LineAngularDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDb3PointAngularDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbAlignedDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbDiametricDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbOrdinateDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbRadialDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbRotatedDimension(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbFcf(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbHatch(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbMText(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbPoint(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbRegion(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbShape(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbSolid(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDbText(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDb2dVertex(AcDbEntity *pEnt, TiXmlElement* element);
void dumpAcDb3dPolylineVertex(AcDbEntity *pEnt, TiXmlElement* element);

string ws2s(TCHAR* originalStr);

static int dimensionCount = 0;

#ifdef ACAP
extern "C" void __stdcall StartCAP();
extern "C" void __stdcall StopCAP();
extern "C" int __stdcall DumpCAP(int (*showProgress) (const TCHAR *msg));
#endif

string ws2s(TCHAR* originalStr)
{
    std::wstring wstr(originalStr);
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);

    return strTo;
}

void dumpDatabase(AcDbDatabase *pDatabase, const TCHAR *xmlname)
{
    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "", "");
    doc->SetCondenseWhiteSpace(false);

	TiXmlElement* rootElement = new TiXmlElement("ROOT");
	doc->LinkEndChild(rootElement);
        
    AcDbLayerTable* pLayerTable;
    if (pDatabase->getLayerTable(pLayerTable, AcDb::kForRead) == Acad::eOk)
    {
        TiXmlElement* tLayerTable = new TiXmlElement("LAYERTABLE");
        dumpLayerTable(pLayerTable, tLayerTable);
        pLayerTable->close();
		rootElement->LinkEndChild(tLayerTable);
    }
    
    AcDbViewTable* pViewTable;
    if (pDatabase->getViewTable(pViewTable, AcDb::kForRead) == Acad::eOk)
    {
        TiXmlElement* tViewTable = new TiXmlElement("VIEWTABLE");
        dumpViewTable(pViewTable, tViewTable);
        pViewTable->close();
		rootElement->LinkEndChild(tViewTable);
    }

    AcDbBlockTable* pBlockTable;
    if (pDatabase->getBlockTable(pBlockTable, AcDb::kForRead) == Acad::eOk)
    {
        TiXmlElement* tBlockTable = new TiXmlElement("BLOCKTABLE");
        dumpBlockTable(pBlockTable, tBlockTable);
        pBlockTable->close();

        XMLGeometryWriter::dumpInt(tBlockTable, dimensionCount, "Dimensions");

		rootElement->LinkEndChild(tBlockTable);
    }
	
		// give path to folder where dxf is created

	//std::strcat(pFileName, "\\Modified.xml");
	/*std::wstring xmlSavePath(pFileName);
	std::wstring str=xmlname;*/
	std::wstring xmlSavePath(xmlname);
	xmlSavePath.pop_back();
	xmlSavePath.pop_back();
	xmlSavePath.pop_back();
	xmlSavePath.append(L"xml");
	
	/*std::wstring str2 = str.substr(24,12);
	str2.append(L".xml");*/
	//xmlSavePath += L"\\Modified.xml";
	//xmlSavePath.append(str2);
	std::string xmlSavePathStr = std::string(xmlSavePath.begin(), xmlSavePath.end());

	//auto xmlSavePath = (pFileName) + L"\\Modified.xml";
	bool state = doc->SaveFile(xmlSavePathStr.c_str());
    delete doc;
}

void dumpBlockTable(AcDbBlockTable *pBlockTable, TiXmlElement* element)
{
    AcDbBlockTableIterator *pIter;
    AcDbBlockTableRecord *pRecord;
    pBlockTable->newIterator(pIter);

    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                if (!wcscmp(pName,L"*MODEL_SPACE") || !wcscmp(pName,L"*PAPER_SPACE"))
                {
                    std::string spaceName;
                    if (wcscmp(pName, L"*MODEL_SPACE"))
                        spaceName = "MODEL_SPACE";
                    else
                        spaceName = "PAPER_SPACE";

                    TiXmlElement* tBlockTableRecord = new TiXmlElement(spaceName.c_str());
                    
                    AcDbBlockTableRecordIterator *pRecordIter;
                    if (pRecord->newIterator(pRecordIter) == Acad::eOk)
                    {
                        AcDbEntity *pEnt;
                        while (!pRecordIter->done())
                        {
                            if (pRecordIter->getEntity(pEnt, AcDb::kForRead) == Acad::eOk)
                            {
                                dumpEntity(pEnt, tBlockTableRecord);

                                pEnt->close();
                            }
                            pRecordIter->step();
                        }
                        delete pRecordIter;
                    }
                    element->LinkEndChild(tBlockTableRecord);
                }
                acdbFree(pName);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpLayerTable(AcDbLayerTable *pLayerTable, TiXmlElement* element)
{
    AcDbLayerTableIterator *pIter;
    AcDbLayerTableRecord *pRecord;
    pLayerTable->newIterator(pIter);

    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {            
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                TCHAR ltName[128] = _T("????????");

                TiXmlElement* tLayerTableRecord = new TiXmlElement("Layer");

                AcDbObjectId linetypeid = pRecord->linetypeObjectId();
                AcDbLinetypeTableRecord *pLinetypeRecord;
            
                Acad::ErrorStatus es = acdbOpenObject(pLinetypeRecord, linetypeid, AcDb::kForRead);
                if (pLinetypeRecord)
                {
                    TCHAR *pLinetypeName = 0;
                    if (pLinetypeRecord->getName(pLinetypeName) == Acad::eOk)
                    {
                        _tcscpy(ltName, pLinetypeName);
                        acdbFree(pLinetypeName);
                    }
                    pLinetypeRecord->close();
                }

                XMLGeometryWriter::dumpText(tLayerTableRecord, ws2s(pName), "Name");
                int colorIndex = pRecord->color().colorIndex();
                XMLGeometryWriter::dumpInt(tLayerTableRecord,colorIndex, "ColorIndex");
                XMLGeometryWriter::dumpText(tLayerTableRecord, ws2s(ltName), "LineType");

                acdbFree(pName);
                element->LinkEndChild(tLayerTableRecord);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpViewTable(AcDbViewTable *pViewTable, TiXmlElement* element)
{
    AcDbViewTableIterator *pIter;
    AcDbViewTableRecord *pRecord;
    pViewTable->newIterator(pIter);

    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TiXmlElement* tView = new TiXmlElement("View");
            
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                Adesk::Boolean isPaperSpaceView = pRecord->isPaperspaceView();
                AcGePoint2d centerPoint = pRecord->centerPoint();
                double height = pRecord->height();
                double width = pRecord->width();
                AcGePoint3d target = pRecord->target();
                AcGeVector3d viewDirection = pRecord->viewDirection();
                double viewTwist = pRecord->viewTwist();
                double lensLength = pRecord->lensLength();
                double frontClipDistance = pRecord->frontClipDistance();
                double backClipDistance = pRecord->backClipDistance();
                Adesk::Boolean prespectiveEnabled = pRecord->perspectiveEnabled();
                Adesk::Boolean frontClipEnabled = pRecord->frontClipEnabled();
                Adesk::Boolean backClipEnabled = pRecord->backClipEnabled();
                Adesk::Boolean frontClipAtEye = pRecord->frontClipAtEye();

                XMLGeometryWriter::dumpText(tView, ws2s(pName), "Name");
                XMLGeometryWriter::dumpBoolean(tView, isPaperSpaceView, "IsPaperSpaceView");
                XMLGeometryWriter::dumpPoint(tView, centerPoint, "CeneterPoint");
                XMLGeometryWriter::dumpDouble(tView, height, "Height");
                XMLGeometryWriter::dumpDouble(tView, width, "Width");
                XMLGeometryWriter::dumpPoint(tView, target, "Target");
                XMLGeometryWriter::dumpVector(tView, viewDirection, "ViewDirection");
                XMLGeometryWriter::dumpDouble(tView, viewTwist, "ViewTwist");
                XMLGeometryWriter::dumpDouble(tView, lensLength, "LensLength");
                XMLGeometryWriter::dumpDouble(tView, frontClipDistance, "FrontClipDistance");
                XMLGeometryWriter::dumpDouble(tView, backClipDistance, "BackClipDistance");
                XMLGeometryWriter::dumpBoolean(tView, prespectiveEnabled, "PrespectiveEnabled");
                XMLGeometryWriter::dumpBoolean(tView, frontClipEnabled, "FrontClipEnabled");
                XMLGeometryWriter::dumpBoolean(tView, backClipEnabled, "BackClipEnabled");
                XMLGeometryWriter::dumpBoolean(tView, frontClipAtEye, "FrontClipAtEye");

                acdbFree(pName);
            }
            pRecord->close();

            element->LinkEndChild(tView);
        }
        pIter->step();
    }
    delete pIter;
}

struct ENTNAMES
{
    const TCHAR *pName;
    void (* func)(AcDbEntity *, TiXmlElement*);
};

ENTNAMES EntityList[] =
{
    _T("AcDb2dPolyline"),                   dumpAcDb2dPolyline,
    _T("AcDb3dPolyline"),                   dumpAcDb3dPolyline,
    _T("AcDbArc"),                          dumpAcDbArc,
    _T("AcDbCircle"),                       dumpAcDbCircle,
    _T("AcDbEllipse"),                      dumpAcDbEllipse,
    _T("AcDbLeader"),                       dumpAcDbLeader,
    _T("AcDbLine"),                         dumpAcDbLine,
    _T("AcDbPolyline"),                     dumpAcDbPolyline,
    _T("AcDbRay"),                          dumpAcDbRay,
    _T("AcDbSpline"),                       dumpAcDbSpline,
    _T("AcDbXline"),                        dumpAcDbXline,
    _T("AcDbDimension"),                    dumpAcDbDimension,
    _T("AcDb2LineAngularDimension"),        dumpAcDb2LineAngularDimension,
    _T("AcDb3PointAngularDimension"),       dumpAcDb3PointAngularDimension,
    _T("AcDbAlignedDimension"),             dumpAcDbAlignedDimension,
    _T("AcDbDiametricDimension"),           dumpAcDbDiametricDimension,
    _T("AcDbOrdinateDimension"),            dumpAcDbOrdinateDimension,
    _T("AcDbRadialDimension"),              dumpAcDbRadialDimension,
    _T("AcDbRotatedDimension"),             dumpAcDbRotatedDimension,
    _T("AcDbFcf"),                          dumpAcDbFcf,
    _T("AcDbHatch"),                        dumpAcDbHatch,
    _T("AcDbMText"),                        dumpAcDbMText,
    _T("AcDbPoint"),                        dumpAcDbPoint,
    _T("AcDbRegion"),                       dumpAcDbRegion,
    _T("AcDbShape"),                        dumpAcDbShape,
    _T("AcDbSolid"),                        dumpAcDbSolid,
    _T("AcDbText"),                         dumpAcDbText,
    _T("AcDb2dVertex"),                     dumpAcDb2dVertex,
    _T("AcDb3dPolylineVertex"),             dumpAcDb3dPolylineVertex,
};

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

#define nEntNames ELEMENTS(EntityList)

void dumpEntity(AcDbEntity *pEnt, TiXmlElement* element)
{
    const TCHAR *p = pEnt->isA()->name();

    for (int i = 0; i < nEntNames; i++)
    {
        if (_tcscmp(p, EntityList[i].pName) == 0)
        {
            (*EntityList[i].func)(pEnt, element);
            break;
        }
    }
}

void dumpAcDb2dPolyline(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tPolyline = new TiXmlElement("Polyline2D");
    
    AcDb2dPolyline *pAcDb2dPolyline = (AcDb2dPolyline *)pEnt;

    AcDb::Poly2dType type = pAcDb2dPolyline->polyType();
    double defaultStartWidth = pAcDb2dPolyline->defaultStartWidth();
    double defaultEndWidth = pAcDb2dPolyline->defaultEndWidth();
    double thickness = pAcDb2dPolyline->thickness();
    double elevation = pAcDb2dPolyline->elevation();
    AcGeVector3d normal = pAcDb2dPolyline->normal();
    AcGePoint3d vertexPosition;
    pAcDb2dPolyline->vertexPosition(vertexPosition);

    static TCHAR *szPolyType[] = { _T("k2dSimplePoly"), _T("k2dFitCurvePoly"), _T("k2dQuadSplinePoly"), _T("k2dCubicSplinePoly") };

    XMLGeometryWriter::dumpText(tPolyline, ws2s(szPolyType[type]), "Type");
    XMLGeometryWriter::dumpDouble(tPolyline, defaultStartWidth, "DefaultStartWidth");
    XMLGeometryWriter::dumpDouble(tPolyline, defaultEndWidth, "DefaultEndWidth");
    XMLGeometryWriter::dumpDouble(tPolyline, thickness, "Thickness");
    XMLGeometryWriter::dumpDouble(tPolyline, elevation, "Elevation");
    XMLGeometryWriter::dumpVector(tPolyline, normal, "Normal");
    XMLGeometryWriter::dumpPoint(tPolyline, vertexPosition, "VertexPosition");

    AcDbObjectIterator* pIter = pAcDb2dPolyline->vertexIterator();
    TiXmlElement* tConnectionPoints = new TiXmlElement("Connection_Points");
    while (!pIter->done())
    {
        AcDbObjectId objId = pIter->objectId();
        AcDb2dVertex *pVert;
        if (pAcDb2dPolyline->openVertex(pVert, objId, AcDb::kForRead) == Acad::eOk)
        {
            dumpEntity(pVert, tConnectionPoints);
            pVert->close();
        }
        pIter->step();
    }
    tPolyline->LinkEndChild(tConnectionPoints);
    delete pIter;

    element->LinkEndChild(tPolyline);
}

void dumpAcDb3dPolyline(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement * tPolyLineElement = new TiXmlElement("Line");

    AcDb3dPolyline *pAcDb3dPolyline = (AcDb3dPolyline *)pEnt;

    AcDb::Poly3dType type = pAcDb3dPolyline->polyType();
    static TCHAR *szPolyType[] = { _T("k3dSimplePoly"),
                                  _T("k3dQuadSplinePoly"),
                                  _T("k3dCubicSplinePoly") };

    TCHAR* polyType = szPolyType[type];

    XMLGeometryWriter::dumpText(tPolyLineElement, ws2s(polyType), "Type");
    
    AcDbObjectIterator* pIter = pAcDb3dPolyline->vertexIterator();
    TiXmlElement* tConnectionPoints = new TiXmlElement("Connection_Points");
    while (!pIter->done())
    {
        AcDbObjectId objId = pIter->objectId();
        AcDb3dPolylineVertex *pVert;
        if (pAcDb3dPolyline->openVertex(pVert, objId, AcDb::kForRead)
            == Acad::eOk)
        {
            dumpEntity(pVert, tConnectionPoints);
            pVert->close();
        }
        pIter->step();
    }
    tPolyLineElement->LinkEndChild(tConnectionPoints);
    delete pIter;

    element->LinkEndChild(tPolyLineElement);
}

void dumpAcDbArc(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tArcElement = new TiXmlElement("Arc");

    AcDbArc *pAcDbArc = (AcDbArc *)pEnt;

    double radius = pAcDbArc->radius();
    AcGePoint3d center = pAcDbArc->center();
    double startAngle = pAcDbArc->startAngle();
    double endAngle = pAcDbArc->endAngle();
    double thickness = pAcDbArc->thickness();
    AcGeVector3d normal = pAcDbArc->normal();

    XMLGeometryWriter::dumpDouble(tArcElement, radius, "Radius");
    XMLGeometryWriter::dumpPoint(tArcElement, center, "Center");
    XMLGeometryWriter::dumpDouble(tArcElement, startAngle, "StartAngle");
    XMLGeometryWriter::dumpDouble(tArcElement, endAngle, "EndAngle");
    XMLGeometryWriter::dumpDouble(tArcElement, thickness, "Thickness");
    XMLGeometryWriter::dumpVector(tArcElement, normal, "Normal");

    pAcDbArc->close();

    element->LinkEndChild(tArcElement);
}

void dumpAcDbCircle(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tCircle = new TiXmlElement("Circle");
    
    AcDbCircle *pAcDbCircle = (AcDbCircle *)pEnt;

    double radius = pAcDbCircle->radius();
    AcGePoint3d center = pAcDbCircle->center();
    double thickness = pAcDbCircle->thickness();
    AcGeVector3d normal = pAcDbCircle->normal();

    XMLGeometryWriter::dumpDouble(tCircle, radius, "Radius");
    XMLGeometryWriter::dumpPoint(tCircle, center, "Center");
    XMLGeometryWriter::dumpDouble(tCircle, thickness, "Thickness");
    XMLGeometryWriter::dumpVector(tCircle, normal, "Normal");

    element->LinkEndChild(tCircle);
}

void dumpAcDbEllipse(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tEllipse = new TiXmlElement("Ellipse");

    AcDbEllipse *pAcDbEllipse = (AcDbEllipse *)pEnt;

    AcGeVector3d normal = pAcDbEllipse->normal();
    AcGeVector3d majorAxis = pAcDbEllipse->majorAxis();
    AcGeVector3d minorAxis = pAcDbEllipse->minorAxis();
    AcGePoint3d center = pAcDbEllipse->center();
    double startAngle = pAcDbEllipse->startAngle();
    double endAngle = pAcDbEllipse->endAngle();
    double radiusRatio = pAcDbEllipse->radiusRatio();

    XMLGeometryWriter::dumpVector(tEllipse, normal, "Normal");
    XMLGeometryWriter::dumpVector(tEllipse, majorAxis, "MajorAxis");
    XMLGeometryWriter::dumpVector(tEllipse, minorAxis, "MinorAxis");
    XMLGeometryWriter::dumpPoint(tEllipse, center, "Center");
    
    if (!pAcDbEllipse->isClosed())
    {
        startAngle = startAngle / PIOVER180;
        endAngle = endAngle / PIOVER180;
        XMLGeometryWriter::dumpDouble(tEllipse, startAngle, "StartAngle");
        XMLGeometryWriter::dumpDouble(tEllipse, endAngle, "EndAngle");
    }

    XMLGeometryWriter::dumpDouble(tEllipse, radiusRatio, "RadiusRatio");

    element->LinkEndChild(tEllipse);
}

void dumpAcDbLeader(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tLeader = new TiXmlElement("Leader");
    
    AcDbLeader *pAcDbLeader = (AcDbLeader *)pEnt;

    AcGeVector3d normal = pAcDbLeader->normal();
    Adesk::Boolean hasArrowHead = pAcDbLeader->hasArrowHead();
    Adesk::Boolean hasHookLine = pAcDbLeader->hasHookLine();
    Adesk::Boolean isSplined = pAcDbLeader->isSplined();

    XMLGeometryWriter::dumpVector(tLeader, normal, "Normal");
    XMLGeometryWriter::dumpBoolean(tLeader, hasArrowHead, "HasArrowHead");
    XMLGeometryWriter::dumpBoolean(tLeader, hasHookLine, "HasHookLine");
    XMLGeometryWriter::dumpBoolean(tLeader, isSplined, "IsSplined");

    int numVertices = pAcDbLeader->numVertices();
    TiXmlElement* tVertices = new TiXmlElement("Vertices");
    for (int i = 0; i < numVertices; i++)
    {
        AcGePoint3d pt = pAcDbLeader->vertexAt(i);
        XMLGeometryWriter::dumpPoint(tVertices, pt, "Vertex");
    }
    tLeader->LinkEndChild(tVertices);

    element->LinkEndChild(tLeader);
}

void dumpAcDbLine(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement * tLineElement = new TiXmlElement("Line");
    
    AcDbLine *pAcDbLine = (AcDbLine *)pEnt;

    AcGePoint3d start = pAcDbLine->startPoint();
    AcGePoint3d end = pAcDbLine->endPoint();
    AcGeVector3d normal = pAcDbLine->normal();
    double thickness = pAcDbLine->thickness();

    XMLGeometryWriter::dumpPoint(tLineElement, start, "StartPoint");
    XMLGeometryWriter::dumpPoint(tLineElement, end, "EndPoint");
    XMLGeometryWriter::dumpVector(tLineElement, normal, "Normal");
    XMLGeometryWriter::dumpDouble(tLineElement, thickness, "Thickness");
    
    element->LinkEndChild(tLineElement);
}

void dumpAcDbPolyline(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tPolyline = new TiXmlElement("Polyline");
    
    AcDbPolyline *pAcDbPolyline = (AcDbPolyline *)pEnt;

    Adesk::Boolean isOnlyLines = pAcDbPolyline->isOnlyLines();
    Adesk::Boolean hasPLineGen = pAcDbPolyline->hasPlinegen();

    XMLGeometryWriter::dumpBoolean(tPolyline, isOnlyLines, "IsOnlyLines");
    XMLGeometryWriter::dumpBoolean(tPolyline, hasPLineGen, "HasPLineGen");

    double elevation = pAcDbPolyline->elevation();
    double thickness = pAcDbPolyline->thickness();
    AcGeVector3d normal = pAcDbPolyline->normal();

    XMLGeometryWriter::dumpDouble(tPolyline, elevation, "Elevation");
    XMLGeometryWriter::dumpDouble(tPolyline, thickness, "Thickness");
    XMLGeometryWriter::dumpVector(tPolyline, normal, "Normal");

    double constantWidth;

    if ( pAcDbPolyline->getConstantWidth(constantWidth) == Acad::eOk )
        XMLGeometryWriter::dumpDouble(tPolyline, constantWidth, "ConstantWidth");
        
    int numVerts = pAcDbPolyline->numVerts();
    if (numVerts < 2)
        return;

    int segmentType;
    double  bulge;
    double  startWidth, endWidth;
    AcGePoint3d position;
    static TCHAR *szSegType[] = { _T("kLine"), _T("kArc"), _T("kCoincident"), _T("kPoint"), _T("kEmpty") };

    TiXmlElement* tVertices = new TiXmlElement("Vertices");

    for ( int i = 0; i < numVerts; i++ ) 
    {
        TiXmlElement* tSegment = new TiXmlElement("Segment");
        
        if ( pAcDbPolyline->getPointAt(i, position) == Acad::eOk )
            XMLGeometryWriter::dumpPoint(tSegment, position, "Position");
        if (pAcDbPolyline->getBulgeAt(i, bulge) == Acad::eOk)
            XMLGeometryWriter::dumpDouble(tSegment, bulge, "Bulge");
        if ( pAcDbPolyline->getWidthsAt(i, startWidth, endWidth) == Acad::eOk ) 
        {
            XMLGeometryWriter::dumpDouble(tSegment, startWidth, "StartWidth");
            XMLGeometryWriter::dumpDouble(tSegment, endWidth, "EndWidth");
        }

        segmentType = pAcDbPolyline->segType(i);
        XMLGeometryWriter::dumpText(tSegment, ws2s(szSegType[segmentType]), "SegmentType");

        tVertices->LinkEndChild(tSegment);
    }

    tPolyline->LinkEndChild(tVertices);

    element->LinkEndChild(tPolyline);
}

void dumpAcDbRay(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tRay = new TiXmlElement("Ray");
    
    AcDbRay *pAcDbRay = (AcDbRay *)pEnt;

    AcGePoint3d basePoint = pAcDbRay->basePoint();
    AcGeVector3d unitDir = pAcDbRay->unitDir();

    XMLGeometryWriter::dumpPoint(tRay, basePoint, "BasePoint");
    XMLGeometryWriter::dumpVector(tRay, unitDir, "UnitDirection");

    element->LinkEndChild(tRay);
}

void dumpAcDbSpline(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tSpline = new TiXmlElement("Spline");
    
    AcDbSpline *pAcDbSpline = (AcDbSpline *)pEnt;

    Adesk::Boolean isNull = pAcDbSpline->isNull();
    Adesk::Boolean isRational = pAcDbSpline->isRational();
    Adesk::Boolean isPeriodic = pAcDbSpline->isPeriodic();
    Adesk::Boolean isClosed = pAcDbSpline->isClosed();
    int degree = pAcDbSpline->degree();
    
    XMLGeometryWriter::dumpBoolean(tSpline, isNull, "IsNull");
    XMLGeometryWriter::dumpBoolean(tSpline, isRational, "IsRational");
    XMLGeometryWriter::dumpBoolean(tSpline, isPeriodic, "IsPeriodic");
    XMLGeometryWriter::dumpBoolean(tSpline, isClosed, "IsClosed");
    XMLGeometryWriter::dumpInt(tSpline, degree, "Degree");

    if (pAcDbSpline->hasFitData())
    {
        double fitTol = pAcDbSpline->fitTolerance();
        AcGeVector3d startTangent, endTangent;
        pAcDbSpline->getFitTangents(startTangent, endTangent);
        int nFitPoints = pAcDbSpline->numFitPoints();
        TiXmlElement* tFitPts = new TiXmlElement("FitPoints");
        for (int i = 0; i < nFitPoints; i++)
        {
            AcGePoint3d pt;
            pAcDbSpline->getFitPointAt(i, pt);
            XMLGeometryWriter::dumpPoint(tFitPts, pt, "Point");
        }
        tSpline->LinkEndChild(tFitPts);

        XMLGeometryWriter::dumpDouble(tSpline, fitTol, "FitTolerance");
        XMLGeometryWriter::dumpVector(tSpline, startTangent, "StartTangent");
        XMLGeometryWriter::dumpVector(tSpline, endTangent, "EndTangent");
    }

    int nControlPoints = pAcDbSpline->numControlPoints();
    TiXmlElement* tControlPts = new TiXmlElement("ControlPoints");
    for (int i = 0; i < nControlPoints; i++)
    {
        AcGePoint3d pt;
        pAcDbSpline->getControlPointAt(i, pt);
        double weight = pAcDbSpline->weightAt(i);
        XMLGeometryWriter::dumpPoint(tControlPts, pt, "Point");
        XMLGeometryWriter::dumpDouble(tControlPts, weight, "Weight");
    }
    tSpline->LinkEndChild(tControlPts);

    TiXmlElement* tNurbsData = new TiXmlElement("NurbsData");
    AcGePoint3dArray controlPoints;
    AcGeDoubleArray knots;
    AcGeDoubleArray weights;
    double controlPtTol;
    double knotTol;
    pAcDbSpline->getNurbsData(degree, isRational, isClosed, isPeriodic, controlPoints, knots, weights, controlPtTol, knotTol);

    XMLGeometryWriter::dumpBoolean(tNurbsData, isRational, "IsRational");
    XMLGeometryWriter::dumpBoolean(tNurbsData, isClosed, "IsClosed");
    XMLGeometryWriter::dumpBoolean(tNurbsData, isPeriodic, "IsPeriodic");
    XMLGeometryWriter::dumpInt(tNurbsData, degree, "Degree");
    XMLGeometryWriter::dumpDouble(tNurbsData, controlPtTol, "ControlPointsTolerance");
    XMLGeometryWriter::dumpDouble(tNurbsData, knotTol, "KnotTolerance");
    
    TiXmlElement* tControlPtsNurbs = new TiXmlElement("ControlPoints");
    for (int i = 0; i < controlPoints.length(); i++)
    {
        XMLGeometryWriter::dumpPoint(tControlPtsNurbs, controlPoints[i], "Point");
    }
    tNurbsData->LinkEndChild(tControlPtsNurbs);

    TiXmlElement* tknotsNurbs = new TiXmlElement("Knots");
    for (int i = 0; i < knots.length(); i++)
    {
        XMLGeometryWriter::dumpDouble(tknotsNurbs, knots[i], "Knot");
    }
    tNurbsData->LinkEndChild(tknotsNurbs);

    TiXmlElement* tWeights = new TiXmlElement("Weights");
    for (int i = 0; i < weights.length(); i++)
    {
        XMLGeometryWriter::dumpDouble(tWeights, weights[i], "Weight");
    }
    tNurbsData->LinkEndChild(tWeights);

    tSpline->LinkEndChild(tNurbsData);

    element->LinkEndChild(tSpline);
}

void dumpAcDbXline(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tXLine = new TiXmlElement("XLine");
    
    AcDbXline *pAcDbXline = (AcDbXline *)pEnt;

    AcGePoint3d basePoint = pAcDbXline->basePoint();
    AcGeVector3d unitDir = pAcDbXline->unitDir();

    XMLGeometryWriter::dumpPoint(tXLine, basePoint, "BasePoint");
    XMLGeometryWriter::dumpVector(tXLine, unitDir, "UnitDirection");

    element->LinkEndChild(tXLine);
}

void dumpAcDbDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    AcDbDimension *pAcDbDimension = (AcDbDimension *)pEnt;

    ++dimensionCount;

    Adesk::Boolean   isUsingDefaultTextPosition = pAcDbDimension->isUsingDefaultTextPosition();
    AcGePoint3d      textPosition = pAcDbDimension->textPosition();
    AcGePoint3d      dimBlockPosition = pAcDbDimension->dimBlockPosition();
    AcGeVector3d     normal = pAcDbDimension->normal();
    TCHAR*           dimensionText = pAcDbDimension->dimensionText(); 
    double           textRotation = pAcDbDimension->textRotation();
    double           horizontalRotation = pAcDbDimension->horizontalRotation();
    
    XMLGeometryWriter::dumpBoolean(element, isUsingDefaultTextPosition, "Using_Default_TextPosition");
    XMLGeometryWriter::dumpPoint(element, textPosition, "TextPosition");
    XMLGeometryWriter::dumpPoint(element, dimBlockPosition, "BlockPOsition");
    XMLGeometryWriter::dumpVector(element, normal, "Normal");
    XMLGeometryWriter::dumpText(element, ws2s(dimensionText), "DimensionText");
    XMLGeometryWriter::dumpDouble(element, textRotation, "TextRotationAngle");
    XMLGeometryWriter::dumpDouble(element, horizontalRotation, "HorizontalRotationAngle");

    acdbFree(dimensionText);
}

void dumpAcDb2LineAngularDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tAngularDimension = new TiXmlElement("Line2AngularDimension");
    
    AcDb2LineAngularDimension *pAcDbDimension = (AcDb2LineAngularDimension *)pEnt;

    dumpAcDbDimension(pEnt, tAngularDimension);

    AcGePoint3d arcPoint = pAcDbDimension->arcPoint(); 
    AcGePoint3d xLine1Start = pAcDbDimension->xLine1Start(); 
    AcGePoint3d xLine1End = pAcDbDimension->xLine1End(); 
    AcGePoint3d xLine2Start = pAcDbDimension->xLine2Start(); 
    AcGePoint3d xLine2End = pAcDbDimension->xLine2End(); 

    XMLGeometryWriter::dumpPoint(tAngularDimension, arcPoint, "ArcPoint");
    XMLGeometryWriter::dumpPoint(tAngularDimension, xLine1Start, "xLine1Start");
    XMLGeometryWriter::dumpPoint(tAngularDimension, xLine1End, "xLine1End");
    XMLGeometryWriter::dumpPoint(tAngularDimension, xLine2Start, "xLine2Start ");
    XMLGeometryWriter::dumpPoint(tAngularDimension, xLine2End, "xLine2End");

    element->LinkEndChild(tAngularDimension);
}

void dumpAcDb3PointAngularDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tAngularDimension = new TiXmlElement("Point3AngularDimension");
    
    AcDb3PointAngularDimension *pAcDbDimension = (AcDb3PointAngularDimension *)pEnt;
    
    dumpAcDbDimension(pEnt, tAngularDimension);

    AcGePoint3d arcPoint = pAcDbDimension->arcPoint(); 
    AcGePoint3d xLine1Point = pAcDbDimension->xLine1Point(); 
    AcGePoint3d xLine2Point = pAcDbDimension->xLine2Point(); 
    AcGePoint3d centerPoint = pAcDbDimension->centerPoint(); 

    XMLGeometryWriter::dumpPoint(tAngularDimension, xLine1Point, "xLine1Point");
    XMLGeometryWriter::dumpPoint(tAngularDimension, xLine2Point, "xLine2Point");
    XMLGeometryWriter::dumpPoint(tAngularDimension, arcPoint, "ArcPoint");
    XMLGeometryWriter::dumpPoint(tAngularDimension, centerPoint, "CenterPoint");

    element->LinkEndChild(tAngularDimension);
}

void dumpAcDbAlignedDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tAlignedDimension = new TiXmlElement("AlignedDimension");
    
    AcDbAlignedDimension *pAcDbDimension = (AcDbAlignedDimension *)pEnt;
    
    dumpAcDbDimension(pEnt, tAlignedDimension);

    AcGePoint3d xLine1Point = pAcDbDimension->xLine1Point(); 
    AcGePoint3d xLine2Point = pAcDbDimension->xLine2Point(); 
    AcGePoint3d dimLinePoint = pAcDbDimension->dimLinePoint(); 
    double oblique = pAcDbDimension->oblique();

    XMLGeometryWriter::dumpPoint(tAlignedDimension, xLine1Point, "xLine1Point");
    XMLGeometryWriter::dumpPoint(tAlignedDimension, xLine2Point, "xLine2Point");
    XMLGeometryWriter::dumpPoint(tAlignedDimension, dimLinePoint, "DimensionLinePoint");
    XMLGeometryWriter::dumpDouble(tAlignedDimension, oblique, "Obliquing_Angle");
    
    element->LinkEndChild(tAlignedDimension);
}

void dumpAcDbDiametricDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tDiametricDimension = new TiXmlElement("DiametricDimension");
    
    AcDbDiametricDimension *pAcDbDimension = (AcDbDiametricDimension *)pEnt;

    dumpAcDbDimension(pEnt, tDiametricDimension);

    double leaderLength = pAcDbDimension->leaderLength();
    AcGePoint3d chordPoint = pAcDbDimension->chordPoint();
    AcGePoint3d farChordPoint = pAcDbDimension->farChordPoint();

    XMLGeometryWriter::dumpPoint(tDiametricDimension, farChordPoint, "FarChordPoint");
    XMLGeometryWriter::dumpPoint(tDiametricDimension, chordPoint, "ChordPoint");
    XMLGeometryWriter::dumpDouble(tDiametricDimension, leaderLength, "LeaderLength");

    element->LinkEndChild(tDiametricDimension);
}

void dumpAcDbOrdinateDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tOrdinateDimension = new TiXmlElement("OrdinateDimension");
    
    AcDbOrdinateDimension *pAcDbDimension = (AcDbOrdinateDimension *)pEnt;

    dumpAcDbDimension(pEnt, tOrdinateDimension);

    Adesk::Boolean isUsingXAxis = pAcDbDimension->isUsingXAxis();
    Adesk::Boolean isUsingYAxis = pAcDbDimension->isUsingYAxis();
    AcGePoint3d origin = pAcDbDimension->origin();
    AcGePoint3d definingPoint = pAcDbDimension->definingPoint(); 
    AcGePoint3d leaderEndPoint = pAcDbDimension->leaderEndPoint();

    XMLGeometryWriter::dumpBoolean(tOrdinateDimension, isUsingXAxis, "IsUsingXAxis");
    XMLGeometryWriter::dumpBoolean(tOrdinateDimension, isUsingYAxis, "IsUsingYAxis");
    XMLGeometryWriter::dumpPoint(tOrdinateDimension, origin, "Origin");
    XMLGeometryWriter::dumpPoint(tOrdinateDimension, definingPoint, "DefiningPoint");
    XMLGeometryWriter::dumpPoint(tOrdinateDimension, leaderEndPoint, "LeaderEndPoint");

    element->LinkEndChild(tOrdinateDimension);
}

void dumpAcDbRadialDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tRadialDimension = new TiXmlElement("RadialDimension");
    
    AcDbRadialDimension *pAcDbDimension = (AcDbRadialDimension *)pEnt;

    dumpAcDbDimension(pEnt, tRadialDimension);

    double leaderLength = pAcDbDimension->leaderLength();
    AcGePoint3d center = pAcDbDimension->center();
    AcGePoint3d chordPoint = pAcDbDimension->chordPoint();

    XMLGeometryWriter::dumpPoint(tRadialDimension, center, "CenterPoint");
    XMLGeometryWriter::dumpPoint(tRadialDimension, chordPoint, "ChordPoint");
    XMLGeometryWriter::dumpDouble(tRadialDimension, leaderLength, "LeaderLength");

    element->LinkEndChild(tRadialDimension);
}

void dumpAcDbRotatedDimension(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tRotatedDimension = new TiXmlElement("RotatedDimension");
    
    dumpAcDbDimension(pEnt, tRotatedDimension);
    
    AcDbRotatedDimension *pAcDbDimension = (AcDbRotatedDimension *)pEnt;
    
    AcGePoint3d xLine1Point = pAcDbDimension->xLine1Point(); 
    AcGePoint3d xLine2Point = pAcDbDimension->xLine2Point(); 
    AcGePoint3d dimLinePoint = pAcDbDimension->dimLinePoint(); 
    double oblique = pAcDbDimension->oblique();
    double rotation = pAcDbDimension->rotation();

    XMLGeometryWriter::dumpPoint(tRotatedDimension, xLine1Point, "xLine1Point");
    XMLGeometryWriter::dumpPoint(tRotatedDimension, xLine2Point, "xLine2Point");
    XMLGeometryWriter::dumpPoint(tRotatedDimension, dimLinePoint, "DimensionLinePoint");
    XMLGeometryWriter::dumpDouble(tRotatedDimension, oblique, "Obliquing_Angle");
    XMLGeometryWriter::dumpDouble(tRotatedDimension, rotation, "Rotation_Angle");
    
    element->LinkEndChild(tRotatedDimension);
}

void dumpAcDbFcf(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tFcf = new TiXmlElement("FCF");
    
    AcDbFcf *pAcDbFcf = (AcDbFcf *)pEnt;

    ++dimensionCount;
    
    TiXmlElement* tFrameText = new TiXmlElement("ToleranceFrame");
    while (1)
    {
        int line = 0;
        TCHAR *pText = pAcDbFcf->text(line++);
        if (!pText)
            break;
        XMLGeometryWriter::dumpText(tFrameText, ws2s(pText), "FrameText");
        acdbFree((void *)pText);
    }
    tFcf->LinkEndChild(tFrameText);

    AcGePoint3d location = pAcDbFcf->location();
    AcGeVector3d normal = pAcDbFcf->normal();
    AcGeVector3d direction = pAcDbFcf->direction();

    AcDbHardPointerId dimensionStyle = pAcDbFcf->dimensionStyle();
    AcDbDimStyleTableRecord *pRecord;
    if (acdbOpenObject(pRecord, dimensionStyle, AcDb::kForRead) == Acad::eOk)
    {
        TCHAR *pName;
        if (pRecord->getName(pName) == Acad::eOk)
        {
            XMLGeometryWriter::dumpText(tFcf, ws2s(pName), "DimensionStyle");
            acdbFree(pName);
        }
        pRecord->close();
    }

    XMLGeometryWriter::dumpPoint(tFcf, location, "Location");
    XMLGeometryWriter::dumpVector(tFcf, normal, "Normal");
    XMLGeometryWriter::dumpVector(tFcf, direction, "Direction");

    element->LinkEndChild(tFcf);
}

void dumpAcDbHatch(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tHatch = new TiXmlElement("Hatch");
    
    AcDbHatch *pAcDbHatch = (AcDbHatch *)pEnt;

    Adesk::Boolean isAssociative = pAcDbHatch->associative();
    XMLGeometryWriter::dumpBoolean(tHatch, isAssociative, "IsAssociative");

    static TCHAR *hatchStyle[] = { _T("kNormal"), _T("kOuter"), _T("kIgnore") };
    XMLGeometryWriter::dumpText(tHatch, ws2s(hatchStyle[pAcDbHatch->hatchStyle()]), "HatchStyle");

    double elevation = pAcDbHatch->elevation();
    XMLGeometryWriter::dumpDouble(tHatch, elevation, "Elevation");

    AcGeVector3d normal = pAcDbHatch->normal();
    XMLGeometryWriter::dumpVector(tHatch, normal, "Normal");

    static TCHAR *hatchPattern[] = { _T("kUserDefined"), _T("kPreDefined"), _T("kCustomDefined") };
    XMLGeometryWriter::dumpText(tHatch, ws2s(hatchPattern[pAcDbHatch->patternType()]), "HatchPattern");

    double angle = pAcDbHatch->patternAngle();
    double space = pAcDbHatch->patternSpace();
    double scale = pAcDbHatch->patternScale();
    Adesk::Boolean isDoublePattern = pAcDbHatch->patternDouble();

    XMLGeometryWriter::dumpDouble(tHatch, angle, "PatternAngle");
    XMLGeometryWriter::dumpDouble(tHatch, space, "PatternSpace");
    XMLGeometryWriter::dumpDouble(tHatch, scale, "PatternScale");
    XMLGeometryWriter::dumpBoolean(tHatch, isDoublePattern, "IsDoublePattern");
    
    TiXmlElement* tLoops = new TiXmlElement("Loops");
    int NumLoops = pAcDbHatch->numLoops();
    for ( int i = 0; i < NumLoops; i++ ) {
        int type = pAcDbHatch->loopTypeAt(i);
        XMLGeometryWriter::dumpInt(tLoops, type, "Type");
    }
    tHatch->LinkEndChild(tLoops);

    int numPatDefs = pAcDbHatch->numPatternDefinitions();
    double  baseX;
    double  baseY;
    double  offsetX;
    double  offsetY;
    AcGeDoubleArray dashes;

    TiXmlElement* tPatternDefinitions = new TiXmlElement("Pattern_Definitions");
    for ( int i = 0; i < numPatDefs; i++ ) 
    {
        TiXmlElement* tDefinition = new TiXmlElement("Definition");
        if ( pAcDbHatch->getPatternDefinitionAt(i, angle, baseX, baseY, offsetX, offsetY, dashes) == Acad::eOk ) 
        {
            XMLGeometryWriter::dumpDouble(tDefinition, angle, "Angle");
            XMLGeometryWriter::dumpDouble(tDefinition, baseX, "BaseX");
            XMLGeometryWriter::dumpDouble(tDefinition, baseY, "BaseY");
            XMLGeometryWriter::dumpDouble(tDefinition, offsetX, "OffsetX");
            XMLGeometryWriter::dumpDouble(tDefinition, offsetY, "OffsetY");
        }
        tPatternDefinitions->LinkEndChild(tDefinition);
    }
    tHatch->LinkEndChild(tPatternDefinitions);

    element->LinkEndChild(tHatch);
}

void dumpAcDbMText(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tMText = new TiXmlElement("MText");
    
    AcDbMText *pAcDbMText = (AcDbMText *)pEnt;

    AcGePoint3d location = pAcDbMText->location();
    AcGeVector3d normal = pAcDbMText->normal();
    AcGeVector3d direction = pAcDbMText->direction();
    double rotation = pAcDbMText->rotation();
    double width = pAcDbMText->width();
    AcDbObjectId textStyle = pAcDbMText->textStyle();
    double textHeight = pAcDbMText->textHeight();
    AcDbMText::AttachmentPoint attachment = pAcDbMText->attachment();
    AcDbMText::FlowDirection flowDirection = pAcDbMText->flowDirection();
    TCHAR *contents = pAcDbMText->contents();
    double actualHeight = pAcDbMText->actualHeight();
    double actualWidth = pAcDbMText->actualWidth();
    AcGePoint3dArray boundingPoints;
    pAcDbMText->getBoundingPoints(boundingPoints);

    XMLGeometryWriter::dumpPoint(tMText, location, "Location");
    XMLGeometryWriter::dumpVector(tMText, normal, "Normal");
    XMLGeometryWriter::dumpVector(tMText, direction, "Direction");
    XMLGeometryWriter::dumpDouble(tMText, rotation, "Rotation");
    XMLGeometryWriter::dumpDouble(tMText, width, "Width");

    AcDbTextStyleTableRecord *pRecord = 0;
    acdbOpenObject(pRecord, textStyle, AcDb::kForRead);
    if (pRecord)
    {
        TCHAR *pName;
        if (pRecord->getName(pName) == Acad::eOk)
        {
            XMLGeometryWriter::dumpText(tMText, ws2s(pName), "TextStyle");
            acdbFree(pName);
        }
        pRecord->close();
    }

    XMLGeometryWriter::dumpDouble(tMText, textHeight, "TextHeight");
    XMLGeometryWriter::dumpDouble(tMText, actualWidth, "ActualWidth");
    XMLGeometryWriter::dumpDouble(tMText, actualHeight, "ActualHeight");
    
    TiXmlElement* tBoundingPoints = new TiXmlElement("BoundingPoints");
    XMLGeometryWriter::dumpPoint(tMText, boundingPoints[0], "TopLeft");
    XMLGeometryWriter::dumpPoint(tMText, boundingPoints[1], "TopRight");
    XMLGeometryWriter::dumpPoint(tMText, boundingPoints[2], "BottomLeft");
    XMLGeometryWriter::dumpPoint(tMText, boundingPoints[3], "BottomRight");
    tMText->LinkEndChild(tBoundingPoints);

    std::string attachmentPt = "";
    switch (attachment)
    {
    case AcDbMText::kTopLeft:
        attachmentPt = "kTopLeft";
        break;
    case AcDbMText::kTopCenter:
        attachmentPt = "kTopCenter";
        break;
    case AcDbMText::kTopRight:
        attachmentPt = "kTopRight";
        break;
    case AcDbMText::kMiddleLeft:
        attachmentPt = "kMiddleLeft";
        break;
    case AcDbMText::kMiddleCenter:
        attachmentPt = "kMiddleCenter";
        break;
    case AcDbMText::kMiddleRight:
        attachmentPt = "kMiddleRight";
        break;
    case AcDbMText::kBottomLeft:
        attachmentPt = "kBottomLeft";
        break;
    case AcDbMText::kBottomCenter:
        attachmentPt = "kBottomCenter";
        break;
    case AcDbMText::kBottomRight:
        attachmentPt = "kBottomRight";
        break;
    }

    XMLGeometryWriter::dumpText(tMText, attachmentPt, "AttachmentPoint");

    std::string flowDirectionStr = "";
    switch (flowDirection)
    {
    case AcDbMText::kLtoR:
        flowDirectionStr = "kLtoR";
        break;
    case AcDbMText::kRtoL:
        flowDirectionStr = "kRtoL";
        break;
    case AcDbMText::kTtoB:
        flowDirectionStr = "kTtoB";
        break;
    case AcDbMText::kBtoT:
        flowDirectionStr = "kBtoT";
        break;
    case AcDbMText::kByStyle:
        flowDirectionStr = "kByStyle";
        break;
    }

    XMLGeometryWriter::dumpText(tMText, flowDirectionStr, "FlowDirection");
    XMLGeometryWriter::dumpText(tMText, ws2s(contents), "TextString");

    acdbFree(contents);

    element->LinkEndChild(tMText);
}

void dumpAcDbPoint(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tPoint = new TiXmlElement("Point");
    
    AcDbPoint *pAcDbPoint = (AcDbPoint *)pEnt;

    AcGePoint3d position = pAcDbPoint->position();
    double thickness = pAcDbPoint->thickness();
    AcGeVector3d normal = pAcDbPoint->normal();
    double ecsRotation = pAcDbPoint->ecsRotation();

    XMLGeometryWriter::dumpPoint(tPoint, position, "Position");
    XMLGeometryWriter::dumpDouble(tPoint, thickness, "Thickness");
    XMLGeometryWriter::dumpVector(tPoint, normal, "Normal");
    XMLGeometryWriter::dumpDouble(tPoint, ecsRotation, "Rotation");

    element->LinkEndChild(tPoint);
}

void dumpAcDbRegion(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tRegion = new TiXmlElement("Region");
    
    AcDbRegion *pAcDbRegion = (AcDbRegion *)pEnt;

    double perimeter, regionArea;
    AcGePlane regionPlane;
    AcGeVector3d normal;
    pAcDbRegion->getPerimeter (perimeter);
    pAcDbRegion->getArea(regionArea);
    pAcDbRegion->getPlane(regionPlane);
    pAcDbRegion->getNormal(normal);

    AcGePoint3d planeOrigin;
    AcGeVector3d uAxis;
    AcGeVector3d vAxis;
    regionPlane.get(planeOrigin, uAxis, vAxis);

    XMLGeometryWriter::dumpDouble(tRegion, perimeter, "Perimeter");
    XMLGeometryWriter::dumpDouble(tRegion, regionArea, "Area");
    XMLGeometryWriter::dumpVector(tRegion, normal, "Normal");
    XMLGeometryWriter::dumpPoint(tRegion, planeOrigin, "PlaneOrigin");
    XMLGeometryWriter::dumpVector(tRegion, uAxis, "UAxis");
    XMLGeometryWriter::dumpVector(tRegion, vAxis, "VAxis");

    element->LinkEndChild(tRegion);
}

void dumpAcDbShape(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tShape = new TiXmlElement("Shape");
    
    AcDbShape *pAcDbShape = (AcDbShape *)pEnt;

    AcGePoint3d      position = pAcDbShape->position();
    double           size = pAcDbShape->size();
    TCHAR*           name = pAcDbShape->name();
    double           rotation = pAcDbShape->rotation();
    double           widthFactor = pAcDbShape->widthFactor();
    double           oblique = pAcDbShape->oblique();
    double           thickness = pAcDbShape->thickness();
    AcGeVector3d     normal = pAcDbShape->normal();

    XMLGeometryWriter::dumpPoint(tShape, position, "Position");
    XMLGeometryWriter::dumpDouble(tShape, size, "Size");
    XMLGeometryWriter::dumpDouble(tShape, rotation, "Rotation");
    XMLGeometryWriter::dumpDouble(tShape, widthFactor, "WidthFactor");
    XMLGeometryWriter::dumpDouble(tShape, oblique, "obliquingAngle");
    XMLGeometryWriter::dumpDouble(tShape, thickness, "Thickness");
    XMLGeometryWriter::dumpVector(tShape, normal, "Normal");

    if (name)
    {
        XMLGeometryWriter::dumpText(tShape, ws2s(name), "Name");
        acdbFree(name);
    }
    
    element->LinkEndChild(tShape);
}

void dumpAcDbSolid(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tSolid = new TiXmlElement("Solid");
    
    AcDbSolid *pAcDbSolid = (AcDbSolid *)pEnt;

    double thickness = pAcDbSolid->thickness();
    AcGeVector3d normal = pAcDbSolid->normal();

    XMLGeometryWriter::dumpDouble(tSolid, thickness, "Thickness");
    XMLGeometryWriter::dumpVector(tSolid, normal, "Normal");

    TiXmlElement* tVertices = new TiXmlElement("Vertices");
    for (int i = 0; i < 4; i++)
    {
        AcGePoint3d pt;
        pAcDbSolid->getPointAt(i, pt);
        XMLGeometryWriter::dumpPoint(tVertices, pt, "Vertex");
    }
    tSolid->LinkEndChild(tVertices);

    element->LinkEndChild(tSolid);
}

void dumpAcDbText(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tText = new TiXmlElement("Text");
    
    AcDbText *pAcDbText = (AcDbText *)pEnt;

    AcDbObjectId blockId = pAcDbText->textStyle();
    AcGePoint3d position = pAcDbText->position();
    AcGePoint3d align = pAcDbText->alignmentPoint();
    Adesk::Boolean isDefAlign = pAcDbText->isDefaultAlignment();
    AcGeVector3d normal = pAcDbText->normal();
    double thickness = pAcDbText->thickness();
    double oblique = pAcDbText->oblique();
    double rotation = pAcDbText->rotation();
    double height = pAcDbText->height();
    double widthFactor = pAcDbText->widthFactor();
    TCHAR *pText = pAcDbText->textString();
    Adesk::Boolean isMirrorX = pAcDbText->isMirroredInX();
    Adesk::Boolean isMirrorY = pAcDbText->isMirroredInY();
    AcDb::TextHorzMode horzMode = pAcDbText->horizontalMode();
    AcDb::TextVertMode vertMode = pAcDbText->verticalMode();
    static TCHAR *szHorzMode[] = { _T("kTextLeft"), _T("kTextCenter"), _T("kTextRight"), _T("kTextAlign"), _T("kTextMid"), _T("kTextFit") };
    static TCHAR *szVertMode[] = { _T("kTextBase"), _T("kTextBottom"), _T("kTextVertMid"), _T("kTextTop") };

    AcDbTextStyleTableRecord *pRecord = 0;
    acdbOpenObject(pRecord, blockId, AcDb::kForRead);
    if (pRecord)
    {
        TCHAR *pName;
        if (pRecord->getName(pName) == Acad::eOk)
        {
            XMLGeometryWriter::dumpText(tText, ws2s(pName), "TextStyle");
            acdbFree(pName);
        }
        pRecord->close();
    }

    XMLGeometryWriter::dumpText(tText, ws2s(pText), "TextString");
    XMLGeometryWriter::dumpPoint(tText, position, "Position");
    XMLGeometryWriter::dumpPoint(tText, align, "AlignmentPoint");
    XMLGeometryWriter::dumpVector(tText, normal, "Normal");
    XMLGeometryWriter::dumpDouble(tText, thickness, "Thickness");
    XMLGeometryWriter::dumpDouble(tText, oblique, "ObliquingAngle");
    XMLGeometryWriter::dumpDouble(tText, rotation, "Rotation");
    XMLGeometryWriter::dumpDouble(tText, height, "Height");
    XMLGeometryWriter::dumpDouble(tText, widthFactor, "WidthFactor");
    XMLGeometryWriter::dumpBoolean(tText, isMirrorX, "IsMirroredInX");
    XMLGeometryWriter::dumpBoolean(tText, isMirrorY, "IsMirroredInY");
    XMLGeometryWriter::dumpText(tText, ws2s(szHorzMode[horzMode]), "HorizontalMode");
    XMLGeometryWriter::dumpText(tText, ws2s(szVertMode[vertMode]), "VerticalMode");

    element->LinkEndChild(tText);
}

void dumpAcDb2dVertex(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tVertex2d = new TiXmlElement("Vertex2d");

    AcDb2dVertex *pAcDb2dVertex = (AcDb2dVertex *)pEnt;

    AcGePoint3d position = pAcDb2dVertex->position();
    double startWidth = pAcDb2dVertex->startWidth();
    double endWidth = pAcDb2dVertex->endWidth();
    double bulge = pAcDb2dVertex->bulge();
    Adesk::Boolean isTangentUsed = pAcDb2dVertex->isTangentUsed();
    double tangent = pAcDb2dVertex->tangent();

    XMLGeometryWriter::dumpPoint(tVertex2d, position, "Position");
    XMLGeometryWriter::dumpDouble(tVertex2d, startWidth, "StartWidth");
    XMLGeometryWriter::dumpDouble(tVertex2d, endWidth, "EndWidth");
    XMLGeometryWriter::dumpDouble(tVertex2d, bulge, "Bulge");
    XMLGeometryWriter::dumpBoolean(tVertex2d, isTangentUsed, "IsTangentUsed");
    XMLGeometryWriter::dumpDouble(tVertex2d, tangent, "Tangent");

    element->LinkEndChild(tVertex2d);
}

void dumpAcDb3dPolylineVertex(AcDbEntity *pEnt, TiXmlElement* element)
{
    TiXmlElement* tVertex3d = new TiXmlElement("Vertex3d");
    
    AcDb3dPolylineVertex *pAcDb3dPolylineVertex = (AcDb3dPolylineVertex *)pEnt;

    AcGePoint3d position = pAcDb3dPolylineVertex->position();
    
    XMLGeometryWriter::dumpPoint(tVertex3d, position, "Position");

    element->LinkEndChild(tVertex3d);
}
