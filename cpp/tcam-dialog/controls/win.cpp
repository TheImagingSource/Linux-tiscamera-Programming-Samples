#include "win.h"


void createWindows()
{
    initscr();
    start_color();
    cbreak();
    noecho();
    clear();
    keypad(stdscr, TRUE);
       /* Initialize all the colors */
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);
	init_pair(5, COLOR_YELLOW, COLOR_BLACK);

    init_pair(6, COLOR_BLACK, COLOR_WHITE);
    init_pair(7, COLOR_BLACK, COLOR_GREEN);
    init_pair(8, COLOR_BLACK, COLOR_RED);
    init_pair(9, COLOR_GREEN, COLOR_BLACK);


/*
    Windows.pbCameraTest.create(0,20,60);
    Windows.pbCamera.create(61,20,40);

    Windows.dutils.create(0,21,15    ,"    Dutils     ");
    Windows.tcamwb.create(15,21,15   ," White Balance ");
    Windows.tcamexp.create(30,21,15  ,"   Exposure    ");
    Windows.tcamfocus.create(45,21,15,"    Focus      ");
    Windows.tcambayer.create(60,21,15,"    Bayer      ");

    Windows.testoutput = newwin(17, 180 , 23, 0);
	wrefresh(Windows.testoutput);
    scrollok(Windows.testoutput,true);
*/
    curs_set(1);
}

WINDOW *createWindow(const char* title, int x, int y, int width, int height)
{
    
    WINDOW *win =  newwin(height, width , y, x);
	box(win, 0 , 0);	
    mvwprintw(win,0,1,title);
	wrefresh(win);

    return win;
}

void resetModuleLights(bool dutilsinstalled)
{
    if( dutilsinstalled )
        Windows.dutils.setState(Light::LightStates::on);
    else
        Windows.dutils.setState(Light::LightStates::off);
    
    Windows.tcamexp.setState(Light::LightStates::on);
    Windows.tcamwb.setState(Light::LightStates::on);
    Windows.tcamfocus.setState(Light::LightStates::on);
    Windows.tcambayer.setState(Light::LightStates::on);
}

void setModuleLights(std::list<std::string> tcambinchilds)
{
    for( auto &child : tcambinchilds)
    {
        if( child == "tcambin-dutils")
            Windows.dutils.setState(Light::LightStates::onok);
        if( child == "tcambin-exposure")
            Windows.tcamexp.setState(Light::LightStates::onok);
        if( child == "tcambin-whitebalance")
            Windows.tcamwb.setState(Light::LightStates::onok);
        if( child == "tcambin-focus")
            Windows.tcamfocus.setState(Light::LightStates::onok);
        if( child == "tcambin-debayer")
            Windows.tcambayer.setState(Light::LightStates::onok);
   }
}

