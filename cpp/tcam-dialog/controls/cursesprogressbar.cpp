
#include "cursesprogressbar.h"
#include <string.h>

WinBase::WinBase()
{
    _win = NULL;
}

CurseProgressbar::CurseProgressbar()
{
    
}


CurseProgressbar::CurseProgressbar(int x, int y, int width)
{
    create( x,  y,  width);
}

void CurseProgressbar::create(int x, int y, int width)
{
    _win =  newwin(1, width , y, x);
    _blocks = width - 2;
    _min = 0;
    _max = 100;
    _value = 0;
    
    mvwprintw(_win, 0,0,">");
    mvwprintw(_win, 0,_blocks+1,"<");
    for( int i =0 ; i < _blocks; i++)
    {
        mvwprintw(_win, 0,i+1,"_");
    }
    wrefresh(_win);
    //curs_set(1);
}

void CurseProgressbar::setMax( int max)
{
    if(_win == NULL)
    {
        return;
    }

    _max = max;
}

void CurseProgressbar::stepOne()
{
    setValue(_value + 1);
}

void CurseProgressbar::setValue( int value)
{
    if(_win == NULL)
    {
        return;
    }

    _value = value;

    int pos = (int)(  (float(_blocks) / (float)_max) * (float)_value);
    for( int i =0 ; i < _blocks; i++)
    {
        if( i <= pos)
        {
            //mvwprintw(_win, 0,i+1,"#",WACS_BLOCK);
            mvwaddch(_win, 0,i+1, ACS_CKBOARD);
            
        }
        else
            mvwprintw(_win, 0,i+1,"_");
        
    }
    wrefresh(_win);
}


///////////////////////////////////////////////////////////////
//
//  Light
///


Light::Light()
{
    strcpy(_text,"");
}

Light::Light(int x, int y, int width, const char* text)
{
    create(x,y,width,text);
}

void Light::create(int x, int y, int width, const char* text)
{
    _win =  newwin(1, width , y, x);
    strcpy(_text, text);
    mvwprintw(_win, 0,0,_text);
    wrefresh(_win);
    setState(LightStates::on);
}

void Light::setState(Light::LightStates state)
{
    if( _win == NULL )
        return;
    int color = 1;
    switch (state)
    {
    case LightStates::on :
        color = 6;
        break;
    case LightStates::onok :
        color = 7;
        break;
    case LightStates::onfail :
        color = 8;
        break;

    case LightStates::off :
    default:
        color = 1;
        break;
    }
    wattron( _win, COLOR_PAIR(color) );
    mvwprintw(_win, 0,0,_text);
    wattroff( _win, COLOR_PAIR(color) );

    wrefresh(_win);
}

