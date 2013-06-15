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
FILE *loadFP; FILE *writeFP;
long loadFPP, writeFPP; //FPP=File Pointer Position
double fileLoadSpeed;
int bytesRead;
int writeTSQEndFlag = 0, readTSQEndFlag = 0;
int dataLen = 0;
int dataSrc; //0,Device Port| 1,Read File| 2, Simulating Data
int isDataAcqRunning;
//Ploting args
int plotBakgColor;
int plotLineColor;
int plotGridColor;

//Showing stat,signal's showing status
int signalShowingStat[6];
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
void showError (char* msg);
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
void showLogBoxRightClickMenu(int pHdl, int ctrlID, int xPoint, int yPoint);
void runAirplanModel(void);
void popupPanelForGraph(int pHdl);
void closePopupPanel(int pHdl);
void refreshDimmedStat(void);
void addLog(char *msg, int whichBox, int pHdl);
void drawAnalysisResult(int panel);
void showFileInfo(int panel);
void initDirectorySets(void);
void stopDataAcq(void);

char* getStatString(int ctrl);
int seperateData (unsigned char *data);
int getGraphIndex(int PanelCtrl, int fromWhere);

void CVICALLBACK readDataAndPlot(CmtTSQHandle queueHandle, unsigned int event,
        	int value, void *callbackData);
int CVICALLBACK receiveDataWriteToTSQ(void *functionData);
int CVICALLBACK showToast(void *functionData);
int CVICALLBACK graphCallbackFunc(int panelHandle, int controlID, int event, 
			void *callbackData, int eventData1, int eventData2);
void CVICALLBACK menuPopupCallback(int menuBarHandle, int menuItemID, 
	void *callbackData, int panelHandle);
void CVICALLBACK logBoxRightClickMenuCallback(int menuBarHandle, int menuItemID, 
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
	plotGridColor = MakeColor(0, 18, 0);
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
	yAxisRange[0] = -3.0, yAxisRange[1] = 3.0;
	
	dataSrc = 0;
	for(int i=0; i<validMonNum; i++)
		signalShowingStat[i] = 1;
	//init the necessary Directory this system needs
	initDirectorySets();
	refreshDimmedStat();
	isDataAcqRunning = 0;
}
/*---------------------------------------------------------------------------*/
// 程序退出时，清理占用的内存和垃圾 
/*---------------------------------------------------------------------------*/
static void cleanGarbage(void){
	pauseDataAcq();
	stopDataAcq();
	CmtDiscardTSQ(tsqHdl);
	CmtDiscardThreadPool(poolHandle);
	DiscardPanel(panelHdl);
}

/*---------------------------------------------------------------------------*/
// [Control Showing] Button
/*---------------------------------------------------------------------------*/
int CVICALLBACK ControlShowingBtnCallback(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	switch (event){
		case EVENT_COMMIT:
			if(CPanels[0] <=0 && TPanels[0]<=0){
				initChildPanels();
				CmtNewTSQ(READ_LENGTH, sizeof(char), OPT_TSQ_DYNAMIC_SIZE, &tsqHdl);
				CmtInstallTSQCallback (tsqHdl, EVENT_TSQ_ITEMS_IN_QUEUE, READ_LENGTH,
					readDataAndPlot, NULL, CmtGetCurrentThreadID(), NULL);
			}
			int i; GetCtrlVal(panel, MainPanel_Signal_ID_List, &i);
			if(i == 6){
				for(int j=0; j<validMonNum; j++)
					signalShowingStat[j] = 1;//temp signalShowingStat[j]==1?0:1;
			}else{
				signalShowingStat[i] = signalShowingStat[i]==1?0:1;
			}
			if( isDataAcqRunning == 0)
				startDataAcq();
			break;
	}//switch()
	return 0;
}

