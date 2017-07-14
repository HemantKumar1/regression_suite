#pragma once

#include "dbents.h"
#include "tinyxml.h"

class XMLGeometryWriter
{
public:
    static void dumpPoint(TiXmlElement* element, AcGePoint3d& pt, const char* nodeName);
    static void dumpPoint(TiXmlElement* element, AcGePoint2d& pt, const char* nodeName);
    static void dumpVector(TiXmlElement* element, AcGeVector3d& pt, const char* nodeName);
    static void dumpDouble(TiXmlElement* element, double& value, const char* nodeName);
    static void dumpInt(TiXmlElement* element, int& value, const char* nodeName);
    static void dumpText(TiXmlElement* element, std::string& value, const char* nodeName);
    static void dumpBoolean(TiXmlElement* element, Adesk::Boolean& value, const char* nodeName);
};