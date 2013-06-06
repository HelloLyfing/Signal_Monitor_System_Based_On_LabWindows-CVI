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

#define  Panel2                           1
#define  Panel2_stat3                     2       /* control type: string, callback function: (none) */
#define  Panel2_stat2                     3       /* control type: string, callback function: (none) */
#define  Panel2_Stat1                     4       /* control type: string, callback function: (none) */
#define  Panel2_3DGraph                   5       /* control type: activeX, callback function: (none) */
#define  Panel2_airplaneStart             6       /* control type: command, callback function: airplaneStart */
#define  Panel2_Quitbtn                   7       /* control type: command, callback function: airplaneQuit */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK airplaneQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK airplaneStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