/*---------------------------------------------------------------------------*/ 
// In another thread, receives data and writes them into TSQ 
/*---------------------------------------------------------------------------*/
int CVICALLBACK receiveDataWriteToTSQ(void *functionData){
	dataSrc = 0;
	int storeFileFlag = 0;
	GetCtrlVal(panelHdl, MainPanel_StoreTheData_Switch, &storeFileFlag);
	if(0 == dataSrc){ //read data from [device port]
		//if(OpenComConfig(comPort, "", baudRate, parity, 
		//				 dataBits, stopBits, inQueueSize, outQueueSize) <0)
		//	showError("Failed to open RS232 port!");
		if(storeFileFlag > 0){ 
			if( writeFPP >0){
				writeFP = fopen(writeFilePath, "a");
				if(writeFP == NULL) 
					showError("Open WriteFile Failed");
				fseek(writeFP, writeFPP, SEEK_SET);
			}else{	
				SYSTEMTIME localTime;
				char dirName[MAX_PATHNAME_LEN];
				char fileName[200];
				GetProjectDir(dirName);
				strcat(dirName, "\\DataStorage");
				GetLocalTime(&localTime);
				sprintf(fileName, "%04d-%d-%d,%d_%02d_%02d.dat", 
						localTime.wYear, localTime.wMonth, localTime.wDay,
						localTime.wHour, localTime.wMinute, localTime.wSecond);
				MakePathname(dirName, fileName, writeFilePath);
				writeFP = fopen(writeFilePath, "a");
				if(writeFP == NULL) showError("Open WriteFile Failed");
			}
		}//if(storeFileFlag)
	}else if(1 == dataSrc){ //read data from [file]
		loadFP = fopen(loadFilePath, "r");
		if(loadFP == NULL){ 
			showError("Load File Failed");
			return -1;
		}
		fseek(loadFP, loadFPP, SEEK_SET);
		printf("loadFPP=%d", loadFPP);
	}else{ //[simulating data]
	}
	GetCtrlVal(panelHdl, MainPanel_StoreTheData_Switch, &storeFileFlag);
	while(receiveFlag){
		writeTSQEndFlag = 1;
		switch(dataSrc){
			case 0:
				//bytesRead = ComRd(comPort, readData, READ_LENGTH);
				generateSimulateData(readData);
				if(storeFileFlag > 0)
					fwrite(readData, sizeof(char), READ_LENGTH, writeFP);
				Delay(0.1);
				break;
			case 1:
				Delay(fileLoadSpeed);
				if(!feof(loadFP)){
					bytesRead = fread(readData, sizeof(char), READ_LENGTH, loadFP);
				}else{
					receiveFlag = 0;
					MessagePopup("Reach The End", "All data in this file has been read !");
				}
				break;
			case 2:
				generateSimulateData(readData);
				Delay(0.1);
				break;
		}//switch()
		CmtWriteTSQData(tsqHdl, &readData, READ_LENGTH, TSQ_INFINITE_TIMEOUT, NULL);
		writeTSQEndFlag = 0;
	}//while()
	switch(dataSrc){
		case 0:
			//CloseCom(comPort);
			if(storeFileFlag >0){
				if(!feof(writeFP))
					writeFPP = ftell(writeFP);
				fclose(writeFP);
				writeFP = NULL;
			}
			break;
		case 1:
			if(!feof(loadFP))
				loadFPP = ftell(loadFP);
			printf("loadFPP=%d", loadFPP);
			fclose(loadFP);
			loadFP = NULL;
			break;
	}//switch()
	addLog("End-Acq", 0, panelHdl);
	return 0;
}

/* Init the Directory Sets this System needs */
void initDirectorySets(void){
	char dirName[MAX_PATHNAME_LEN];
	char DataDirPath[MAX_PATHNAME_LEN];
	char LogDirPath[MAX_PATHNAME_LEN];
	GetProjectDir(dirName);
	sprintf(DataDirPath, "%s\\DataStorage", dirName);
	sprintf(LogDirPath, "%s\\Logs", dirName);
	printf("%s\n%s", DataDirPath, LogDirPath);
	int oldValue;
	oldValue = SetBreakOnLibraryErrors(0);
	int isDataDirExisted = MakeDir(DataDirPath);
	int isLogDirExisted = MakeDir(LogDirPath);
	SetBreakOnLibraryErrors (oldValue);
	if(isDataDirExisted == 0)
		addLog("Successfully create the Directory:\n\"DataStorage\" and \"Logs\"", 0, panelHdl);
	else if( isDataDirExisted == -9)
		addLog("\"DataStorage\" and \"Logs\" \nDirectory has already existed!", 0, panelHdl);
	
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
				resultCounter = 0; //用来重新自增计数 resultData[x][resultCounter]中的采集次数
				seperateData(&data[0]);
				/* Plot the data. */
                if(tabFlag==0){ //Child Panels Mode
					for(int i=0; i<validMonNum; i++){
						if(1 == signalShowingStat[i]){
							PlotStripChart(CPanels[i], PGraphs[i], resultData[i], READ_LENGTH/24, 0, 0, VAL_DOUBLE);
						}
					}
				}else{ //Tab pages mode
					int i = 0;
					GetActiveTabPage (panelHdl, tabCtrl, &i);
					if(1 == signalShowingStat[i])
						PlotStripChart(TPanels[i], PGraphs[i], resultData[i], READ_LENGTH/24, 0, 0, VAL_DOUBLE);
				}
				tsqRValue -= READ_LENGTH;
            }
            break;
    }//Switch()
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
// 暂停、开始及结束数据采集
/*---------------------------------------------------------------------------*/	
void startDataAcq(void){
	if(tsqHdl > 0)
		CmtFlushTSQ(tsqHdl, TSQ_FLUSH_ALL, NULL);
	receiveFlag = 1;
	runSecondThread(beginDataAcqType, NULL);
	addLog("Begin Signal Acq !", 0, panelHdl);
	isDataAcqRunning = 0;
}
void pauseDataAcq(void){
	receiveFlag = 0;
	//如果 write to TSQ process 没有完成，则等待完成
	while( writeTSQEndFlag != 0)
		Delay(0.005);
	addLog("Pause Signal Acq !", 0, panelHdl);
	isDataAcqRunning = 0;
}
void stopDataAcq(void){
	receiveFlag = 0;
	while( writeTSQEndFlag != 0)
		Delay(0.005);
	writeFPP = 0;
	addLog("Stop Signal Acq !", 0, panelHdl);
	isDataAcqRunning = 0;
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
		//SetAxisScalingMode(CPanels[i], PGraphs[i], VAL_LEFT_YAXIS, VAL_MANUAL, yAxisRange[0], yAxisRange[1]);
		//SetCtrlAttribute(CPanels[i], PGraphs[i], ATTR_YFORMAT,VAL_FLOATING_PT_FORMAT);
	}
	manageGraphCallbackFunc();
	
}

