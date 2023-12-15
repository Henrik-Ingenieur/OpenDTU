#pragma once

//#include "Display_Graphic.h"

#include "defaults.h"
#include <U8g2lib.h>
#include <time.h>

#define GRAPH_LENGTH 100u

class DisplayDiagramClass {
public:
    DisplayDiagramClass(void);
    ~DisplayDiagramClass(void);
    void init(U8G2* display);
    void update();
    void reset();   // will start the diagram calculation like it's a new day
private:
    U8G2* _display;
    time_t _LastStart;
    float _iRunningAverage;
    float _maxWatts;
    float _acGraph[GRAPH_LENGTH];   // each dot of the line is represented with its value in this array
    unsigned int _mExtra = 0;
    unsigned int _iRunningAverageCnt;
    unsigned char _acGraphCurPos;           // position were we are at the graph over a day
};
