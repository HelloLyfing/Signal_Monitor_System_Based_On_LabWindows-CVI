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
#include "MainPanel.h"

/*-- Error Check --*/
#ifndef errChk
#define errChk(fCall) if (error = (fCall), error < 0) \
{goto Error;} else
#endif

// Define.
#define READ_LENGTH 2400
#define MAX_THREADS 10
/* int functionType */
#define beginDataAcqType		0

static CmtThreadPoolHandle poolHandle = 0;
static CmtTSQHandle tsqHdl;

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

//Data Reading Writing Relative Vals
char loadFilePath[MAX_PATHNAME_LEN];
char writeFilePath[MAX_PATHNAME_LEN];
int bytesRead;
int writeTSQEndFlag = 0, readTSQEndFlag = 0;
int dataLen = 0;

//Ploting args
int plotBakgColor;
int plotLineColor;
int plotGridColor;
int plotSelectLineHdl = 0; //temp
//Showing stat,signal's showing status
int signalShowingStat[6] = {0};
char statString[30];

int cursorsValidNum = 0;

//Callback func
int graphsCallbackInstalled = 0; // >0,not Installed| >1,ChildPMode Installed| >2,TabMode Installed

unsigned char readData[READ_LENGTH] = {0x00};
int validMonNum; // valid Monitoring Number
unsigned char g_databuffer[256];

// Other args
int averSpaceV;
int averSpaceH;

// Analysis relative vals
double tempData[6][READ_LENGTH/24];
// Reading Ploting data vals
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
void showMenuPopup(int pHdl, int ctrlID, int xPoint, int yPoint);
void runAirplanModel(void);
void popupPanelForGraph(int pHdl);
void closePopupPanel(int pHdl);
void refreshDimmedStat(void);
void addLog(char *msg, int whichBox, int pHdl);
void drawAnalysisResult(int panel);
int seperateData (unsigned char *data);
int getGraphIndex(int PanelCtrl, int fromWhere);
char* getStatString(int ctrl);
void showFileInfo(int panel);
void CVICALLBACK readDataAndPlot(CmtTSQHandle queueHandle, unsigned int event,
        	int value, void *callbackData);
int CVICALLBACK receiveDataWriteToTSQ(void *functionData);
int CVICALLBACK showToast(void *functionData);
int CVICALLBACK graphCallbackFunc(int panelHandle, int controlID, int event, 
			void *callbackData, int eventData1, int eventData2);
void CVICALLBACK menuPopupCallback(int menuBarHandle, int menuItemID, 
	void *callbackData, int panelHandle);

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
	plotBakgColor = MakeColor(255, 205, 105); //Set background color of graphs
	plotGridColor = MakeColor(0, 158, 0);
	plotLineColor = MakeColor(249, 249, 50);
	popPanelIsPloting = 0;
	//Create a ThreadPool
	if(setupThreadPool()<0){
		MessagePopup("s", "Create thread pool error.");
	}
	//ChildPanel,TabPage、and Popup panel's Vals
	tabWidth = pWidth-extraRight-averSpaceH*3;
	tabHeight = pHeight - averSpaceV*3;
	//x,y坐标的标值范围
	xAxisRange[0] = 0.0, xAxisRange[1] = 9.0; //temp
	yAxisRange[0] = 0.0, yAxisRange[1] = 5.0;
	
	refreshDimmedStat();
}
/*---------------------------------------------------------------------------*/
// 程序退出时，清理占用的内存和垃圾 
/*---------------------------------------------------------------------------*/
static void cleanGarbage(void){
	pauseDataAcq();
	CmtDiscardTSQ(tsqHdl);
	CmtDiscardThreadPool (poolHandle);
	DiscardPanel(panelHdl);
}