/*---------------------------------------------------------------------------*/
// 在Tab 模式 和 ChildPanels 模式之间切换
/*---------------------------------------------------------------------------*/	
void CVICALLBACK switchViewMode(int menuBar, int menuItem, void *callbackData,
		int panel){
	int tempReceiveFlag = receiveFlag;
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
			//SetAxisScalingMode(TPanels[i], PGraphs[i], VAL_LEFT_YAXIS, VAL_MANUAL, yAxisRange[0], yAxisRange[1]);
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
	receiveFlag = tempReceiveFlag;
	if(receiveFlag == 0){
		if(tabFlag == 0){
			for(int i=0; i<validMonNum; i++)
				PlotStripChart(CPanels[i], PGraphs[i], resultData[i], READ_LENGTH/24, 0, 0, VAL_DOUBLE);
		}else{
			PlotStripChart(TPanels[0], PGraphs[0], resultData[0], READ_LENGTH/24, 0, 0, VAL_DOUBLE);
		}
	}else{
		startDataAcq();
	}
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
// A Manager to Install or Graphs' call back func.
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
	int i = getGraphIndex(panelHandle, 0);
	int x = eventData2, y = eventData1;
	switch(event){
		case EVENT_LEFT_CLICK:
			signalShowingStat[i]= signalShowingStat[i]==0?1:0;		
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			signalShowingStat[i]= signalShowingStat[i]==0?1:0;
			popupPanelForGraph(panelHandle);
			break;
		case EVENT_RIGHT_CLICK:
			showMenuPopup(pHdl, ctrlID, x, y);
			break;
	}//switch()
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
	char pauseStr[20] = "Pause";
	char startStr[20] = "Start";
	if(tabFlag == 0){
		NewMenuItem(menuBar, menuIDForPop, signalShowingStat[i]>0?pauseStr:startStr, -1, 0, menuPopupCallback, NULL);
		NewMenuItem(menuBar, menuIDForPop, PopPanels[i]>0?closeStr:openStr, -1, 0, menuPopupCallback, NULL);
	}else{
		NewMenuItem(menuBar, menuIDForPop, signalShowingStat[i]>0?pauseStr:startStr, -1, 0, menuPopupCallback, NULL);
		NewMenuItem(menuBar, menuIDForPop, PopPanels[i]>0?closeStr:openStr, -1, 0, menuPopupCallback, NULL);
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
			signalShowingStat[i] = signalShowingStat[i]>0?0:1;
			break;
		case 4:
			if(PopPanels[i]>0)
				closePopupPanel(PopPanels[i]);
			else
				popupPanelForGraph(panelHandle);
			break;
	}
}

