#ifndef __fisher_textinfo_h__
#define __fisher_textinfo_h__

#include "fisher.h"

#include <FTGL/FTGLBitmapFont.h>
#include <string>
#include <map>

namespace Fisher {

/// class Font ///////////////////////////////////////

class TextInfo : public std::map<std::string,std::auto_ptr<FTFont>>
{
    typedef std::map<std::string,std::auto_ptr<FTFont>> BaseT;

public:
    TextInfo();
    ~TextInfo();

    void addFont(char* ttf, int fontSize, int fontDepth);
    void dbgprint(const wchar_t *text);
    void calculateFrameRate(int framesQueued);

private:
    float sfps_;
    int time_begin_;
    int time_refresh_;
    int frames_during_one_sec_;
    int prev_queue_state_;
};

}

#endif  // __fisher_textinfo_h__