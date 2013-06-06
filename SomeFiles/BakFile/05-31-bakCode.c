// Airplane Part
void runAirplaneModel(void){
	if(panelHdl2 <= 0) {
		runFuncInAnotherThread(showAirplaneModel);
	}else{
		SetActivePanel(panelHdl2);
	}
}
int CVICALLBACK showAirplaneModelll(void *functionData){
	if ((panelHdl2 = LoadPanel(0, "MainPanel.uir", Panel2)) < 0)
	showError(0,0, "Load Error");
	DisplayPanel(panelHdl2);
/*	CAObjHandle plotsHandle = 0;
	GetObjHandleFromActiveXCtrl (panelHdl2, Panel2_3DGraph, &graph3DHdl);
    CW3DGraphLib__DCWGraph3DGetPlots(graph3DHdl, NULL, &plotsHandle);
    CW3DGraphLib_CWPlots3DItem (plotsHandle, NULL, CA_VariantInt(1), &graph3DPlotHdl);
*/
	return 0;
}

static HRESULT Initialize(void){
    HRESULT     hr;
/*  GetObjHandleFromActiveXCtrl(gPanel, PANEL_SURFACE_GRAPH, &gSurfaceGraph);
    GetObjHandleFromActiveXCtrl(gPanel, PANEL_CONTOUR_GRAPH, &gContourGraph);
    hrChk (RefreshControls());
Error:
*/
	return hr;
}
static HRESULT RefreshControls(void){
    HRESULT         hr;
/*    CAObjHandle     plots = 0, labels = 0, ticks = 0, plot = 0;
    short           numPlots;
    VBOOL           autoscale, inverted, logarithmic, 
                    normal, opposite, inside, outside,
                    major, minor;
    VARIANT         vtMin, vtMax;
    double          minimum, maximum;
    char            *caption = 0;

    CA_VariantSetEmpty(&vtMin);
    CA_VariantSetEmpty(&vtMax);
    ///Autoscale Plot
    ///hrChk(CW3DGraphLib__DCWGraph3DGetPlots(graph3DHdl, NULL, &plots));
    ///hrChk(CW3DGraphLib_CWPlots3DGetCount(plots, NULL, &numPlots));
    if (numPlots > 0){
        ///hrChk (CW3DGraphLib_CWPlots3DItem(plots, NULL, CA_VariantShort(1), &plot));
        ///hrChk (CW3DGraphLib_CWPlot3DGetAutoScale(plot, NULL, &autoscale));
        ///SetCtrlVal(panelHdl2, Panel2_AUTOSCALE_PLOT, autoscale != VFALSE); }
    // Autoscale Axis 
    hrChk (CW3DGraphLib_CWAxis3DGetAutoScale(gAxis, NULL, &autoscale));
    SetCtrlVal(gPanel, PANEL_AUTOSCALE, autoscale != VFALSE);
    // Minimum 
    hrChk (CW3DGraphLib_CWAxis3DGetMinimum(gAxis, NULL, &vtMin));
    hrChk (CA_VariantGetDouble(&vtMin, &minimum));
    SetCtrlVal(gPanel, PANEL_MIN, minimum);
    // Maximum 
    hrChk (CW3DGraphLib_CWAxis3DGetMaximum(gAxis, NULL, &vtMax));
    hrChk (CA_VariantGetDouble(&vtMax, &maximum));
    SetCtrlVal(gPanel, PANEL_MAX, maximum);
    // Inverted 
    hrChk (CW3DGraphLib_CWAxis3DGetInverted(gAxis, NULL, &inverted));
    SetCtrlVal(gPanel, PANEL_INVERTED, inverted != VFALSE);
    // Log 
    hrChk (CW3DGraphLib_CWAxis3DGetLog(gAxis, NULL, &logarithmic));
    SetCtrlVal(gPanel, PANEL_LOG, logarithmic != VFALSE);
    // Caption 
    hrChk (CW3DGraphLib_CWAxis3DGetCaption(gAxis, NULL, &caption));
    SetCtrlVal(gPanel, PANEL_CAPTION, caption);
    hrChk (CW3DGraphLib_CWAxis3DGetCaptionNormal(gAxis, NULL, &normal));
    SetCtrlVal(gPanel, PANEL_NORMAL_CAPTION, normal != VFALSE);
    hrChk (CW3DGraphLib_CWAxis3DGetCaptionOpposite(gAxis, NULL, &opposite));
    SetCtrlVal(gPanel, PANEL_OPPOSITE_CAPTION, opposite != VFALSE);
    //* Labels 
    hrChk (CW3DGraphLib_CWAxis3DGetLabels(gAxis, NULL, &labels));
    hrChk (CW3DGraphLib_CWLabels3DGetNormal(labels, NULL, &normal));
    SetCtrlVal(gPanel, PANEL_NORMAL_LABELS, normal != VFALSE);
    hrChk (CW3DGraphLib_CWLabels3DGetOpposite(labels, NULL, &opposite));
    SetCtrlVal(gPanel, PANEL_OPPOSITE_LABELS, opposite != VFALSE);
    // Ticks 
    hrChk (CW3DGraphLib_CWAxis3DGetTicks(gAxis, NULL, &ticks));
    hrChk (CW3DGraphLib_CWTicks3DGetNormal(ticks, NULL, &normal));
    SetCtrlVal(gPanel, PANEL_NORMAL_TICKS, normal != VFALSE);
    hrChk (CW3DGraphLib_CWTicks3DGetOpposite(ticks, NULL, &opposite));
    SetCtrlVal(gPanel, PANEL_OPPOSITE_TICKS, opposite != VFALSE);
    hrChk (CW3DGraphLib_CWTicks3DGetInside(ticks, NULL, &inside));
    SetCtrlVal(gPanel, PANEL_INSIDE_TICKS, inside != VFALSE);
    hrChk (CW3DGraphLib_CWTicks3DGetOutside(ticks, NULL, &outside));
    SetCtrlVal(gPanel, PANEL_OUTSIDE_TICKS, outside != VFALSE);
    hrChk (CW3DGraphLib_CWTicks3DGetMinorTicks(ticks, NULL, &minor));
    SetCtrlVal(gPanel, PANEL_MINOR_TICKS, minor != VFALSE);
    hrChk (CW3DGraphLib_CWTicks3DGetMajorTicks(ticks, NULL, &major));
    SetCtrlVal(gPanel, PANEL_MAJOR_TICKS, major != VFALSE);
    hrChk (CW3DGraphLib_CWTicks3DGetMajorGrid(ticks, NULL, &major));
    SetCtrlVal(gPanel, PANEL_MAJOR_GRID, major != VFALSE);
    hrChk (CW3DGraphLib_CWTicks3DGetMinorGrid(ticks, NULL, &minor));
    SetCtrlVal(gPanel, PANEL_MINOR_GRID, minor != VFALSE);
Error:
    CA_FreeMemory(caption);
    CA_VariantClear(&vtMin);
    CA_VariantClear(&vtMax);
    CA_DiscardObjHandle(plot);
    CA_DiscardObjHandle(plots);
    CA_DiscardObjHandle(labels);
    CA_DiscardObjHandle(ticks);
*/    
	return hr;
}

