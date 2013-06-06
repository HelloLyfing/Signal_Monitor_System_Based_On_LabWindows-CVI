#include <cviauto.h>
#include "toolbox.h"
#include <ansi_c.h>
#include <utility.h>
#include <rs232.h>
#include <formatio.h>
#include <analysis.h>
#include <cvirte.h>		
#include <userint.h>
#include <stdio.h>
#include <cviauto.h>
#include "3DGraphCtrl.h"
#include "MainPanel.h"

/*-- Error Check --*/
#ifndef errChk
#define errChk(fCall) if (error = (fCall), error < 0) \
{goto Error;} else
#endif
#define hrChk(f)            if (hr = (f), FAILED(hr)) goto Error; else

// Define.
#define READ_LENGTH 240
#define MAX_THREADS 49
/* int functionType */
#define beginDataAcqType		0
#define showToastType			1

//temp things
static char filePath[MAX_PATHNAME_LEN];
static int temp_int = 0;

/* ============= Static args ============= */
static CmtThreadPoolHandle poolHandle = 0;
static CmtTSQHandle tsqHdl;
static char filePath[MAX_PATHNAME_LEN];

static CAObjHandle graph3DHdl = 0;
static CAObjHandle graph3DPlotHdl = 0;

int panelHdl;
int menuBar; //For right-click popup.
int menuIDForPopup;
// Panel and it's child panels' args
int pWidth, pHeight, cWidth, cHeight, mHeight, tabPWidth, tabPHeight, extraRight,
		   	gFrameStyle, gFrameThickness, gTitleBarStyle, gTitleBarThickness, gSystemTheme,
			comPort, parity, dataBits, stopBits, inQueueSize, outQueueSize;
long baudRate;

//<tabFlag,0,ChildPanels>;<tabFlag,1,TabPages>. TPanels[] Panels In the Tab
static int tabFlag, tabCtrl, tabWidth, tabHeight;
//Panels' Control Array;PGraphs = Panel's graph; PGPots = Panel graph's plot.
int PopPanels[6]={0}, PopGraphs[6]={0}, PopGPlots[6]={0}, popPanelIsPloting;
int CPanels[6]={0}, TPanels[6]={0}, PGraphs[6], PGPlots[6]; 
// Flag Vals
// configComFlag =1,Dimmed; =0, enabled
// receiveFlag =1,Receive; =0, pause.
int receiveFlag = 1, configComFlag = 1;
double xAxisRange[2], yAxisRange[2]; //x,y坐标轴的标值范围
//Data Read Write Relative
int bytesRead;
int writeTSQEndFlag = 0, readTSQEndFlag = 0;
int dataLen = 0;

//Ploting args
int plotBakgColor = VAL_BLACK;
int plotLineColor = VAL_BLUE;
int plotGridColor = VAL_GREEN;
int plotSelectLineHdl = 0; //temp
//Showing stat,signal's showing status
int signalShowingStat[6] = {0};
char statString[30];
// User Event
int mouseLeftBtnStat = 0;
double startPoint[2] = {0.0};  //temp
double endPoint[2] = {0.0};	   //temp
int cursorsValidNum = 0;
//Callback func
int graphsCallbackInstalled = 0; // >0,not Installed; >1,ChildPMode Installed; >2,TabMode Installed.

unsigned char readData[READ_LENGTH];
int validMonNum; // valid Monitoring Number
unsigned char g_databuffer[256];

// Other args
int averSpaceV;
int averSpaceH;

// Reading Ploting data args
double resultData[6][READ_LENGTH/24];
int resultCounter = 0;

// Error args
int RS232_Error = 0;

/*----------- Fuctions' prototype -----------*/
static int  ConvertTabPageToChildPanel(int, int, int);
static int createChildPanel(char title[], int index);
static int setupThreadPool (void);
static void cleanGarbage(void);
static HRESULT RefreshControls(void);

void initCOMConfig(void);
void showError (int ,int, char*);
void readDataFromFile(void);
void pauseDataAcq(void);
void startDataAcq(void);
void initChildPanels(void);
void stopDataAcq(void); //temp
void SeperateFunc (unsigned char* finddata);
void SolveFunc (unsigned char* solvedata, int readnum);
void runSecondThread(int functionType, void* data);
void initVars(void);
void generateSimulateData(unsigned char *data);
void manageGraphCallbackFunc(void);
void showInfo(char *msg);
void showMenuPopup(int pHdl, int ctrlID, int xPoint, int yPoint);
void runAirplanModel(void);
void popupPanelForGraph(int pHdl);
void closePopupPanel(int pHdl);
void refreshDimmedStat(void);
int seperateData (unsigned char *data);
int getGraphIndex(int gCtrl);
char* getStatString(int ctrl);
void CVICALLBACK readDataAndPlot(CmtTSQHandle queueHandle, unsigned int event,
        	int value, void *callbackData);
int CVICALLBACK receiveDataWriteToTSQ(void *functionData);
int CVICALLBACK showToast(void *functionData);
int CVICALLBACK graphCallbackFunc(int panelHandle, int controlID, int event, 
			void *callbackData, int eventData1, int eventData2);