/*---------------------------------------------------------------------------*/
// [Start] Button
/*---------------------------------------------------------------------------*/
int CVICALLBACK StartBtnCallback(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	switch (event){
		case EVENT_COMMIT:
			if (CPanels[0]>0 || TPanels[0]>0) break;
			initChildPanels();
			CmtNewTSQ (READ_LENGTH, sizeof(char), OPT_TSQ_DYNAMIC_SIZE, &tsqHdl);
			CmtInstallTSQCallback (tsqHdl, EVENT_TSQ_ITEMS_IN_QUEUE, READ_LENGTH,
				readDataAndPlot, NULL, CmtGetCurrentThreadID(), NULL);
			startDataAcq();
			break;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/ 
// In another thread, receives data and writes them into TSQ 
/*---------------------------------------------------------------------------*/
int CVICALLBACK receiveDataWriteToTSQ(void *functionData){
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
				resultCounter = 0; //用来重新 自增计数 resultData[x][resultCounter]中的采集次数
				seperateData(&data[0]);
				/* Plot the data. Doing any signal-processing/data-analysis stuff here */
                if(tabFlag==0){ //Child Panels Mode
					for(int i=0; i<validMonNum; i++){
						PlotStripChart(CPanels[i], PGraphs[i], resultData[i], READ_LENGTH/24, 0, 0, VAL_DOUBLE);
					}
				}else{ //Tab pages mode
					int i = 0;
					GetActiveTabPage (panelHdl, tabCtrl, &i);
					PlotStripChart(TPanels[i], PGraphs[i], resultData[i], READ_LENGTH/24, 0, 0, VAL_DOUBLE);
				}
				/*-temp popPanelIsPloting = 1;
				for(int i=0; i<validMonNum; i++){
					if( PopPanels[i]>0 ){
						PlotY(PopPanels[i], PopGraphs[i], resultData[i], READ_LENGTH/24, VAL_DOUBLE, VAL_THIN_LINE,
               				   VAL_EMPTY_SQUARE, VAL_SOLID, 1, plotLineColor);
					}
				}
				popPanelIsPloting = 0; temp-*/
				tsqRValue -= READ_LENGTH;
            }
            break;
    }//Switch()
}

/*---------------------------------------------------------------------------*/ 
// 辅助类函数 读取存储文件内容
/*---------------------------------------------------------------------------*/
void readDataFromFile(void){
	Fmt(loadFilePath, "c:\\Users\\Lyfing\\Desktop\\Perfect.dat");
	FILE *file = fopen(loadFilePath, "r");
	bytesRead = fread(&readData, sizeof(char), READ_LENGTH, file);
	fclose(file);
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
	CPanels[0] = createChildPanel("xAxls-Accelerator", 0);
	CPanels[1] = createChildPanel("yAxls-Accelerator", 1);
	CPanels[2] = createChildPanel("zAxls-Accelerator", 2);
	CPanels[3] = createChildPanel("xAxls-Gyro", 3);
	CPanels[4] = createChildPanel("yAxls-Gyro", 4);
	CPanels[5] = createChildPanel("zAxls-Gyro", 5);
	for(int i = 0; i<validMonNum; i++){
		DisplayPanel (CPanels[i]);
	}
	//在子Panel上创建图表，设置Graph属性，准备显示数据
	for(int i=0; i<validMonNum; i++){
		PGraphs[i] = NewCtrl(CPanels[i], CTRL_STRIP_CHART, "temp", 0,0);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_WIDTH, cWidth);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_HEIGHT, cHeight);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_PLOT_BGCOLOR, plotBakgColor);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_GRID_COLOR, plotGridColor);
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_EDGE_STYLE, VAL_FLAT_EDGE);		
		SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_SCROLL_MODE, VAL_CONTINUOUS);
		SetAxisScalingMode(CPanels[i], PGraphs[i], VAL_LEFT_YAXIS, VAL_MANUAL, yAxisRange[0], yAxisRange[1]);
		///SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_ENABLE_ZOOM_AND_PAN, 1);
		///SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_ZOOM_STYLE, VAL_ZOOM_XAXIS);
	}
	//canvasCtrl = createCanvas(CPanels[0]); //temp
	manageGraphCallbackFunc();
	
}

