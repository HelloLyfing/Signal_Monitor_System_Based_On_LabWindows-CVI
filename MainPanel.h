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
#define  MainPanel_ControlBtn             4       /* control type: command, callback function: ControlShowingBtnCallback */
#define  MainPanel_StoreTheData_Switch    5       /* control type: binary, callback function: (none) */
#define  MainPanel_Signal_ID_List         6       /* control type: ring, callback function: chooseASignal_List_Callback */
#define  MainPanel_MainLogBox             7       /* control type: textBox, callback function: MainLog_Callback */

#define  PopupPanel                       3       /* callback function: PopupPanelCallBack */
#define  PopupPanel_PopGraph3             2       /* control type: graph, callback function: (none) */
#define  PopupPanel_PopGraph2             3       /* control type: graph, callback function: (none) */
#define  PopupPanel_PopGraph1             4       /* control type: graph, callback function: PopGraph1Callback */
#define  PopupPanel_PopLogInfoBtn         5       /* control type: command, callback function: PopLogInfoBtn */
#define  PopupPanel_PopQuitBtn            6       /* control type: command, callback function: PopQuitBtn */
#define  PopupPanel_RingFFT               7       /* control type: ring, callback function: analysisCallback */
#define  PopupPanel_DECORATION_3          8       /* control type: deco, callback function: (none) */
#define  PopupPanel_DECORATION            9       /* control type: deco, callback function: (none) */
#define  PopupPanel_DECORATION_2          10      /* control type: deco, callback function: (none) */
#define  PopupPanel_RingWindowType        11      /* control type: ring, callback function: windowTypeCallback */
#define  PopupPanel_PopupSwitcher         12      /* control type: binary, callback function: PopSwitcherCallback */
#define  PopupPanel_PopLogBox             13      /* control type: textBox, callback function: (none) */

#define  ReadPanel                        4
#define  ReadPanel_PathString             2       /* control type: string, callback function: (none) */
#define  ReadPanel_FinishChooseFileBtn    3       /* control type: command, callback function: finishChooseFileBtnCallback */
#define  ReadPanel_ChooseFileBtn          4       /* control type: command, callback function: chooseFileBtnCallback */
#define  ReadPanel_SetReadSpeedRing       5       /* control type: ring, callback function: readSpeedCallback */
#define  ReadPanel_TEXTMSG                6       /* control type: textMsg, callback function: (none) */
#define  ReadPanel_DECORATION             7       /* control type: deco, callback function: (none) */
#define  ReadPanel_DECORATION_2           8       /* control type: deco, callback function: (none) */
#define  ReadPanel_FileInfoDetail         9       /* control type: textMsg, callback function: (none) */
#define  ReadPanel_TEXTMSG_2              10      /* control type: textMsg, callback function: (none) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_Menu_File                2
#define  MENUBAR_Menu_File_Menubar_LoadData 3     /* callback function: menuBarLoadDataCallback */
#define  MENUBAR_Menu_File_Menu_Exit      4       /* callback function: menu_Exit */
#define  MENUBAR_Menu_Control             5
#define  MENUBAR_Menu_Control_Menu_Pause  6       /* callback function: pauseReceive */
#define  MENUBAR_Menu_Control_Menu_StartAll 7     /* callback function: Menu_StartAll_Callback */
#define  MENUBAR_Menu_Control_Menu_STOP   8       /* callback function: Menu_StopAcq_Callback */
#define  MENUBAR_Menu3_View               9
#define  MENUBAR_Menu3_View_M_View_1      10      /* callback function: switchViewMode */


     /* Callback Prototypes: */

int  CVICALLBACK analysisCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK chooseASignal_List_Callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK chooseFileBtnCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Config_Com_Btn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ConfigPanelCancelBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ConfigPanelYesBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ControlShowingBtnCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK finishChooseFileBtnCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MainLog_Callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK menu_Exit(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK Menu_StartAll_Callback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK Menu_StopAcq_Callback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK menuBarLoadDataCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK pauseReceive(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK PopGraph1Callback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PopLogInfoBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PopQuitBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PopSwitcherCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PopupPanelCallBack(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK readSpeedCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK switchViewMode(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK windowTypeCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
