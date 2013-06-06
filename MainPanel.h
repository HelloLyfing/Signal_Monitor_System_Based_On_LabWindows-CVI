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
#define  ConfigP_YesConfig                12      /* control type: command, callback function: ConfigPanelYesBtn */
#define  ConfigP_CancelConfig             13      /* control type: command, callback function: ConfigPanelCancelBtn */
#define  ConfigP_TIMEOUT_MSG1             14      /* control type: textMsg, callback function: (none) */

#define  MainPanel                        2
#define  MainPanel_Quit                   2       /* control type: command, callback function: QuitCallback */
#define  MainPanel_Config_Btn             3       /* control type: command, callback function: Config_Com_Btn */
#define  MainPanel_Start                  4       /* control type: command, callback function: StartBtnCallback */
#define  MainPanel_BINARYSWITCH_2         5       /* control type: binary, callback function: (none) */
#define  MainPanel_ID_List                6       /* control type: ring, callback function: (none) */
#define  MainPanel_TEXTBOX                7       /* control type: textBox, callback function: (none) */

#define  PopupPane                        3       /* callback function: PopupPanelCallBack */
#define  PopupPane_PopupGraph             2       /* control type: strip, callback function: (none) */
#define  PopupPane_FFT_Btn                3       /* control type: command, callback function: FFT_Btn_Callback */
#define  PopupPane_GRAPH                  4       /* control type: graph, callback function: (none) */

#define  PopupPanel                       4       /* callback function: PopupPanelCallBack */
#define  PopupPanel_GRAPH_ANALYSIS        2       /* control type: graph, callback function: (none) */
#define  PopupPanel_PopGraph2             3       /* control type: graph, callback function: (none) */
#define  PopupPanel_PopGraph1             4       /* control type: graph, callback function: (none) */
#define  PopupPanel_PopQuitBtn            5       /* control type: command, callback function: PopQuitBtn */
#define  PopupPanel_NUMERIC_PHASE         6       /* control type: numeric, callback function: ring1Callback */
#define  PopupPanel_NUMERIC_SAMPLEPOINT   7       /* control type: numeric, callback function: ring1Callback */
#define  PopupPanel_NUMERIC_AMP           8       /* control type: numeric, callback function: ring1Callback */
#define  PopupPanel_RING                  9       /* control type: ring, callback function: analysisCallback */
#define  PopupPanel_DECORATION_3          10      /* control type: deco, callback function: (none) */
#define  PopupPanel_DECORATION            11      /* control type: deco, callback function: (none) */
#define  PopupPanel_DECORATION_2          12      /* control type: deco, callback function: (none) */
#define  PopupPanel_RING_WINDOWTYPE       13      /* control type: ring, callback function: ring2Callback */
#define  PopupPanel_BINARYSWITCH          14      /* control type: binary, callback function: (none) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_Menu_File                2
#define  MENUBAR_Menu_File_Menu_Exit      3       /* callback function: menu_Exit */
#define  MENUBAR_Menu_Control             4
#define  MENUBAR_Menu_Control_Menu_Pause  5       /* callback function: pauseReceive */
#define  MENUBAR_Menu3_View               6
#define  MENUBAR_Menu3_View_M_View_1      7       /* callback function: switchMode_Tab_CPanel */


     /* Callback Prototypes: */

int  CVICALLBACK analysisCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Config_Com_Btn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ConfigPanelCancelBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ConfigPanelYesBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FFT_Btn_Callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK menu_Exit(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK pauseReceive(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK PopQuitBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PopupPanelCallBack(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ring1Callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ring2Callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StartBtnCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK switchMode_Tab_CPanel(int menubar, int menuItem, void *callbackData, int panel);


#ifdef __cplusplus
    }
#endif
