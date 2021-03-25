#ifndef __PROGRESSBAR
#define __PROGRESSBAR
#include <curses.h>

class WinBase
{
    public:
        WinBase();

    protected:
        WINDOW *_win;
};

class CurseProgressbar : public WinBase
{
    public:
        CurseProgressbar();
        CurseProgressbar(int x, int y, int width);
        void create(int x, int y, int width);
        void setMax( int max);
        void setValue( int value);   
        void stepOne(); 
    private:
        int _blocks;
        int _min;
        int _max;
        int _value;
};

class Light : public WinBase
{
    public:
        Light();
        Light(int x, int y, int width, const char* text);
        void create(int x, int y, int width, const char* text);

        enum LightStates
        {
            off,
            on,
            onok,
            onfail
        };

        void setState(Light::LightStates state);

    private:
        char _text[255];

};

#endif