void CVICALLBACK menuPopupCallback(int menuBarHandle, int menuItemID, 
	void *callbackData, int panelHandle);
int CVICALLBACK showAirplaneModelll(void *functionData);
/*---------------------------------------------------------------------------*/
// Main Func, entry point
/*---------------------------------------------------------------------------*/
int main (int argc, char *argv[]){
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHdl = LoadPanel(0, "MainPanel.uir", MainPanel)) < 0)
		return -1;
	initVars(); //init variables
	DisplayPanel (panelHdl);
	RunUserInterface();
	cleanGarbage();
	return 0;
}

/*---------------------------------------------------------------------------*/
// 初始化→变量 
/*---------------------------------------------------------------------------*/
void initVars(void){
	/*-- After load Panel, you can init args' value right here ---*/
	//Panel relative Vals
	GetPanelAttribute(panelHdl, ATTR_WIDTH, &pWidth);
	GetPanelAttribute(panelHdl, ATTR_HEIGHT, &pHeight);
	GetPanelAttribute(panelHdl, ATTR_MENU_HEIGHT, &mHeight);
	validMonNum = 6;
	averSpaceH = 8; averSpaceV = 20; extraRight = 150;
	pHeight = pHeight - mHeight;
	cWidth = (pWidth - extraRight - averSpaceH*3)/3;
	cHeight = (pHeight - averSpaceV*2)/2;
	plotBakgColor = MakeColor(255, 204, 102); //Set background color of graphs
	plotGridColor = MakeColor(51, 102, 153);
	plotLineColor = MakeColor(0, 51, 102);
	popPanelIsPloting = 0;
	//Create a ThreadPool
	if(setupThreadPool()<0){
		MessagePopup("s", "Create thread pool error.");
	}
	//ChildPanel,TabPage、and Popup panel's Vals
	tabWidth = pWidth-extraRight-averSpaceH*3;
	tabHeight = pHeight - averSpaceV*3;
	//x,y坐标的标值范围
	xAxisRange[0] = 0.0, xAxisRange[1] = 9.0; 
	yAxisRange[0] = 0.0, yAxisRange[1] = 5.0;
	refreshDimmedStat();
}
/*---------------------------------------------------------------------------*/
// 程序退出时，清理占用的空间和垃圾 
/*---------------------------------------------------------------------------*/
static void cleanGarbage(void){
	pauseDataAcq();
	CmtDiscardTSQ(tsqHdl);
	DiscardPanel(panelHdl);
}