int getGraphIndex(int pHdl,int fromWhere){
	int i = 0;
	switch(fromWhere){
		case 0: //from Main Panel
			if(tabFlag == 0){ //Child Panel Mode
				for(i=0; i<validMonNum; i++){
					if( pHdl == CPanels[i])
						return i;
				}
			}else{
				GetActiveTabPage(panelHdl, tabCtrl, &i);
				return i;
			}
			break;
		case 1: //from Analysis Panel
			for(i=0; i<validMonNum; i++){
				if(pHdl == PopPanels[i])
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
			//dataSrc = 0;
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
	Ctrls[0] = MainPanel_ControlBtn;
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
			showError("Load ReadPanelPanel Error!");
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
	pauseDataAcq();
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
void showError(char* msg){
    char ErrorMsg[256];
	strcat(ErrorMsg, "An error has occured,here is the detail:\n");
	strcat(ErrorMsg, msg);
    MessagePopup("Error !", ErrorMsg);
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
void SolveFunc(unsigned char* solvedata, int readnum){
	unsigned char storedata[24];
	for (int k=0; k<readnum; k++){
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
		if(k == 0 && dataLen != 0){
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
	int i; 
	GetCtrlVal(panel, PopupPanel_RingWindowType, &i);
	switch(event){
		case EVENT_COMMIT:
			drawAnalysisResult(panel);
			char msg[256];msg[0] = '\0';
			sprintf(msg, "添加-");
			switch(i){
				case 0:
					strcat(msg, "海宁窗");
					break;
				case 1:
					strcat(msg, "海明窗");
					break;
				case 2:
					strcat(msg, "布拉克曼窗");
					break;
			  	case 3:
					strcat(msg, "指数窗");
					break;
				case 4:
					strcat(msg, "高斯窗");
					break;
				case 5:
					strcat(msg, "三角窗");
					break;
			}
			strcat(msg, "-函数");
			addLog(msg, 1, panel);
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
			char msg[256];msg[0] = '\0';
			sprintf(msg, "添加频谱分析！");
			addLog(msg, 1, panel);
			break;
	}
	return 0;
}

/* 多个下拉控件-共同调用的函数 */
void drawAnalysisResult(int panel){
	int i = getGraphIndex(panel, 1);
	double tempDataDuplicate1[READ_LENGTH/24], tempDataDuplicate2[READ_LENGTH/24];
	int type_2, type_3, isThroughWindow;
	int pointsNum =  READ_LENGTH/24;
	double spectrum[pointsNum], df;
	for(int j=0; j<(sizeof(tempData[i])/sizeof(double)); j++){
		tempDataDuplicate1[j] = tempData[i][j];
		tempDataDuplicate2[j] = tempData[i][j];
	}
	GetCtrlVal(panel, PopupPanel_RingWindowType, &type_2);
	GetCtrlVal(panel, PopupPanel_RingFFT, &type_3);
	GetCtrlVal(panel, PopupPanel_PopupSwitcher, &isThroughWindow);	
	DeleteGraphPlot(panel, PopupPanel_PopGraph2, -1, VAL_IMMEDIATE_DRAW);
	DeleteGraphPlot(panel, PopupPanel_PopGraph3, -1, VAL_IMMEDIATE_DRAW);
	//-temp signal1 (panel, PANEL_RING_SIGNALTYPE, EVENT_COMMIT, NULL, 0, 0);   
	//获得窗函数类型
	switch(type_2){ //对信号加窗
		case 0://加 汉宁窗
			HanWin(tempDataDuplicate1, pointsNum); break;
		case 1://加 海明窗
			HamWin(tempDataDuplicate1, pointsNum); break;
		case 2://加 布拉克曼窗
			BkmanWin(tempDataDuplicate1, pointsNum); break;
		case 3://加 指数窗
			ExpWin(tempDataDuplicate1, pointsNum, 0.01); break;
		case 4://加 高斯窗
			GaussWin(tempDataDuplicate1, pointsNum, 0.2); break;
		case 5://加 三角窗
			TriWin(tempDataDuplicate1, pointsNum);break;
	}//switch()
	PlotY(panel, PopupPanel_PopGraph2, tempData[i], pointsNum, 
			VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);
	if(isThroughWindow == 1){
		AutoPowerSpectrum(tempDataDuplicate1, pointsNum, 1/100, spectrum, &df);
	}else{
		AutoPowerSpectrum(tempDataDuplicate2, pointsNum, 1/100, spectrum, &df);
	}
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
			showError("Load PopupPanel Error!");
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
			drawAnalysisResult(panel);
			break;
	}
	return 0;
}

int CVICALLBACK PopLogInfoBtn (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			ResetTextBox(panel, PopupPanel_PopLogBox, "");
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
            SetCtrlVal(panel, ReadPanel_PathString, loadFilePath);
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
	double timeLast, temp1 = len/READ_LENGTH;
	if(temp1 <= 1)
		timeLast = 1;
	else
		timeLast = (temp1-1)*timeFrequency;
	Fmt(temp, "预计持续时长： %f s", timeLast);
	strcat(msg, temp);
	SetCtrlVal(panel, ReadPanel_FileInfoDetail, msg);
}
int CVICALLBACK finishChooseFileBtnCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event){
		case EVENT_COMMIT:
			configComFlag = 0;
			refreshDimmedStat();
			//Set the data source to ReadFile
			dataSrc = 1;
			loadFPP = 0;
			GetCtrlVal(panel, ReadPanel_SetReadSpeedRing, &fileLoadSpeed);
			DiscardPanel(panel);
			break;
	}
	return 0;
}

/* 信号分析窗口-原始信号图表-回调函数 */
int CVICALLBACK PopGraph1Callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event){
		case EVENT_LEFT_DOUBLE_CLICK:
			int i = getGraphIndex(panel, 1);
			for(int j=0; j<(sizeof(resultData[i])/sizeof(double)); j++)
				tempData[i][j] = resultData[i][j];
			DeleteGraphPlot(panel, PopGraphs[i], -1, VAL_DELAYED_DRAW);
			PopGPlots[i] = PlotY(PopPanels[i], PopGraphs[i], tempData[i], READ_LENGTH/24, 
							 VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,1, plotLineColor);
			
			addLog("成功刷新一组数据!", 1, panel);
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

/* 主面板-信号选择-下拉列表-回调函数 */
int CVICALLBACK chooseASignal_List_Callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			int i; GetCtrlVal(panel, control, &i);
			int inTheSameStat = 0;
			int _0Count=0, _1Count=0;
			if(i == 6){
				SetCtrlAttribute(panel, MainPanel_ControlBtn, ATTR_LABEL_TEXT, "Start All");
				break;
			}
			if(0 == signalShowingStat[i])
				SetCtrlAttribute(panel, MainPanel_ControlBtn, ATTR_LABEL_TEXT, "Start");
			else
				SetCtrlAttribute(panel, MainPanel_ControlBtn, ATTR_LABEL_TEXT, "Pause");
			break;
	}
	return 0;
}

void CVICALLBACK Menu_StartAll_Callback (int menuBar, int menuItem, void *callbackData,
		int panel){
	if(PGraphs[0] <= 0) return;
	for(int i=0; i<validMonNum; i++){
		signalShowingStat[i] = 1;
	}
	startDataAcq();
}

void CVICALLBACK Menu_StopAcq_Callback (int menuBar, int menuItem, void *callbackData,
		int panel){
	stopDataAcq();
}

int CVICALLBACK MainLog_Callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_RIGHT_CLICK:
			showLogBoxRightClickMenu(panel, control, eventData2, eventData1);
			break;
	}
	return 0;
}

