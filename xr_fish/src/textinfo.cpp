#include "textinfo.h"
#include "app.h"

#include <freetype/freetype.h>
#include <FTCharmap.h>

using namespace Fisher;


////////////////////////////////////////////////
TextInfo::TextInfo() 
    : sfps_(0.0f),
    time_begin_(0),
    time_refresh_(0),
    frames_during_one_sec_(0),
    prev_queue_state_(0)
{}

TextInfo::~TextInfo() 
{}

void TextInfo::addFont(char* ttf, int fontSize, int fontDepth)
{
    FTGLBitmapFont* f = new FTGLBitmapFont(ttf);
    f->FaceSize(fontSize);
    f->Depth(fontDepth);
    f->CharMap(ft_encoding_unicode);
    insert(BaseT::value_type(ttf, f));
}

void TextInfo::dbgprint(const wchar_t *text)
{
    if( empty() )
        return;

    FTFont* f = begin()->second.get();

    //////////////////////////////////////
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, App::clientRect_.right,App::clientRect_.bottom, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //////////////////////////////////////

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f,1.0f,1.0f);

        // text height layout
        GLfloat ypos = 0.9;
        if( App::clientRect_.bottom < WINDOW_SIZE_Y )
            ypos = 1.0 - 0.1 * GLfloat(WINDOW_SIZE_Y)/(App::clientRect_.bottom ? App::clientRect_.bottom : 1);

        glTranslatef(10,20,-1);
        glRasterPos2f(-1, 0.5f);
        f->Render(text);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    ////////////////////////////////////////
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

static int sFrames_during_one_sec = 0;
static int sPrev_queue_state = 0;

void TextInfo::calculateFrameRate(int framesEnqued)
{
    frames_during_one_sec_++;
    if ( (time_refresh_/500) )
    {
        sfps_ = frames_during_one_sec_ * 500;
        sfps_ /= time_refresh_;
        
/*        swprintf_s(buf, 512, L"fps: %.4f, enqued %d, added %d, removed %d, coef %.3f%%");
            sfps_, 
            framesEnqued, 
            frames_during_one_sec_ - prev_queue_state_ + framesEnqued_, 
            frames_during_one_sec_, 
            float(frames_during_one_sec_*100)/(frames_during_one_sec_ - prev_queue_state_ + framesEnqued) );
*/
        sFrames_during_one_sec = frames_during_one_sec_;
        sPrev_queue_state = prev_queue_state_;

        frames_during_one_sec_ = 0;
        time_refresh_ = 0;
        prev_queue_state_ = framesEnqued;
        time_begin_ = ::GetTickCount();
    }
    else {
        frames_during_one_sec_++;
        time_refresh_ = GetTickCount() - time_begin_;
    }

    wchar_t buf[512];
    swprintf_s( buf, 512, L"pos.x %f, pos.z %f, view.x %f, view.y %f", 
            App::camera_.pos().x, App::camera_.pos().z, App::camera_.view().x, App::camera_.view().y  );

/*
    swprintf_s( buf, 512, L"fps: %.4f, enqued %d, added %d, removed %d, coef %.3f%%",
                sfps_, 
                framesEnqued, 
                sFrames_during_one_sec - prev_queue_state_ + framesEnqued, 
                sFrames_during_one_sec, 
                float(sFrames_during_one_sec*100)/(sFrames_during_one_sec - sPrev_queue_state + framesEnqued) );
*/

    dbgprint(buf);
}