/*---------------------------------------------------------------------------*/
// [Start] Button
/*---------------------------------------------------------------------------*/
int CVICALLBACK Start_Callback(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	switch (event){
		case EVENT_COMMIT:
			if (CPanels[0]>0 || TPanels[0]>0) break;
			initChildPanels();
			CmtNewTSQ (READ_LENGTH, sizeof(char), OPT_TSQ_DYNAMIC_SIZE, &tsqHdl);
			CmtInstallTSQCallback (tsqHdl, EVENT_TSQ_ITEMS_IN_QUEUE, READ_LENGTH,
				readDataAndPlot, NULL, CmtGetCurrentThreadID(), NULL);
			startDataAcq();
			runSecondThread(showToastType, "Hello World !");
			break;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/ 
// In another thread, Receive datas and write them into TSQ 
/*---------------------------------------------------------------------------*/
int CVICALLBACK receiveDataWriteToTSQ(void *functionData){
	initCOMConfig();
	///OpenComConfig(comPort, "", baudRate, parity, dataBits, stopBits, inQueueSize, outQueueSize);
	//Fmt(filePath, "c:\\Users\\Lyfing\\Desktop\\temp\\Perfect.dat");
	//FILE *file = fopen(filePath, "r");
	while(receiveFlag){
		///bytesRead = ComRd(comPort, readData, READ_LENGTH);
		// fwrite(&readData, sizeof(char), READ_LENGTH, file);
		writeTSQEndFlag = 1;
		generateSimulateData(readData);
		CmtWriteTSQData(tsqHdl, &readData, READ_LENGTH, TSQ_INFINITE_TIMEOUT, NULL);
		Delay(0.6);
		writeTSQEndFlag = 0;
	}
	///CloseCom(comPort);
	// fclose(file);
	return 0;
}

/* Run func in another thread according to functionType */
void runSecondThread(int functionType, void *data){
	switch(functionType){
		case beginDataAcqType:
			CmtScheduleThreadPoolFunction(poolHandle, receiveDataWriteToTSQ, NULL, NULL);
			break;
		case showToastType:
			char *msg = (char *)data;
			showInfo(msg);
			CmtScheduleThreadPoolFunction(poolHandle, showToast, msg, NULL);
			break;
	}
}

/*---------------------------------------------------------------------------*/
// In another thread, perform the Read data and Plot data actions.
/*---------------------------------------------------------------------------*/	
void CVICALLBACK readDataAndPlot(CmtTSQHandle queueHandle, unsigned int event,
                                int value, void *callbackData){
	unsigned char data[READ_LENGTH];
	int tsqRValue = value, tsqREvent = event; //Thread Safe Result Value.
	switch (tsqREvent){
        case EVENT_TSQ_ITEMS_IN_QUEUE:
            /* Read data from the thread safe queue and plot on the graph */
            while (tsqRValue >= READ_LENGTH){
                CmtReadTSQData(tsqHdl, data, READ_LENGTH, TSQ_INFINITE_TIMEOUT, 0);
				/* Start to calculate the 陀螺仪的x,y,z 以及加计的x,y,z */
				// memset(resultData, 0, sizeof(resultData));
				resultCounter = 0; //用来重新 自增计数 resultData[x][resultCounter]中的采集次数
				seperateData(&data[0]);
				/* Plot the data. Doing any signal-processing/data-analysis stuff here */
                if(tabFlag==0){ //Child Panels Mode
					for(int i=0; i<validMonNum; i++){
						if( PGPlots[i] > 0){ 
							DeleteGraphPlot(CPanels[i], PGraphs[i], PGPlots[i], VAL_IMMEDIATE_DRAW);
						}
						PGPlots[i] = PlotY (CPanels[i], PGraphs[i], resultData[i], READ_LENGTH/24, VAL_DOUBLE, VAL_THIN_LINE,
	                    VAL_EMPTY_SQUARE, VAL_SOLID, 1, plotLineColor);
					}
				}else{ //Tab pages mode
					int i = 0;
					GetActiveTabPage (panelHdl, tabCtrl, &i);
					if( PGPlots[i] > 0){
						DeleteGraphPlot(TPanels[i], PGraphs[i], PGPlots[i], VAL_IMMEDIATE_DRAW);
	                }
					PGPlots[i] = PlotY (TPanels[i], PGraphs[i], resultData[i], READ_LENGTH/24, VAL_DOUBLE, VAL_THIN_LINE,
	                           VAL_EMPTY_SQUARE, VAL_SOLID, 1, plotLineColor);	
				}
				popPanelIsPloting = 1;
				for(int i=0; i<validMonNum; i++){
					if( PopPanels[i]>0 ){
						char msg[200];Fmt(msg, "PopPanels[%d]=%d,\n  PopGPlots[%d]=%d\n", i,PopPanels[i],i,PopGPlots[i]);
						showInfo(msg);
						if(PopGPlots[i]>0){
							DeleteGraphPlot(PopPanels[i], PopGraphs[i], PopGPlots[i], VAL_IMMEDIATE_DRAW);
						}
						PopGPlots[i] = PlotY (PopPanels[i], PopGraphs[i], resultData[i], READ_LENGTH/24, VAL_DOUBLE, VAL_THIN_LINE,
	                           VAL_EMPTY_SQUARE, VAL_SOLID, 1, plotLineColor);
					}
				}
				popPanelIsPloting = 0;
				tsqRValue -= READ_LENGTH;
            }
            break;
    }//Switch()
}

/*---------------------------------------------------------------------------*/ 
// 辅助类函数 读取存储文件内容
/*---------------------------------------------------------------------------*/
void readDataFromFile(void){
	Fmt(filePath, "c:\\Users\\Lyfing\\Desktop\\Perfect.dat");
	FILE *file = fopen(filePath, "r");
	bytesRead = fread(&readData, sizeof(char), READ_LENGTH, file);
	fclose(file);
	printf("bytesRead = %d\n", bytesRead);
	for(int i=0; i<240; i++){
		if( i> 0 && i%24 == 0){ printf("-------------------\n");}
		printf("char[%d] = %x\n", i,readData[i]);
	}
}

/*---------------------------------------------------------------------------*/
// 辅助类函数，初始化COM端口的配置数据
/*---------------------------------------------------------------------------*/	
void initCOMConfig(void){
	comPort = 4;
	baudRate = 115200;
	parity = 0;
	dataBits = 8;
	stopBits = 1;
	inQueueSize = 512;
	outQueueSize = 512;
}

/*---------------------------------------------------------------------------*/
// 暂停、恢复数据采集
/*---------------------------------------------------------------------------*/	
void startDataAcq(void){
	if(tsqHdl > 0)
	CmtFlushTSQ(tsqHdl, TSQ_FLUSH_ALL, NULL);
	receiveFlag = 1;
	runSecondThread(beginDataAcqType, NULL);
}
void pauseDataAcq(void){
	receiveFlag = 0;
	//如果 write to TSQ process 没有完成，则等待完成
	while( writeTSQEndFlag != 0){
		Delay(0.01);
	}
}

/*---------------------------------------------------------------------------*/
// 初始化六个子面板及各个面板上的内容
/*---------------------------------------------------------------------------*/	
void initChildPanels(void){
	/* 循环创建六个Panel */
	char msg[256];
	for(int i = 0; i<6; i++){
		Fmt(msg, "ChildPanel[%d]", i);
		CPanels[i] = createChildPanel(msg, i);
		DisplayPanel (CPanels[i]);
	}
	//在子Panel上创建图表，设置Graph属性，准备显示数据
	for(int i=0; i<validMonNum; i++){
		PGraphs[i] = NewCtrl(CPanels[i], CTRL_GRAPH, "temp", 0,0);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_WIDTH, cWidth);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_HEIGHT, cHeight);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_PLOT_BGCOLOR, plotBakgColor);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_GRID_COLOR, plotGridColor);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_CTRL_MODE, VAL_HOT); //CtrlMode→Hot
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_GRID_COLOR, plotGridColor);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_NUM_CURSORS, 0);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_EDGE_STYLE, VAL_FLAT_EDGE);		
		//Enable Zoom and customize it's style
		///SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_ENABLE_ZOOM_AND_PAN, 1);
		///SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_ZOOM_STYLE, VAL_ZOOM_XAXIS);
	}
	//canvasCtrl = createCanvas(CPanels[0]); //temp
	manageGraphCallbackFunc();
	
}

