
#include<string>
#include<tchar.h>
#include<Windows.h>

#include "entitydumper.h"

void XMLGeometryWriter::dumpPoint(TiXmlElement* element, AcGePoint3d& pt, const char* nodeName)
{
    TiXmlElement* ptEle = new TiXmlElement(nodeName);
    {
        TiXmlElement* xPosEle = new TiXmlElement("X");
        xPosEle->LinkEndChild(new TiXmlText(std::to_string(pt.x).c_str()));
        ptEle->LinkEndChild(xPosEle);

        TiXmlElement* yPosEle = new TiXmlElement("Y");
        yPosEle->LinkEndChild(new TiXmlText(std::to_string(pt.y).c_str()));
        ptEle->LinkEndChild(yPosEle);

        TiXmlElement* zPosEle = new TiXmlElement("Z");
        zPosEle->LinkEndChild(new TiXmlText(std::to_string(pt.z).c_str()));
        ptEle->LinkEndChild(zPosEle);
        element->LinkEndChild(ptEle);
    }
}

void XMLGeometryWriter::dumpPoint(TiXmlElement* element, AcGePoint2d& pt, const char* nodeName)
{
    TiXmlElement* ptEle = new TiXmlElement(nodeName);
    {
        TiXmlElement* xPosEle = new TiXmlElement("X");
        xPosEle->LinkEndChild(new TiXmlText(std::to_string(pt.x).c_str()));
        ptEle->LinkEndChild(xPosEle);

        TiXmlElement* yPosEle = new TiXmlElement("Y");
        yPosEle->LinkEndChild(new TiXmlText(std::to_string(pt.y).c_str()));
        ptEle->LinkEndChild(yPosEle);

        element->LinkEndChild(ptEle);
    }
}

void XMLGeometryWriter::dumpVector(TiXmlElement* element, AcGeVector3d& vec, const char* nodeName)
{
    TiXmlElement* vecEle = new TiXmlElement(nodeName);
    {
        TiXmlElement* xPosEle = new TiXmlElement("X");
        xPosEle->LinkEndChild(new TiXmlText(std::to_string(vec.x).c_str()));
        vecEle->LinkEndChild(xPosEle);

        TiXmlElement* yPosEle = new TiXmlElement("Y");
        yPosEle->LinkEndChild(new TiXmlText(std::to_string(vec.y).c_str()));
        vecEle->LinkEndChild(yPosEle);

        TiXmlElement* zPosEle = new TiXmlElement("Z");
        zPosEle->LinkEndChild(new TiXmlText(std::to_string(vec.z).c_str()));
        vecEle->LinkEndChild(zPosEle);
        element->LinkEndChild(vecEle);
    }
}

void XMLGeometryWriter::dumpDouble(TiXmlElement* element, double& value, const char* nodeName)
{
    TiXmlElement* doubleEle = new TiXmlElement(nodeName);
    doubleEle->LinkEndChild(new TiXmlText(std::to_string(value).c_str()));
    element->LinkEndChild(doubleEle);
}

void XMLGeometryWriter::dumpInt(TiXmlElement* element, int& value, const char* nodeName)
{
    TiXmlElement* intEle = new TiXmlElement(nodeName);
    intEle->LinkEndChild(new TiXmlText(std::to_string(value).c_str()));
    element->LinkEndChild(intEle);
}

void XMLGeometryWriter::dumpText(TiXmlElement* element, std::string& value, const char* nodeName)
{
    TiXmlElement* textEle = new TiXmlElement(nodeName);
    textEle->LinkEndChild(new TiXmlText(value.c_str()));
    element->LinkEndChild(textEle);
}

void XMLGeometryWriter::dumpBoolean(TiXmlElement* element, Adesk::Boolean& value, const char* nodeName)
{
    std::string val = "FALSE";
    if (value)
        val = "TRUE";
    
    TiXmlElement* boolEle = new TiXmlElement(nodeName);
    boolEle->LinkEndChild(new TiXmlText(val.c_str()));
    element->LinkEndChild(boolEle);
}
