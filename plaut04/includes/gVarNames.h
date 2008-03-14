    const char *typeTokenNames[] =
    {
        "DEFAULT","BP ALG", "LP ALG",
        "HB", "UZ4", "UZ-4",
        "LP DIF", "BP DIF", "PD",
        "TR", "EP", "MX",
        "OTHERS"
    };

   const char *intVariableNames[] =
    {
        "Graph Type",
        "Graph Style",
        "Window Width",
        "Window Height",
//        "Labels",
        "Coloring Method",
        "Coloring Method Solution",
        "Coloring Method Bifurcation",
        "Line Width Scaler",
        "AniLine Thickness Scaler",
#ifndef R3B
        "Object Max Animation Speed",
        "Object Min Animation Speed",
#else
        "Sat Max Animation Speed",
        "Sat Min Animation Speed",
#endif
        "Orbit Max Animation Speed",
        "Orbit Min Animation Speed",
        "Coordinate Type",
        "Background Transparency",
        "Number of Period Animated",
        "Label Sphere Radius",
#ifndef R3B
        "Object Radius",
#else
        "Satellite Radius",
        "Large Primary Radius",
        "Small Primary Radius",
        "Libration Point Size",
	"Disk Transparency",
	"Disk From File",
        "Coordinate system",
        "Number of Stars",
#endif
    };

    const char * hexdecimalVarNames[] =
    {
        "UNSTABLE LINE PATTERN",
        "STABLE LINE PATTERN"
    };

   const char * nDataVarNames[] =
    {
        "Background Color",
        "X Axis Color",
        "Y Axis Color",
        "Z Axis Color",
#ifndef R3B
        "Object Color",
#else
        "satellite Color",
        "large primary Color",
        "large primary tail Color",
        "small primary Color",
        "small primary tail Color",
#endif
        "Surface Color",
        "Unstable Solution Color",
        "Stable Solution Color",
    };

    const char * blWidgetName[] =
    {
        "3D",
        "3DBif",
        "3DSol",
    };

const char * graphWidgetItems[]=
{
#ifndef R3B
    "Highlight Orbit",
#else
    "Draw Reference Plane", 
    "Draw Primaries", 
    "Draw Libration Points",
#endif
    "Orbit Animation", 
#ifdef R3B
    "Satellite Animation",
#endif
    "Draw Labels",
    "Show Label Numbers",
    "Draw Background", 
    "Draw Legend", 
    "Normalize Data",
};

const char * axesNames[] =
{
        "X Axis Solution",
        "Y Axis Solution",
        "Z Axis Solution",
        "X Axis Bifurcation",
        "Y Axis Bifurcation",
        "Z Axis Bifurcation",
};