/*---------------------------------------------------------------------------*/
// 在Tab 模式 和 ChildPanels 模式之间切换
/*---------------------------------------------------------------------------*/	
void CVICALLBACK switchMode_Tab_CPanel(int menuBar, int menuItem, void *callbackData,
		int panel){
	pauseDataAcq();
	if(tabFlag == 0){ //Child-Panel => Tab-Pages
		//创建Tab
		if(tabCtrl <=0){
			tabWidth = pWidth-extraRight-averSpaceH;
			tabHeight = pHeight - averSpaceV*2;
			tabCtrl = NewCtrl(panelHdl, CTRL_TABS, "", (mHeight + averSpaceV), averSpaceH);
	        SetCtrlAttribute(panelHdl, tabCtrl, ATTR_WIDTH, tabWidth);
	        SetCtrlAttribute(panelHdl, tabCtrl, ATTR_HEIGHT, tabHeight);
		}
		for(int i=0; i<validMonNum; i++){
			InsertPanelAsTabPage(panelHdl, tabCtrl, -1, CPanels[i]);
		}
		for(int i=0; i<validMonNum; i++){
			GetPanelHandleFromTabPage(panelHdl, tabCtrl, i, &TPanels[i]);
			SetCtrlAttribute(TPanels[i], PGraphs[i], ATTR_WIDTH, tabWidth-3);
			SetCtrlAttribute(TPanels[i], PGraphs[i], ATTR_HEIGHT, tabHeight-15);
			SetCtrlAttribute(TPanels[i], PGraphs[i], ATTR_NUM_CURSORS, cursorsValidNum);// cursorsValidNum
			SetAxisScalingMode(TPanels[i], PGraphs[i], VAL_BOTTOM_XAXIS, VAL_MANUAL, xAxisRange[0], xAxisRange[1]);
			SetAxisScalingMode(TPanels[i], PGraphs[i], VAL_LEFT_YAXIS, VAL_MANUAL, yAxisRange[0], yAxisRange[1]);
		}
		tabFlag = 1;
		for(int i=0; i<validMonNum; i++){
			DiscardPanel(CPanels[i]);
			CPanels[i] = 0;
		}
	}else{ //Tab-Pages => Child-Panel
		for(int i=0; i<validMonNum; i++){
			CPanels[i] = ConvertTabPageToChildPanel(panelHdl, tabCtrl, i);
			SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_WIDTH, cWidth);
			SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_HEIGHT, cHeight);
		}
		for(int i=0; i<validMonNum; i++){
			if(PGraphs[i]>0){
				SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_WIDTH, cWidth);
				SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_WIDTH, cWidth);
				SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_NUM_CURSORS, 0);
			}
		}
		tabFlag = 0;
		for(int i=0; i<validMonNum; i++){
			DisplayPanel(CPanels[i]);
			TPanels[i] = 0;
		}
		DiscardCtrl(panelHdl, tabCtrl);
		tabCtrl = 0;
	}//else
	startDataAcq();
}

/*---------------------------------------------------------------------------*/ 
// 辅助类函数，将TabPage还原回ChildPanel
/*---------------------------------------------------------------------------*/
static int ConvertTabPageToChildPanel(int panel, int tab, int tabIndex){
    int     childPanel, tabPanel, ctrl;
    char    title[256];
	//Get the tab-page label and handle
    GetTabPageAttribute(panel, tab, tabIndex, ATTR_LABEL_TEXT, title);
    GetPanelHandleFromTabPage(panel, tab, tabIndex, &tabPanel);
    //Create new child panel and set its attributes
    childPanel = createChildPanel(title, tabIndex);
    //Get the first control in the tab-page to convert.
	GetPanelAttribute(tabPanel, ATTR_PANEL_FIRST_CTRL, &ctrl);
	while(ctrl != 0){
		//Duplicate the control to the child panel
        DuplicateCtrl(tabPanel, ctrl, childPanel, 0, 
			VAL_KEEP_SAME_POSITION, VAL_KEEP_SAME_POSITION);
        //Get the next control to duplicate
        GetCtrlAttribute(tabPanel, ctrl, ATTR_NEXT_CTRL, &ctrl);
	}
    return childPanel;
}

