#ifndef VERIDICTENGINE_VERFILE_HPP
#define VERIDICTENGINE_VERFILE_HPP

#include <map>
#include "iostream"
#include "./Tools.hpp"

namespace VerFile
{
    class File
    {
    private:
        std::map<std::string, VerTools::any>  G_mValue;
        std::string                 G_sPath;

        void                        f_vWriteFile();
        std::map<std::string, VerTools::any>  f_mReadFile();

    public:
        File(const std::string& l_sPath) {
            if(!VerTools::F_bIsExistFile(l_sPath))
            {
                std::cerr << "[FILE] File not found: " << l_sPath << std::endl;
                exit(99);
            }
            G_sPath = l_sPath;

            this->G_mValue = (std::map<std::string, VerTools::any>) std::move(this->f_mReadFile());
        }

        void                F_vSetParam(const std::string& l_sKey, VerTools::any l_aValue);

        float               F_fGetParam(const std::string& l_sKey);
        double              F_dGetParam(const std::string& l_sKey);
        std::string         F_sGetParam(const std::string& l_sKey);
        std::vector<int>    F_lGetParam(const std::string& l_sKey);
        bool                F_bGetParam(const std::string& l_sKey);
        void                F_vShowList();
    };
}

#endif