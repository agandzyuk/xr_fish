#ifndef __fisher_config_h__
#define __fisher_config_h__

#include "fisher.h"
#include <string>

namespace Fisher {

/// class Config ////////////////////////////////
class Config
{
protected:
    Config(const std::wstring& profile);
    ~Config();

public:
    static void create_profile(std::wstring profile = L"");
    static void reload_profile();
    static Config& get();

    const std::wstring& Config::get_profile_name() const;

private:
    std::wstring profile_;
    static Config* self_;
};

}
#endif  // __fisher_config_h__