/*---------------------------------------------------------------------------*/ 
// 辅助类函数 根据索引创建 六个Child Panel中的某一个
/*---------------------------------------------------------------------------*/
static int createChildPanel(char title[], int index){
	int cPanel = 0;
	if (index<3){
		cPanel = NewPanel(panelHdl, title,
		(mHeight+averSpaceV), ((cWidth+averSpaceH)*index + averSpaceH), 
		cHeight, cWidth);
	}else{
		cPanel = NewPanel(panelHdl, title,
		(mHeight+averSpaceV*2+cHeight), ((cWidth+averSpaceH)*(index-3) + averSpaceH),
		cHeight, cWidth);
	}
	SetPanelAttribute(cPanel, ATTR_CONFORM_TO_SYSTEM, 1);
	SetPanelAttribute(cPanel, ATTR_CONFORM_TO_SYSTEM_THEME, 1);
	SetPanelAttribute(cPanel, ATTR_FRAME_STYLE, VAL_OUTLINED_FRAME);
    return cPanel;
}

/*---------------------------------------------------------------------------*/
// A Manager to Install or Uninstall Graphs' call back func.
/*---------------------------------------------------------------------------*/
void manageGraphCallbackFunc(void){
	for(int i=0; i<validMonNum; i++){
		if(CPanels[0] <= 0) break;
		InstallCtrlCallback(CPanels[i], PGraphs[i], graphCallbackFunc, NULL);
	}
}

