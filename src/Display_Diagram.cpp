#include "Display_Diagram.h"
#include "Datastore.h"
#include "SunPosition.h"

#define CHART_HIGHT      20u    // chart area hight in pixels
#define CHART_WIDTH      47u    // chart area width in pixels
#define DIAG_POSX        80u    // position were Diag is drawn at
#define DIAG_POSY         0u
#define TIME_TO_RESET   (60u * 60u * 4u) // time gap to detect a reset (4 hours)
// resolution per dot: 24h * 60 min * 60 sec / 100 display dots
// resolution per dot: 10h * 60min * 60 sec / 47 display dots => 766
#define SEC_PER__DOT    ((10u * 60u * 60u) / CHART_WIDTH)
// earliest acceptable date 2023-01-01
#define EARLIEST_DATE (2022u * 365u * 24u * 60u * 60u)

DisplayDiagramClass::DisplayDiagramClass(void)
{
    _LastStart = 0;
    _iRunningAverage = 0;
    _maxWatts = 200;
    memset((void*)&_acGraph, 0, sizeof(_acGraph));
    _iRunningAverageCnt = 0;
    _acGraphCurPos = 0;
}

DisplayDiagramClass::~DisplayDiagramClass(void)
{
}
void DisplayDiagramClass::reset()
{
    _LastStart = time(nullptr);
    _iRunningAverage = 0;
    _iRunningAverageCnt = 0;
    memset((void*)&_acGraph, 0, sizeof(_acGraph));
    _acGraphCurPos = 0;
    _maxWatts = 100;
}

void DisplayDiagramClass::init(U8G2* display)
{
    _display = display;
}

void DisplayDiagramClass::update()
{
    time_t now = time(nullptr);
    char fmtText[6];
    u_char cnt;
    u_char n = 1; 
    char PosX = DIAG_POSX + (_mExtra % 2);
    char PosY = DIAG_POSY + (_mExtra % 2);

    // night detection to reset graph
    if ((now - TIME_TO_RESET) >  (_LastStart)){
        reset();
        _LastStart = now;
        Serial.println(_LastStart + SEC_PER__DOT);
    }
    // calculation average over the last period
    if(now >= (_LastStart + SEC_PER__DOT))
    {   // time period is over let's calculate the average and start a new period
        if(_acGraphCurPos >= CHART_WIDTH){  // we're exceeding the end of the graph area -> shift graph to left
            for(int i = 0; i < CHART_WIDTH - 1; i++){
                _acGraph[i] = _acGraph[i+1];
            }
            _acGraphCurPos = CHART_WIDTH - 1;
        }
        if(_iRunningAverageCnt != 0){
            _acGraph[_acGraphCurPos++] = _iRunningAverage / _iRunningAverageCnt;
            _iRunningAverage = 0;
            _iRunningAverageCnt = 0;
        }
        _LastStart = now;
      
        //Serial.print("Time: "); Serial.print(_LastStart + SEC_PER__DOT);
        //Serial.print(" value: "); Serial.println((int)_acGraph[_acGraphCurPos-1]);
    }

    float currentWatts = Datastore.getTotalAcPowerEnabled();   // get the current AC production 
    _iRunningAverage += currentWatts;
    _iRunningAverageCnt++;
    _maxWatts = currentWatts > _maxWatts ? currentWatts : _maxWatts;  // max wattage for diagram and scaling

    _display->clearBuffer();
    // draw diagram axis
    _display->drawVLine(PosX, PosY, CHART_HIGHT);
    _display->drawHLine(PosX, PosY + CHART_HIGHT-1, CHART_WIDTH);
    _display->drawLine(PosX + 1, PosY + 1 ,PosX + 2, PosY + 2); //UP-arrow
    _display->drawLine(PosX + CHART_WIDTH - 3, PosY + CHART_HIGHT - 3 ,PosX + CHART_WIDTH-2, PosY + CHART_HIGHT - 2); //LEFT-arrow
    _display->drawLine(PosX + CHART_WIDTH - 3, PosY + CHART_HIGHT + 1 ,PosX + CHART_WIDTH-2, PosY + CHART_HIGHT); //LEFT-arrow

    // draw AC value
    _display->setFont(u8g2_font_tom_thumb_4x6_mr);
    snprintf(fmtText, sizeof(fmtText), "%dW", (int)_maxWatts);
    for(cnt = 0; fmtText[(int)cnt] != 0; cnt++);  // cout chars of wattage
    _display->drawStr(PosX - (cnt * 4), DIAG_POSY + 5, fmtText);
    // draw chart
    float scaleFactor = _maxWatts / CHART_HIGHT;
    for(int i = 0; i < _acGraphCurPos; i++){
        if(i == 0){
            _display->drawPixel(PosX + 1 + i, PosY + CHART_HIGHT - ((_acGraph[i] / scaleFactor) + 0.5)); // + 0.5 to round mathematical
        }
        else{
            _display->drawLine(PosX + i, PosY + CHART_HIGHT - ((_acGraph[i-1] / scaleFactor) + 0.5), PosX + 1 + i, PosY + CHART_HIGHT - ((_acGraph[i] / scaleFactor) + 0.5));
        }
        // draw one tick per hour to the x-axis
        if(i * SEC_PER__DOT > (3600u * n)){
            _display->drawPixel(PosX + 1 + i, PosY + CHART_HIGHT);
            n += 1;
        }
    }
}

