#ifndef VERIDICTENGINE_TOOLS_HPP
#define VERIDICTENGINE_TOOLS_HPP

#include <map>
#include <vector>
#include <variant>
#include "iostream"
#include "raylib.h"

namespace VerTools
{
    using any = std::variant<int, float, double, std::string, bool, std::vector<int>>;

    float                    F_fGetRadian(Vector2 l_v2Point1, Vector2 l_v2Point2);
    float                    F_fGetAngle(Vector2 l_v2Point1, Vector2 l_v2Point2);

    Color                    F_colHexToColor(std::string hex);
    bool                     F_bIsCursorFromScreen();

    std::string              F_sReadFile(const std::string& l_sPath);
    void                     F_vWriteFile(const std::string& l_sPath, const std::string& l_sValue);
    void                     F_vCreateFile(const std::string& l_sPath);
    void                     F_vRemoveFile(const std::string& l_sPath);
    void                     F_vCreateFolder(const std::string& l_sPath);
    void                     F_vRemoveFolder(const std::string& l_sPath);

    bool                                        F_bIsExistFolder(const std::string& l_sPath);
    bool                                        F_bIsExistFile(const std::string& l_sPath);
    std::vector<std::pair<std::string, bool>>   F_lGetContentFolder(std::string l_sPath);

    int                      F_iGetNumberOccurences(const std::string& l_sContent, const std::string& l_sTarget);

    std::string                          F_sCreateNetworkMSG(const std::string& l_sAction, const std::map<std::string, std::string>& l_sArgs);
    std::map<std::string, std::string>   F_sParseNetworkMSG(const std::string& l_sNetworkMessage);

    std::vector<std::string> F_lSplit(const std::string& l_sContent, const std::string& l_sDelimiter);
    std::string              F_sReplaceStringFromString(const std::string& source, const std::string& toReplace, const std::string& replacement);

    std::string F_sGetParent(const std::string& l_sPath); // "SomeFolder/files/main.cpp" will become "SomeFolder/files"
    std::string F_sGetNameFromPath(const std::string& l_sPath); // "SomeFolder/files/main.cpp" will become "main.cpp"
    std::string F_sGetExtFromName(const std::string &l_sName);

    std::vector<std::string> F_lSearch(std::vector<std::string> l_lList, const std::string& l_sSearch, bool l_bMatchCase=true);

    std::string F_sToLower(const std::string &l_sStr);
}

#endif