void showLogBoxRightClickMenu(int pHdl, int ctrlID, int xPoint, int yPoint){
	int menuBar = NewMenuBar(0);
	int menuIDForPop = NewMenu(menuBar, "" , -1);
	NewMenuItem(menuBar, menuIDForPop, "Save Log to File", -1, 0, logBoxRightClickMenuCallback, NULL);
	NewMenuItem(menuBar, menuIDForPop, "Clear Log Console", -1, 0, logBoxRightClickMenuCallback, NULL);
	RunPopupMenu(menuBar, menuIDForPop, 
		pHdl, yPoint, xPoint, 0, 0, 0, 0);
}

void CVICALLBACK logBoxRightClickMenuCallback(int menuBarHandle, int menuItemID, 
			void *callbackData, int panelHandle){
	switch(menuItemID){
		case 3:
			SYSTEMTIME localTime;
			char dirName[MAX_PATHNAME_LEN];
			char filePath[MAX_PATHNAME_LEN]; 
			char fileName[200]; fileName[0]='\0';
			GetProjectDir(dirName);
			strcat(dirName, "\\Logs");
			GetLocalTime(&localTime);
			sprintf(fileName, "Log,%04d-%d-%d,%d_%02d_%02d.txt", 
					localTime.wYear, localTime.wMonth, localTime.wDay,
					localTime.wHour, localTime.wMinute, localTime.wSecond);
			MakePathname(dirName, fileName, filePath);
			FILE *fp = fopen(filePath, "w");
			int len=0;
			GetCtrlAttribute(panelHdl, MainPanel_MainLogBox, ATTR_STRING_TEXT_LENGTH, &len);
			char tempLog[10000];
			GetCtrlVal(panelHdl, MainPanel_MainLogBox, tempLog);
			fwrite(tempLog, sizeof(char), len, fp);
			fclose(fp);
			ResetTextBox(panelHdl, MainPanel_MainLogBox, "Logs have been saved!");
			break;
		case 4:
			ResetTextBox(panelHdl, MainPanel_MainLogBox, "");
			break;
	}
}