int CVICALLBACK airplaneQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	switch (event){
		case EVENT_COMMIT:
			DiscardPanel(panelHdl2);
			panelHdl2 = 0;
			break;
	}
	return 0;
}

/* Temp stuff waiting to be deleted.
GetRelativeMouseState (TPanels[i], PGraphs[i], &tempX, &tempY,
					 NULL, NULL, NULL);
startPoint[0] = tempX * 9.0 / (tabWidth - 3);
startPoint[1] = tempY * 5.0 / (tabHeight - 15);
			
int createCanvas(int panelHdl){
	int penWidth = 1, penColor = VAL_YELLOW, penStyle = VAL_SOLID;
	int penFillColor = VAL_BLUE, penMode = VAL_COPY_MODE;
	int canvasCtrl = NewCtrl(panelHdl, CTRL_CANVAS, "", 100, 100);
	SetCtrlAttribute(panelHdl, canvasCtrl, ATTR_PEN_WIDTH, penWidth);
	SetCtrlAttribute(panelHdl, canvasCtrl, ATTR_PEN_COLOR, penColor);
	SetCtrlAttribute(panelHdl, canvasCtrl, ATTR_PEN_FILL_COLOR, penFillColor);
	SetCtrlAttribute(panelHdl, canvasCtrl, ATTR_PEN_STYLE, penStyle);
	SetCtrlAttribute(panelHdl, canvasCtrl, ATTR_PEN_MODE, penMode);
	return canvasCtrl;
}
*/

/* Airplane Callback area */
int CVICALLBACK airplaneStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2){
	HRESULT hr;
    double  t[41], xData[41][41], yData[41][41], zData[41][41];
    VARIANT xVt, yVt, zVt;
    int     i, j;
    switch (event){
        case EVENT_COMMIT:
			CA_VariantSetEmpty(&xVt);
            CA_VariantSetEmpty(&yVt);
            CA_VariantSetEmpty(&zVt);
            /* Generate data */
            for (i = 0; i < 41; ++i)
                t[i] = (i - 20.0) / 20.0 * PI;
            for (i = 0; i < 41; ++i)
                for(j = 0; j < 41; ++j){
                    xData[j][i] = (cos(t[j]) + 3.0) * cos(t[i]);
                    yData[j][i] = (cos(t[j]) + 3.0) * sin(t[i]);
                    zData[j][i] = sin(t[j]); }
            hrChk (CA_VariantSet2DArray(&xVt, CAVT_DOUBLE, 41, 41, xData));
            hrChk (CA_VariantSet2DArray(&yVt, CAVT_DOUBLE, 41, 41, yData));
            hrChk (CA_VariantSet2DArray(&zVt, CAVT_DOUBLE, 41, 41, zData));
            
            // Display data 
/*            hrChk(CW3DGraphLib__DCWGraph3DPlot3DParametricSurface(graph3DHdl,
                NULL, xVt, yVt, zVt, CA_DEFAULT_VAL));
*/
			hrChk(RefreshControls());
Error:
            if (FAILED(hr))
            CA_DisplayErrorInfo(0, "ActiveX Error", hr, NULL);
            CA_VariantClear(&xVt);
            CA_VariantClear(&yVt);
            CA_VariantClear(&zVt);
            break;
    }//switch()
    return 0;
}


