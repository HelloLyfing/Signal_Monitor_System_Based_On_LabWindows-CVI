/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2013. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  ConfigP                          1
#define  ConfigP_COMPORT                  2       /* control type: slide, callback function: (none) */
#define  ConfigP_BAUDRATE                 3       /* control type: slide, callback function: (none) */
#define  ConfigP_PARITY                   4       /* control type: slide, callback function: (none) */
#define  ConfigP_DATABITS                 5       /* control type: slide, callback function: (none) */
#define  ConfigP_STOPBITS                 6       /* control type: slide, callback function: (none) */
#define  ConfigP_INPUTQ                   7       /* control type: numeric, callback function: (none) */
#define  ConfigP_OUTPUTQ                  8       /* control type: numeric, callback function: (none) */
#define  ConfigP_CTSMODE                  9       /* control type: binary, callback function: (none) */
#define  ConfigP_XMODE                    10      /* control type: binary, callback function: (none) */
#define  ConfigP_TIMEOUT                  11      /* control type: numeric, callback function: (none) */
#define  ConfigP_YesConfig                12      /* control type: command, callback function: YesConfigBtn */
#define  ConfigP_CancelConfig             13      /* control type: command, callback function: CancelConfigBtn */
#define  ConfigP_TIMEOUT_MSG1             14      /* control type: textMsg, callback function: (none) */

#define  MainPanel                        2
#define  MainPanel_MP_Config              2       /* control type: command, callback function: Config_Com_Btn */
#define  MainPanel_Start                  3       /* control type: command, callback function: Start_Callback */
#define  MainPanel_Quit                   4       /* control type: command, callback function: QuitCallback */
#define  MainPanel_DataSrcSwitcher        5       /* control type: binary, callback function: (none) */
#define  MainPanel_TextMsg                6       /* control type: textMsg, callback function: (none) */

#define  Panel2                           3
#define  Panel2_stat3                     2       /* control type: string, callback function: (none) */
#define  Panel2_stat2                     3       /* control type: string, callback function: (none) */
#define  Panel2_Stat1                     4       /* control type: string, callback function: (none) */
#define  Panel2_3DGraph                   5       /* control type: activeX, callback function: (none) */
#define  Panel2_airplaneStart             6       /* control type: command, callback function: airplaneStart */
#define  Panel2_Quitbtn                   7       /* control type: command, callback function: airplaneQuit */

#define  PopupPanel                       4       /* callback function: PopupPanelCallBack */
#define  PopupPanel_PopupGraph            2       /* control type: graph, callback function: (none) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_Menu_File                2
#define  MENUBAR_Menu_File_Menu_Exit      3       /* callback function: menu_Exit */
#define  MENUBAR_Menu_Control             4
#define  MENUBAR_Menu_Control_Menu_Pause  5       /* callback function: pauseReceive */
#define  MENUBAR_Menu3_View               6
#define  MENUBAR_Menu3_View_M_View_1      7       /* callback function: switchMode_Tab_CPanel */
#define  MENUBAR_Menu3_View_ShowAirplaneModel 8
#define  MENUBAR_Menu3_View_ShowAirPlaneModel 9   /* callback function: gotoRunAirplanModel */


     /* Callback Prototypes: */

int  CVICALLBACK airplaneQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK airplaneStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CancelConfigBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Config_Com_Btn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK gotoRunAirplanModel(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK menu_Exit(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK pauseReceive(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK PopupPanelCallBack(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Start_Callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK switchMode_Tab_CPanel(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK YesConfigBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