/*---------------------------------------------------------------------------*/
// 在Tab 模式 和 ChildPanels 模式之间切换
/*---------------------------------------------------------------------------*/	
void CVICALLBACK switchViewMode(int menuBar, int menuItem, void *callbackData,
		int panel){
	pauseDataAcq();
	if(tabFlag == 0){ //Child-Panel => Tab-Pages
		//创建Tab
		if(tabCtrl <=0){
			tabWidth = pWidth-extraRight-averSpaceH;
			tabHeight = pHeight - averSpaceV*2 + 12;
			tabCtrl = NewCtrl(panelHdl, CTRL_TABS, "", mHeight + 5, averSpaceH);
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
				//temp SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_NUM_CURSORS, 0);
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
// 右键弹出菜单(只对Graph类的控件有效)
/*---------------------------------------------------------------------------*/
void showMenuPopup(int pHdl, int ctrlID, int xPoint, int yPoint){
	int i = getGraphIndex(pHdl, 0);
	int menuBar = NewMenuBar(0);
	int menuIDForPop = NewMenu(menuBar, "" , -1);
	char openStr[25]="Open Analysis Window";
	char closeStr[25]="Close Analysis Window";
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
	int i = getGraphIndex(panelHandle, 0);
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

int getGraphIndex(int ctrl,int fromWhere){
	int i = 0;
	switch(fromWhere){
		case 0: //from Main Panel
			if(tabFlag == 0){ //Child Panel Mode
				for(i=0; i<validMonNum; i++){
					if(CPanels[i] == ctrl)
						return i;
				}
			}else{
				GetActiveTabPage (panelHdl, tabCtrl, &i);
				return i;
			}
			break;
		case 1: //from Analysis Panel
			for(i=0; i<validMonNum; i++){
				if(ctrl == PopPanels[i])
					return i;
			}
			break;
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
// Menu菜单的 所有回调函数
/*---------------------------------------------------------------------------*/
void CVICALLBACK menu_Exit(int menuBar, int menuItem, void *callbackData,int panel){
  	QuitUserInterface (0);
}

void CVICALLBACK menuBarLoadDataCallback(int menuBar, int menuItem, void *callbackData,int panel){
	int fileLoadPanel;
	if((fileLoadPanel = LoadPanel(0, "MainPanel.uir", ReadPanel)) < 0)
			showError(0,0, "Load ReadPanelPanel Error!");
	DisplayPanel(fileLoadPanel);
}
/*---------------------------------------------------------------------------*/
// 辅助类函数 新建并初始化线程池
/*---------------------------------------------------------------------------*/
static int setupThreadPool (void){
    int error = 0;
    errChk(CmtNewThreadPool(MAX_THREADS, &poolHandle));
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
		SetMenuBarAttribute (menuBar, menuItem, ATTR_ITEM_NAME, "Pause All");
		receiveFlag = 1;
		startDataAcq();
	}else{ 
		pauseDataAcq();
		SetMenuBarAttribute (menuBar, menuItem, ATTR_ITEM_NAME, "Resume All");
	}
}

/*---------------------------------------------------------------------------*/
// 退出程序 
/*---------------------------------------------------------------------------*/
int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	switch (event){
		case EVENT_COMMIT:
			QuitUserInterface(0);
			break;
	}
	return 0;
}
int CVICALLBACK PopupPanelCallBack(int panel, int event, void *callbackData, 
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
	                Fmt(ErrorMessage, "%s<RS232 error number %i", errorCode);
	                MessagePopup("RS232 Message", ErrorMessage);}
	            break;
	        case 0:
	            MessagePopup("RS232 Message", "No errors.");
	            break;
	        case -2:
	            Fmt(ErrorMessage, "%s", 
					"Invalid port number (must be in the range 1 to 8).");
	            MessagePopup("RS232 Message", ErrorMessage);
	            break;
	        case -3 :
	            Fmt(ErrorMessage, "%s", 
					"No port is open.\n Check COM Port setting in Configure.");
	            MessagePopup("RS232 Message", ErrorMessage);
	            break;
	        case -99 :
	            Fmt(ErrorMessage, "%s", "Timeout error.\n\n"
	                 "Either increase timeout value,\n"
	                 "       check COM Port setting, or\n"
	                 "       check device.");
	            MessagePopup("RS232 Message", ErrorMessage);
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
	}//for()
	int j=0;
	for(int i=0; i<READ_LENGTH; i++){
		j = (int)Random(0.0, 5.0);
		if(0x00 == data[i]){
			if(0 == i%24)
				data[i] = 0xeb;	
			else if(1 ==i%24 )
				data[i] = 0xea;
			else{	
				switch(j){
					case 1:
						data[i] = 0x8f;
						break;
					case 2:
						data[i] = 0x7f;
						break;
					case 3:
						data[i] = 0x90;
						break;
					default:
						data[i] = 0x86;
				}//switch()
			}
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

/* 用来在窗口打印Log信息 */
void addLog(char *msg, int whichBox, int pHdl){
	char msgTemp[256];
	Fmt(msgTemp,"[%s] -%c ", TimeStr(), 62);
	strcat(msgTemp, msg);
	strcat(msgTemp, "\n");
	if(0 == whichBox){ //Show Log on Main Panel's Log Box
		InsertTextBoxLine(panelHdl, MainPanel_MainLogBox, 0, msgTemp);
	}else{
		int i = getGraphIndex(pHdl, 1);
		InsertTextBoxLine(PopPanels[i], PopupPanel_PopLogBox, 0, msgTemp);
	}
}

/*-temp int CVICALLBACK showLogInfoIn2ndThread(void *functionData){
	char title[50], *msg;
	msg = (char *)functionData;
	return 0;
} temp-*/

int CVICALLBACK PopQuitBtn(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			closePopupPanel(panel);
			break;
	}
	return 0;
}

/* 下拉控件-原信号-回调函数 */
int CVICALLBACK ring1Callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

/* 下拉控件-加窗类型-回调函数 */
int CVICALLBACK windowTypeCallback(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	switch(event){
		case EVENT_COMMIT:
			drawAnalysisResult(panel);
			
			addLog("加窗类型变为", 1, panel);
			break;
	}
	return 0;
}

/* 下拉控件-信号分析处理-回调函数 */
int CVICALLBACK analysisCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	int type;
	switch (event){
		case EVENT_COMMIT:
			drawAnalysisResult(panel);
			break;
	}
	return 0;
}

/* 多个下拉控件-共同调用的函数 */
void drawAnalysisResult(int panel){
	int type_2, type_3;
	int i = getGraphIndex(panel, 1);
	int pointsNum =  READ_LENGTH/24;
	double spectrum[pointsNum], df;
	GetCtrlVal(panel, PopupPanel_RingWindowType, &type_2);
	GetCtrlVal(panel, PopupPanel_RingFFT, &type_3);	
	DeleteGraphPlot(panel, PopupPanel_PopGraph2, -1, VAL_IMMEDIATE_DRAW);
	DeleteGraphPlot(panel, PopupPanel_PopGraph3, -1, VAL_IMMEDIATE_DRAW);
	//-temp signal1 (panel, PANEL_RING_SIGNALTYPE, EVENT_COMMIT, NULL, 0, 0);   
	//获得窗函数类型
	switch(type_2){
		//对信号加窗
		case 0://加 汉宁窗
			HanWin(tempData[i], pointsNum); break;
		case 1://加 海明窗
			HamWin(tempData[i], pointsNum); break;
		case 2://加 布拉克曼窗
			BkmanWin(tempData[i], pointsNum); break;
		case 3://加 指数窗
			ExpWin(tempData[i], pointsNum, 0.01); break;
		case 4://加 高斯窗
			GaussWin(tempData[i], pointsNum, 0.2); break;
		case 5://加 三角窗
			TriWin(tempData[i], pointsNum);break;
	}//switch()
	PlotY(panel, PopupPanel_PopGraph2, tempData[i], pointsNum, 
			VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
	AutoPowerSpectrum (tempData[i], pointsNum, 1/100, spectrum, &df);
    /*-temp switch(scaling){
        case 1:
            for (j=0; j<(pointsNum/2); j++) 
                spectrum[j] = 20*(log10(spectrum[j]));
            break;
    } temp-*/
    PlotY(panel, PopupPanel_PopGraph3, spectrum, pointsNum/2,
          VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, (i)?VAL_YELLOW:VAL_RED);
}

/* PopupPanel */
void popupPanelForGraph(int pHdl){
	int i = getGraphIndex(pHdl, 0); //Need a index to find one of the six graphs.
	char title[50];
	for(int j=0; j<(sizeof(resultData[i])/sizeof(double)); j++)
		tempData[i][j] = resultData[i][j];
	// 从uir文件中加载 信号分析面板
	if(PopPanels[i] <=0 ){
		if ((PopPanels[i] = LoadPanel(0, "MainPanel.uir", PopupPanel)) < 0)
			showError(0,0, "Load PopupPanel Error!");
		DisplayPanel(PopPanels[i]);
		PopGraphs[i] = PopupPanel_PopGraph1;
		GetPanelAttribute(pHdl, ATTR_TITLE, title);
		SetPanelAttribute(PopPanels[i], ATTR_TITLE, title);
		SetCtrlAttribute(PopPanels[i], PopGraphs[i], ATTR_GRID_COLOR, plotGridColor);
		SetCtrlAttribute(PopPanels[i], PopGraphs[i], ATTR_GRAPH_BGCOLOR, VAL_TRANSPARENT);
		//temp SetAxisScalingMode(PopPanels[i], PopGraphs[i], VAL_LEFT_YAXIS, VAL_MANUAL, yAxisRange[0], yAxisRange[1]);
		PopGPlots[i] = PlotY(PopPanels[i], PopGraphs[i], tempData[i], READ_LENGTH/24, 
							 VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,1, plotLineColor);
	}else{
		SetActivePanel(PopPanels[i]);
	}
	
}

int CVICALLBACK PopSwitcherCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK PopLogInfoBtn (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK chooseFileBtnCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
            if (FileSelectPopup ("", "", "", "Select File to Load",
               		VAL_OK_BUTTON, 0, 0, 1, 1, loadFilePath) <= 0) return 0;
            showFileInfo(panel);
			break;
	}
	return 0;
}

void showFileInfo(int panel){
	char msg[250],temp[250];
	msg[0] = '\0';
	strcat(msg, loadFilePath);
	strcat(msg, ";\n\n");
	long len; 
	GetFileSize(loadFilePath, &len);
	Fmt(temp, "文件大小： %d Bytes \n\n", len);
	strcat(msg, temp);
	double timeFrequency;GetCtrlVal(panel, ReadPanel_SetReadSpeedRing, &timeFrequency);
	double timeLast = len/READ_LENGTH*timeFrequency;
	Fmt(temp, "预计持续时长： %f s", timeLast);
	strcat(msg, temp);
	SetCtrlVal(panel, ReadPanel_FileInfoDetail, msg);
}
int CVICALLBACK finishChooseFileBtnCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			DiscardPanel(panel);
			configComFlag = 0;
			refreshDimmedStat();
			break;
	}
	return 0;
}

/* 信号分析窗口-原始信号图表-回调函数 */
int CVICALLBACK PopGraph1Callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_LEFT_DOUBLE_CLICK:
			int i = getGraphIndex(panel, 1);
			for(int j=0; j<(sizeof(resultData[i])/sizeof(double)); j++)
				tempData[i][j] = resultData[i][j];
			DeleteGraphPlot(panel, PopGraphs[i], -1, VAL_DELAYED_DRAW);
			PopGPlots[i] = PlotY(PopPanels[i], PopGraphs[i], tempData[i], READ_LENGTH/24, 
							 VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,1, plotLineColor);
			
			break;
	}
	return 0;
}

/* 读取文件面板-下拉控件-读取速度 */
int CVICALLBACK readSpeedCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			if(strlen(loadFilePath) <= 2)
				break;
			showFileInfo(panel);
			break;
	}
	return 0;
}
