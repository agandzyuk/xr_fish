#include "config.h"
#include "exception.h"

using namespace std;
using namespace Fisher;

Config* Config::self_ = 0;

/// Config //////////////////////////////////////////////////////
Config::Config( const wstring& profile )
    : profile_(profile)
{
}

Config::~Config()
{
}

void Config::create_profile( wstring profile )
{
    if ( self_ ) 
    {
        if ( profile.empty() )
            profile = DEFAULT_PROFILE_NAME;

        if (  0 == _wcsicmp( self_->get_profile_name().c_str(), profile.c_str() ) )
            throw Exception( L"Profile \"" + profile + L"\" already loaded." );
    }

    Config* obj = new Config(profile);
    if ( obj ) {
        delete self_;
        self_ = obj;
    }
}

void Config::reload_profile()
{
    if ( !self_ )
        throw Exception("Do load profile on first!");

    Config* obj = new Config( self_->get_profile_name() );
    if ( obj ) {
        delete self_;
        self_ = obj;
    }
}

Config& Config::get()
{
    if ( !self_ )
        throw Exception("Do load profile on first!");
    return *self_;
}

const std::wstring& Config::get_profile_name() const
{
    return profile_;
}
