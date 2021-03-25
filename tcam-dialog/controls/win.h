
#ifndef __WIN
#define __WIN
#include <list>
#include <string>
#include <curses.h>
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define YELLOW 5
#include "cursesprogressbar.h"
#include <menu.h>
#include <form.h>

struct WINDOWS_t
{
    WINDOW *testoutput;
    CurseProgressbar pbCamera;
    CurseProgressbar pbCameraTest;
    Light tcamwb;
    Light tcamexp;
    Light tcamfocus;
    Light tcambayer;
    Light dutils;
}; 
extern WINDOWS_t Windows;
void createWindows();
WINDOW * createWindow(const char* title, int x, int y, int width, int height);
void resetModuleLights(bool dutilsinstalled);
void setModuleLights(std::list<std::string> tcambinchilds);
#endif