int CVICALLBACK graphCallbackFunc(int panelHandle, int controlID, int event, 
		void *callbackData, int eventData1, int eventData2){
	int pHdl = panelHandle, ctrlID = controlID;
	int x = eventData2, y = eventData1;
	char msg[256] = {'0'};
	switch(event){
		case EVENT_LEFT_CLICK:
			if( mouseLeftBtnStat == 0){
				Fmt(msg, "LeftClick.");
				showInfo(msg);
			}
			break;
		case EVENT_LEFT_CLICK_UP:
			if(tabFlag == 1){ //Select area only active on Tab Page mode
				int i = 0;
				Fmt(msg, "Click_Up");
				showInfo(msg);
			}
			break;
		case EVENT_RIGHT_CLICK:
			showMenuPopup(pHdl, ctrlID, x, y);
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			popupPanelForGraph(panelHandle);
			break;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
// 右键弹出菜单(只对Graph有效)
/*---------------------------------------------------------------------------*/
void showMenuPopup(int pHdl, int ctrlID, int xPoint, int yPoint){
	int i = getGraphIndex(pHdl);
	int menuBar = NewMenuBar(0);
	int menuIDForPop = NewMenu(menuBar, "" , -1);
	char openStr[25]="View in new window";
	char closeStr[25]="Close the popup window";
	if(tabFlag == 0){
		NewMenuItem(menuBar, menuIDForPop, "Pause", -1, 0, menuPopupCallback, NULL);
		NewMenuItem(menuBar, menuIDForPop, PopPanels[i]>0?closeStr:openStr, -1, 0, menuPopupCallback, NULL);
		NewMenuItem(menuBar, menuIDForPop, "Temp", -1, 0, menuPopupCallback, NULL);
	}else{
		NewMenuItem(menuBar, menuIDForPop, "Pause", -1, 0, menuPopupCallback, NULL);
		NewMenuItem(menuBar, menuIDForPop, PopPanels[i]>0?closeStr:openStr, -1, 0, menuPopupCallback, NULL);
		NewMenuItem(menuBar, menuIDForPop, "Temp", -1, 0, menuPopupCallback, NULL);
	}//if-else
	RunPopupMenu(menuBar, menuIDForPop, 
		pHdl, yPoint, xPoint, 0, 0, 0, 0);
	///DisplayPanel(menuPopupHdl);
}

void CVICALLBACK menuPopupCallback(int menuBarHandle, int menuItemID, 
	void *callbackData, int panelHandle){
	///temp gCtrl = *(int *)callbackData;
	int i = getGraphIndex(panelHandle);
	switch(menuItemID){
		case 3:
			break;
		case 4:
			if(PopPanels[i]>0)
				closePopupPanel(PopPanels[i]);
			else
				popupPanelForGraph(panelHandle);
			break;
		case 5:
			break;
	}
}

/* 弹出窗口，显示数据 */
void popupPanelForGraph(int pHdl){
	char panelTitle[20] = "Graph Detail";
	int panelH = 450;
	int panelW = 650;
	int i = getGraphIndex(pHdl); //Need a index to find one of the six graphs.
	// pHdl2 是从uir文件中load的弹出窗口
	if(PopPanels[i] <=0 ){
		if ((PopPanels[i] = LoadPanel(0, "MainPanel.uir", PopupPanel)) < 0)
			showError(0,0, "Load Error");
		DisplayPanel(PopPanels[i]);
		PopGraphs[i] = PopupPanel_PopupGraph;
		SetCtrlAttribute(PopPanels[i], PopGraphs[i], ATTR_PLOT_BGCOLOR, plotBakgColor);
		SetCtrlAttribute(PopPanels[i], PopGraphs[i], ATTR_GRID_COLOR, plotGridColor);
		SetCtrlAttribute(PopPanels[i], PopGraphs[i], ATTR_CTRL_MODE, VAL_HOT); //CtrlMode→Hot
		SetCtrlAttribute(PopPanels[i], PopGraphs[i], ATTR_NUM_CURSORS, 0);
		SetCtrlAttribute(PopPanels[i], PopGraphs[i], ATTR_GRAPH_BGCOLOR, VAL_TRANSPARENT);
		SetAxisScalingMode(PopPanels[i], PopGraphs[i], VAL_BOTTOM_XAXIS, VAL_MANUAL, xAxisRange[0], xAxisRange[1]);
		SetAxisScalingMode(PopPanels[i], PopGraphs[i], VAL_LEFT_YAXIS, VAL_MANUAL, yAxisRange[0], yAxisRange[1]);
		PopGPlots[i] = PlotY(PopPanels[i], PopGraphs[i], resultData[i], READ_LENGTH/24, VAL_DOUBLE, VAL_THIN_LINE,
	                           VAL_EMPTY_SQUARE, VAL_SOLID, 1, plotLineColor);
	}else{
		SetActivePanel(PopPanels[i]);
	}
	
}

int getGraphIndex(int ctrl){
	int i = 0;
	if(tabFlag == 0){ //Child Panel Mode
		for(i=0; i<validMonNum; i++){
			if(CPanels[i] == ctrl)
				return i;
		}
	}else{
		GetActiveTabPage (panelHdl, tabCtrl, &i);
		return i;
	}
	return i;
}
char* getStatString(int ctrl){
	int ctrlID;
	for(ctrlID=0; ctrlID<validMonNum; ctrlID++){
		if(ctrl == PGraphs[ctrlID]) break;	
	}
	if( signalShowingStat[ctrlID] == 0){ //stat = Pause
		return "Start";
	}else if (signalShowingStat[ctrlID] == 1){ //stat = Acquisition
		return "Pause";
	}
	return "";
}

int CVICALLBACK Config_Com_Btn(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch(event){
		case EVENT_COMMIT:
			int popConfigPanel = LoadPanel(0, "MainPanel.uir", ConfigP);
			InstallPopup(popConfigPanel);
			break;
	}
	return 0;
}

/* Show Toast Msg */
int CVICALLBACK showToast(void *functionData){
	char *msg = (char *)functionData;
	int length = sizeof(msg) + 5;
	int panelHdlTemp = panelHdl;
	int msgCtrl = NewCtrl(panelHdlTemp, CTRL_TEXT_MSG, 0, -10, pWidth-length);
	SetCtrlAttribute(panelHdlTemp, msgCtrl, ATTR_WIDTH, length);
	SetCtrlAttribute(panelHdlTemp, msgCtrl, ATTR_HEIGHT, 15);
	SetCtrlVal(panelHdl, msgCtrl, msg);
	double dTime = 0.5/15;
	
	for(int i=0; i<15; i++){
		SetCtrlAttribute(panelHdlTemp, msgCtrl, ATTR_TOP, i-10);
		Delay( dTime );
		showInfo(msg);
	}
	Delay(1);
	DiscardCtrl(panelHdlTemp, msgCtrl);
	return 0;
}

int CVICALLBACK ConfigPanelYesBtn(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch(event){
		case EVENT_COMMIT:
			configComFlag = 0; //Set Ctrols enable
			GetCtrlVal(panel, ConfigP_COMPORT, &comPort);
			GetCtrlVal(panel, ConfigP_BAUDRATE, &baudRate);
			GetCtrlVal(panel, ConfigP_PARITY, &parity);
			GetCtrlVal(panel, ConfigP_DATABITS, &dataBits);
			GetCtrlVal(panel, ConfigP_STOPBITS, &stopBits);
			GetCtrlVal(panel, ConfigP_INPUTQ, &inQueueSize);
			GetCtrlVal(panel, ConfigP_OUTPUTQ, &outQueueSize);
			
			comPort = 4;
			baudRate = 115200;
			parity = 0;
			dataBits = 8;
			stopBits = 1;
			inQueueSize = 512;
			outQueueSize = 512;
			refreshDimmedStat();
			DiscardPanel(panel);
			break;
	}
	return 0;
}
int CVICALLBACK ConfigPanelCancelBtn(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch(event){
		case EVENT_COMMIT:
			DiscardPanel(panel);
			break;
	}
	return 0;
}

void refreshDimmedStat(void){
	int i=0;
	//configComFlag =1, dimmed; =0, enabled.
	int Ctrls[2]={0}; //Object control
	int menuCtrls[5]={0}; //Menu control
	Ctrls[0] = MainPanel_Start;
	menuCtrls[0] = GetPanelMenuBar(panelHdl);
	menuCtrls[1] = MENUBAR_Menu_Control;
	menuCtrls[2] = MENUBAR_Menu3_View;
	for(i=0; i<sizeof(Ctrls)/sizeof(int); i++){
		if(Ctrls[i] <=0) continue;
		SetCtrlAttribute(panelHdl, Ctrls[i], ATTR_DIMMED, configComFlag);	
	}
	for(i=1; i<sizeof(menuCtrls)/sizeof(int); i++){
		if(menuCtrls[i] <=0 ) continue;
		SetMenuBarAttribute(menuCtrls[0], menuCtrls[i], ATTR_DIMMED, configComFlag);	
	}
	
}
/*---------------------------------------------------------------------------*/
// ***************************
// * Insert before this part *
// ***************************
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/ 
// Menu菜单的 所有回调函数
/*---------------------------------------------------------------------------*/
void CVICALLBACK gotoRunAirplanModel(int menuBar, int menuItem, void *callbackData,int panel){
	// runAirplaneModel();
}
void CVICALLBACK menu_Exit(int menuBar, int menuItem, void *callbackData,int panel){
  	QuitUserInterface (0);
}

void CVICALLBACK showMenu_2_1_1 (int menuBar, int menuItem, void *callbackData,int panel){
  MessagePopup("Menu_1_1","Hello world! I love you !");
}
void CVICALLBACK showMenu_2_2 (int menuBar, int menuItem, void *callbackData,int panel){
  MessagePopup("Menu_1_1","Hello world! I love you !");
}

/*---------------------------------------------------------------------------*/
// 辅助类函数 新建并初始化线程池
/*---------------------------------------------------------------------------*/
static int setupThreadPool (void){
    int error = 0;
    // errChk (CmtNewThreadLocalVar (sizeof (int), (void *)0, NULL, NULL, &tlvHandle));
    errChk (CmtNewThreadPool(MAX_THREADS, &poolHandle));
Error:
    return error;
}

/*---------------------------------------------------------------------------*/
// 菜单项 暂停采集
/*---------------------------------------------------------------------------*/
void CVICALLBACK pauseReceive (int menuBar, int menuItem, 
	void *callbackData, int panel){
	if(PGraphs[0] <= 0) return;
	if(receiveFlag == 0){
		SetMenuBarAttribute (menuBar, menuItem, ATTR_ITEM_NAME, "Pause");
		receiveFlag = 1;
		startDataAcq();
	}else{ 
		pauseDataAcq();
		SetMenuBarAttribute (menuBar, menuItem, ATTR_ITEM_NAME, "Resume");
	}
}

/*---------------------------------------------------------------------------*/
// 退出程序 
/*---------------------------------------------------------------------------*/
int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	switch (event){
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
	}
	return 0;
}
int  CVICALLBACK PopupPanelCallBack(int panel, int event, void *callbackData, 
		int eventData1, int eventData2){
	switch (event){
		case EVENT_CLOSE:
			closePopupPanel(panel);
			break;
	}//switch()
	return 0;
}
void closePopupPanel(int pHdl){
	int i = 0;
	while(popPanelIsPloting != 0)
		Delay(0.001);
	DiscardPanel(pHdl);
	for(i = 0; i<validMonNum; i++){
		if(PopPanels[i] == pHdl){	
			PopPanels[i] = 0;
			PopGPlots[i] = 0;
		}
	}
}
/*---------------------------------------------------------------------------*/
// 辅助类函数  显示错误信息			                                         
/*---------------------------------------------------------------------------*/
void showError (int errorType, int errorCode, char* msg){
    char ErrorMessage[256];
	strcat(ErrorMessage, msg);
    if (errorType == RS232_Error){
		switch (errorCode){
	        default:
	            if (errorCode < 0){  
	                Fmt (ErrorMessage, "%s<RS232 error number %i", errorCode);
	                MessagePopup ("RS232 Message", ErrorMessage);}
	            break;
	        case 0:
	            MessagePopup ("RS232 Message", "No errors.");
	            break;
	        case -2:
	            Fmt (ErrorMessage, "%s", 
					"Invalid port number (must be in the range 1 to 8).");
	            MessagePopup ("RS232 Message", ErrorMessage);
	            break;
	        case -3 :
	            Fmt (ErrorMessage, "%s", 
					"No port is open.\n Check COM Port setting in Configure.");
	            MessagePopup ("RS232 Message", ErrorMessage);
	            break;
	        case -99 :
	            Fmt (ErrorMessage, "%s", "Timeout error.\n\n"
	                 "Either increase timeout value,\n"
	                 "       check COM Port setting, or\n"
	                 "       check device.");
	            MessagePopup ("RS232 Message", ErrorMessage);
	            break;
		}//switch();
	}else {
		 MessagePopup("Error !", ErrorMessage);
	}
}

/*---------------------------------------------------------------------------*/
// 生成模拟数据
/*---------------------------------------------------------------------------*/	
void generateSimulateData(unsigned char *data){
	unsigned char buffData[24] = {0x00};
	buffData[0] = 0xeb; buffData[1] = 0xea; buffData[2] = 0x0f;
	buffData[3] = 0x23; buffData[4] = 0x6f; buffData[23] = 0x3a;
	int temp_int;
	for(temp_int=0; temp_int<10; temp_int++){
		int rand1 = (int)Random(0.0, 6.0);
		int rand2 = (int)Random(4.0, 8.0);
		int rand3 = (int)Random(7.0, 10.0);
		int rand4 = (int)Random(3.0, 7.0);
		for(int j=0; j<24; j++){
			data[j + 24*temp_int] = buffData[j];}
		if( temp_int == rand1){
				for(int k=0; k<6; k++){
					int ran1 = (int)Random(0.0, 6.0);
					int ran2 = (int)Random(0.0, 6.0);
					if(k == ran1){
						data[temp_int*24 +5+k*3] = 0x8b;
						data[temp_int*24 +6+k*3] = 0x99;
						data[temp_int*24 +7+k*3] = 0x8c;}	
					else if(k == ran2){
						data[temp_int*24 +5+k*3] = 0x80;
						data[temp_int*24 +6+k*3] = 0x9c;
						data[temp_int*24 +7+k*3] = 0x7f;}
					else{
					data[ 5 + k*3 + temp_int*24] = 0x8f;
					data[ 6	+ temp_int*24 +k*3] = 0x9c;
					data[temp_int*24 + 7+k*3] = 0x64;}}
		}else if(temp_int == rand2){
				for(int k=0; k<6; k++){
					data[temp_int*24 +5+k*3] = 0x80;
					data[temp_int*24 +6+k*3] = 0x9a;
					data[temp_int*24 +7+k*3] = 0xa1;}
		}else if(temp_int == rand3){
				for(int k=0; k<6; k++){
					data[temp_int*24 +5+k*3] = 0x8b;
					data[temp_int*24 +6+k*3] = 0x99;
					data[temp_int*24 +7+k*3] = 0x8c;}
		}else if(temp_int == rand4){
				for(int k=0; k<6; k++){
					int ran1 = (int)Random(0.0, 6.0);
					int ran2 = (int)Random(0.0, 6.0);
					if(k == ran1){
						data[temp_int*24 +5+k*3] = 0x8f;
						data[temp_int*24 +6+k*3] = 0x90;
						data[temp_int*24 +7+k*3] = 0x8c;}	
					else if(k == ran2){
						data[temp_int*24 +5+k*3] = 0x85;
						data[temp_int*24 +6+k*3] = 0x91;
						data[temp_int*24 +7+k*3] = 0x7f;}
					else{
					data[ 5 + k*3 + temp_int*24] = 0x7f;
					data[ 6	+ temp_int*24 +k*3] = 0x8e;
					data[temp_int*24 + 7+k*3] = 0x64;}}
		}else{
				for(int k=0; k<6; k++){
					int ran1 = (int)Random(0.0, 6.0);
					int ran2 = (int)Random(0.0, 6.0);
					if(k == ran1){
						data[temp_int*24 +5+k*3] = 0x82;
						data[temp_int*24 +6+k*3] = 0x9a;
						data[temp_int*24 +7+k*3] = 0x8c;}	
					else if(k == ran2){
						data[temp_int*24 +5+k*3] = 0x85;
						data[temp_int*24 +6+k*3] = 0x91;
						data[temp_int*24 +7+k*3] = 0x7f;}
					else{
					data[ 5 + k*3 + temp_int*24] = 0x87;
					data[ 6	+ temp_int*24 +k*3] = 0x9e;
					data[temp_int*24 + 7+k*3] = 0x89;}}
		}
	}
}

/*---------------------------------------------------------------------------*/
// 辅助类函数 将上传的数据分组
/*---------------------------------------------------------------------------*/
int seperateData (unsigned char *data){
	SolveFunc(&data[0], READ_LENGTH);
	return 0;
}
void SolveFunc (unsigned char* solvedata, int readnum){
	unsigned char storedata[24];
	for (int k=0; k<readnum; ){
		if (solvedata[k] == 0xeb && solvedata[k+1] == 0xea ){ //正常寻找帧
			SeperateFunc (&solvedata[k]);
			k = k + 24;}
		else{ k++;}
		if ((k + 24) > readnum){ //判断是否有不够一帧的数据剩余
			dataLen = readnum - k;
			for (int j=0; j<(readnum-k); j++){
				g_databuffer[j] = solvedata[k+j];}
			break;
		}
		if (k == 0 && dataLen != 0){
			for (int j=0; j<dataLen; j++){  //寻找帧头
				if (g_databuffer[j] == 0xeb){	//找到帧头	
					for (int m=j; m<dataLen; m++){	//开始连接帧
						storedata[m-j] = g_databuffer[m];}
					for (int m=dataLen; m<24; m++){ //将后面的帧接到前面
						storedata[m] = solvedata[m-dataLen];}
					if (storedata[0] == 0xeb && storedata[1] == 0xea ){	//判断连接的帧是否正确
						k = 1;
						dataLen = 0;
						SeperateFunc (&solvedata[0]);
						break;
		}}}}}
}
void SeperateFunc(unsigned char* finddata){
	static int i = 0;
	static int k = 0;
	double sprtdata[6];		//惯组系统
	static double savedata[60];
	/* 车载用惯组分离 */
	for (int j=0; j<6; j++){
		if(finddata[7+3*j] > 128){
			sprtdata[j] =  (double)((finddata[7+3*j] - 128)*256  + finddata[6+3*j])*5/65535;	
		}
		else{
			sprtdata[j] =  (double)((finddata[7+3*j] + 128)*256  + finddata[6+3*j])*5/65535;
		}
		resultData[j][resultCounter] = sprtdata[j];
	}
	resultCounter ++;
}
/* 暂时用来打印数据 */
void showInfo(char *msg){
	char temp[256];
	GetCtrlVal(panelHdl, MainPanel_TextMsg, temp);
	if (strlen(temp)>200) temp[0] = '\0';
	strcat(temp, "\n");
	strcat(temp, msg);
	SetCtrlVal(panelHdl, MainPanel_TextMsg, temp);
}




