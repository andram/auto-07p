#define QT3_SUPPORT
#define LENGTH(arr) ((sizeof(arr) / sizeof(arr[0])))

#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>

#include <qapplication.h>
#include <qpainter.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qtoolbar.h>
#include <qbuttongroup.h>
#if QT_VERSION < 0x40000
#define addButton insert
#endif

#include "gplaut04.h"
#include "gmainqt.h"

#define LBL_OFFSET   4

extern SbBool printToPostScript (SoNode *root, FILE *file,
SoQtExaminerViewer *viewer, int printerDPI);
extern SoSeparator * createBoundingBox();

static MainWindow *mainWindow;

////////////////////////////////////////////////////////////////////////
//
//  functions
//
////////////////////////////////////////////////////////////////////////

DecSpinBox::DecSpinBox(int minValue, int maxValue, int step, QWidget * parent,
                       const char *name)
      : QSpinBox(minValue, maxValue, step, parent, name)
{
}

#if QT_VERSION >= 0x40000
QString
DecSpinBox::textFromValue( int value ) const {
    return QString("%1.%2").arg(value/10).arg(abs(value%10));
}

int
DecSpinBox::valueFromText( QString text ) const {
    return int(text.toFloat()*10);
}
#else
QString
DecSpinBox::mapValueToText( int value ) {
    return QString("%1.%2").arg(value/10).arg(abs(value%10));
}

int
DecSpinBox::mapTextToValue( bool* ok ) {
    return int(text().toFloat()*10);
}
#endif

////////////////////////////////////////////////////////////////////////
//
void
MainWindow::orbitSpeedCB(int value)
//
////////////////////////////////////////////////////////////////////////
{
    orbitSpeed = value/50.0;                ///50.0;     ///75.0;
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::satSpeedCB(int value)
//
////////////////////////////////////////////////////////////////////////
{
    satSpeed = value/100.0;
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::numPeriodAnimatedCB(const QString &myChoice)
//
////////////////////////////////////////////////////////////////////////
{
    if ( strcmp(myChoice, "inf") == 0 )
    {
       numPeriodAnimated = -1; 
    } 
    else 
    {
       numPeriodAnimated = atof(myChoice);
    }

//cout <<" Num Period Animated "<<myChoice<<"   "<<numPeriodAnimated;

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::colorMethodSelectionCB(const QString &myChoice)
//
////////////////////////////////////////////////////////////////////////
{
    int choice = colorMethodSeletionList->currentItem();

    coloringMethod = (strcasecmp(myChoice,"COMP")==0) ?  CL_COMPONENT:
    ((strcasecmp(myChoice,"TYPE")==0) ?  CL_ORBIT_TYPE :
    ((strcasecmp(myChoice,"BRAN")==0) ? CL_BRANCH_NUMBER:
    ((strcasecmp(myChoice,"PONT")==0) ? CL_POINT_NUMBER :
    ((strcasecmp(myChoice,"LABL")==0) ? CL_LABELS:
    ((strcasecmp(myChoice,"STAB")==0) ? CL_STABILITY : choice - specialColorItems)))));

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::lineWidthCB(int position)
//
////////////////////////////////////////////////////////////////////////
{
    lineWidthScaler = position/10.0;
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt when a menu item is picked from the File menu.
//
void
MainWindow::fileMenuPick(int which)
//
////////////////////////////////////////////////////////////////////////
{
    switch (which)
    {
        case SAVE_ITEM:
            getFileName(SAVE_ITEM);
            break;

        case QUIT_ITEM:
            postDeals();
            exit(0);
            break;
        case PRINT_ITEM:
            cropScene("myfile");
            break;
        case OPEN_ITEM:
            getFileName(OPEN_ITEM);
            break;
        default:
            printf("UNKNOWN file menu item!!!\n"); break;
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt when a menu item is picked from the Edit menu.
//
void
MainWindow::editMenuPick(int which)
//
////////////////////////////////////////////////////////////////////////
{
#ifdef R3B
    EditMenuItems *menuItems;
    menuItems = coordSystemMenuItems;
    menuItems->which = which;
    whichCoordSystem = which;
    whichCoordSystemOld = whichCoordSystem;

    updateScene();
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt when a menu item is picked from the TYPE menu.
//
void
MainWindow::typeMenuPick(int which)
//
////////////////////////////////////////////////////////////////////////
{
// get the user data for this menu item.
    EditMenuItems *menuItems;

    menuItems = typeMenuItems;
    menuItems->which = which;
    whichType    = which;

    if ( whichType != whichTypeOld )
    {
        graphWidgetToggleSet = 0;
        if( whichType != BIFURCATION )
        {
            setShow3D = setShow3DSol;
            for(int i=0; i<11; ++i)
            {
                optBif[i]  = options[i];
                options[i] = optSol[i];
                graphWidgetToggleSet |= options[i] << i;
            }
        }
        else
        {
            setShow3D = setShow3DBif;
            for(int i=0; i<11; ++i)
            {
                optSol[i]  = options[i];
                options[i] = optBif[i];
                graphWidgetToggleSet |= options[i] << i;
            }
        }
    }

    whichTypeOld = whichType;

    setListValue();

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//       This is called by Qt when a menu item is picked from the Option menu.
//
void
MainWindow::optMenuPick(int which)  
//
////////////////////////////////////////////////////////////////////////
{
    options[which] = !options[which];
    if (options[which])
        graphWidgetToggleSet |= (1 << which);
    else
        graphWidgetToggleSet &= ~(1 << which);

    satAniSpeedSlider->setEnabled(options[OPT_SAT_ANI]);
    orbitAniSpeedSlider->setEnabled(options[OPT_PERIOD_ANI]);

#ifdef R3B
    if(graphWidgetToggleSet & (1<<OPT_NORMALIZE_DATA))
    {
        options[OPT_PRIMARY] = false;
        options[OPT_LIB_POINTS] = false;
        options[OPT_REF_PLAN] = false;
        graphWidgetToggleSet &= ~(1 << OPT_REF_PLAN);
        graphWidgetToggleSet &= ~(1 << OPT_LIB_POINTS);
        graphWidgetToggleSet &= ~(1 << OPT_PRIMARY);
    }
#endif

    updateScene();
}

///////////////////////////////////////////////////////////////////////////
//
void
setListValue()
//
///////////////////////////////////////////////////////////////////////////
{
    mainWindow->setListValue();
}

///////////////////////////////////////////////////////////////////////////
//
void
MainWindow::setListValue()
//
///////////////////////////////////////////////////////////////////////////
{
    long nItems = (whichType != BIFURCATION) ? 
                           mySolNode.totalLabels+SP_LBL_ITEMS : 
                                           myBifNode.totalLabels+SP_LBL_ITEMS;
    if(whichType != BIFURCATION)
    {
        xCoordIdxSize = dai.solXSize;
        yCoordIdxSize = dai.solYSize;
        zCoordIdxSize = dai.solZSize;
        for(int i = 0; i<xCoordIdxSize; ++i)
            xCoordIndices[i] = dai.solX[i];
        for(int i = 0; i<yCoordIdxSize; ++i)
            yCoordIndices[i] = dai.solY[i];
        for(int i = 0; i<zCoordIdxSize; ++i)
            zCoordIndices[i] = dai.solZ[i];

        xAxisList->clear();
        yAxisList->clear();
        zAxisList->clear();
        for (int i = 0; i < mySolNode.nar; i++) {
            xAxisList->insertItem(xAxis[i]);
            yAxisList->insertItem(yAxis[i]);
            zAxisList->insertItem(zAxis[i]);
        }

        int sp = 0;
        strcpy(coloringMethodList[0],"STAB"); sp++;
        strcpy(coloringMethodList[1],"PONT"); sp++;
        strcpy(coloringMethodList[2],"BRAN"); sp++;
        strcpy(coloringMethodList[3],"TYPE"); sp++;
        strcpy(coloringMethodList[4],"LABL"); sp++;
        strcpy(coloringMethodList[5],"COMP"); sp++;
        specialColorItems = sp;
        for(int i=sp; i<mySolNode.nar+sp; ++i)
        {
            sprintf(coloringMethodList[i],"%d",i-sp);
        }
        for(int i=mySolNode.nar+sp; i<mySolNode.nar+mySolNode.npar+sp; ++i)
        {
            sprintf(coloringMethodList[i],"PAR(%d)",mySolNode.parID[i-(mySolNode.nar+sp)]+1);
        }

        colorMethodSeletionList->clear();
        for (int i = 0; i < mySolNode.nar+mySolNode.npar+sp; i++) 
            colorMethodSeletionList->insertItem(coloringMethodList[i]);
        labelsList->clear();
        for (int i = 0; i < nItems; i++) 
            labelsList->insertItem(labels[i]);
    }
    else
    {
        xCoordIdxSize = dai.bifXSize;
        yCoordIdxSize = dai.bifYSize;
        zCoordIdxSize = dai.bifZSize;
        for(int i = 0; i<xCoordIdxSize; ++i)
            xCoordIndices[i] = dai.bifX[i];
        for(int i = 0; i<yCoordIdxSize; ++i)
            yCoordIndices[i] = dai.bifY[i];
        for(int i = 0; i<zCoordIdxSize; ++i)
            zCoordIndices[i] = dai.bifZ[i];
        xAxisList->clear();
        yAxisList->clear();
        zAxisList->clear();
        for (int i = 0; i < myBifNode.nar; i++) {
            xAxisList->insertItem(xAxis[i]);
            yAxisList->insertItem(yAxis[i]);
            zAxisList->insertItem(zAxis[i]);
        }

        int sp = 0;
        strcpy(coloringMethodList[0],"STAB"); sp++;
        strcpy(coloringMethodList[1],"PONT"); sp++;
        strcpy(coloringMethodList[2],"BRAN"); sp++;
        specialColorItems = sp;
        for(int i=sp; i<myBifNode.nar+sp; ++i)
        {
            sprintf(coloringMethodList[i],"%d",i-sp);
        }
        colorMethodSeletionList->clear();
        for (int i = 0; i < myBifNode.nar+sp; i++) 
            colorMethodSeletionList->insertItem(coloringMethodList[i]);
        labelsList->clear();
        for (int i = 0; i < nItems; i++) 
            labelsList->insertItem(labels[i]);
    }
    xAxisList->setCurrentItem(xCoordIndices[0]);
    yAxisList->setCurrentItem(yCoordIndices[0]);
    zAxisList->setCurrentItem(zCoordIndices[0]);
    labelsList->setCurrentItem(lblChoice[0]+LBL_OFFSET-1); //lblIndices[0]
    colorMethodSeletionList->setCurrentItem(coloringMethod < 0 ?
       coloringMethod+CL_SP_ITEMS : coloringMethod+specialColorItems);
    dimButton->setText(setShow3D ? "3D" : "2D");

    if(setShow3D)
        zAxisList->setEnabled(true);
    else
    {
        for(int i=0; i<MAX_LIST; i++)
            zCoordIndices[i] = -1;
        zCoordIdxSize = 1;
        zAxisList->setEnabled(false);
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt when a menu item is picked from the STYLE menu.
//
void
MainWindow::styleMenuPick(int which)
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems;

    menuItems = styleMenuItems;
    menuItems->which = which;
    whichStyle = which;
    whichStyleOld = whichStyle;

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt when a menu item is picked from the STYLE menu.
//
void
MainWindow::coordMenuPick(int which)
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems;

    menuItems = coordMenuItems;
    menuItems->which = which;

    if(which == DRAW_TICKER)
        blDrawTicker = !blDrawTicker ;
    else
    {
        whichCoord = which;
        whichCoordOld = whichCoord;
    }

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt just before the TYPE menu is displayed.
//
void MainWindow::typeMenuDisplay()
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems = typeMenuItems;

    menuItems->items->setItemChecked(BIFURCATION, false);
    menuItems->items->setItemChecked(SOLUTION, false);

    menuItems->items->setItemChecked(whichType, true);

    menuItems->items->setItemEnabled(BIFURCATION, true);
    menuItems->items->setItemEnabled(SOLUTION, true);

    if(!blOpenSolFile)
        menuItems->items->setItemEnabled(SOLUTION, false);

    if(!blOpenBifFile)
        menuItems->items->setItemEnabled(BIFURCATION, false);
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt just before the STYLE menu is displayed.
//
void 
MainWindow::styleMenuDisplay()
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems = styleMenuItems;
    menuItems->items->setItemChecked(LINE, false);
    menuItems->items->setItemChecked(TUBE, false);
    menuItems->items->setItemChecked(SURFACE, false);
    menuItems->items->setItemChecked(MESH_POINTS, false);
    menuItems->items->setItemChecked(ALL_POINTS, false);

#ifndef R3B
    if( whichType == BIFURCATION)
#else
    if(whichCoordSystem != ROTATING_F || whichType == BIFURCATION)
#endif
    {
        menuItems->items->setItemEnabled(SURFACE, false);
        menuItems->items->setItemEnabled(MESH_POINTS, false);
        menuItems->items->setItemEnabled(ALL_POINTS, false);
#ifdef R3B
        if(menuItems->which == SURFACE || menuItems->which == MESH_POINTS || menuItems->which == ALL_POINTS)
            menuItems->which = LINE;
#endif
    }
    else
    {
        menuItems->items->setItemEnabled(SURFACE, true);
        menuItems->items->setItemEnabled(MESH_POINTS, true);
        menuItems->items->setItemEnabled(ALL_POINTS, true);
    }

    menuItems->items->setItemChecked(menuItems->which, true);
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt just before the STYLE menu is displayed.
//
void 
MainWindow::coordMenuDisplay()
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems = coordMenuItems;

    if(menuItems->which == DRAW_TICKER)
    {
        menuItems->items->setItemChecked(DRAW_TICKER, blDrawTicker);
    }
    else
    {
        menuItems->items->setItemChecked(NO_COORD, false);
        menuItems->items->setItemChecked(COORDORIGIN, false);
        menuItems->items->setItemChecked(LEFTBACK, false);
        menuItems->items->setItemChecked(LEFTAHEAD, false);
        menuItems->items->setItemChecked(DRAW_TICKER, blDrawTicker);
        menuItems->items->setItemChecked(menuItems->which, true);
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt just before the Edit menu is displayed.
//
void
MainWindow::centerMenuDisplay()
//
////////////////////////////////////////////////////////////////////////
{
#ifdef R3B
    EditMenuItems *menuItems = coordSystemMenuItems;

    menuItems->items->setItemChecked(ROTATING_F, false);
    menuItems->items->setItemChecked(INERTIAL_B, false);
    menuItems->items->setItemChecked(INERTIAL_S, false);
    menuItems->items->setItemChecked(INERTIAL_E, false);
    menuItems->items->setItemChecked(menuItems->which, true);

    bool enable = whichType == SOLUTION;
    menuItems->items->setItemEnabled(ROTATING_F, enable);
    menuItems->items->setItemEnabled(INERTIAL_B, enable);
    menuItems->items->setItemEnabled(INERTIAL_E, enable);
    menuItems->items->setItemEnabled(INERTIAL_S, enable);
#endif
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Qt just before the TYPE menu is displayed.
//
void 
MainWindow::optMenuDisplay()
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems = optMenuItems;

    for (int i = 0; i < LENGTH (graphWidgetItems); i++)
    {
        menuItems->items->setItemChecked(i, (graphWidgetToggleSet & (1<<i)) != 0);
    }

#ifdef R3B
    menuItems->items->setItemEnabled(OPT_PRIMARY, blMassDependantOption);
    menuItems->items->setItemEnabled(OPT_LIB_POINTS, blMassDependantOption);

    if(graphWidgetToggleSet & (1<<OPT_NORMALIZE_DATA))
    {
        menuItems->items->setItemEnabled(OPT_PRIMARY, false);
        menuItems->items->setItemEnabled(OPT_LIB_POINTS, false);
        menuItems->items->setItemEnabled(OPT_REF_PLAN, false);
        menuItems->items->setItemChecked(OPT_PRIMARY, false);
        menuItems->items->setItemChecked(OPT_LIB_POINTS, false);
        menuItems->items->setItemChecked(OPT_REF_PLAN, false);
    }
    else
    {
        menuItems->items->setItemEnabled(OPT_PRIMARY, true);
        menuItems->items->setItemEnabled(OPT_LIB_POINTS, true);
        menuItems->items->setItemEnabled(OPT_REF_PLAN, true);
    }
#endif

    if(whichType == SOLUTION)
    {
        menuItems->items->setItemEnabled(OPT_SAT_ANI, true);
#ifndef R3B
        menuItems->items->changeItem(OPT_PERIOD_ANI, "&Highlight Orbit");
#else
        menuItems->items->changeItem(OPT_PERIOD_ANI, "&Orbit Animation");
#endif
    }
    else
    {
        menuItems->items->setItemEnabled(OPT_SAT_ANI, false);
        menuItems->items->changeItem(OPT_PERIOD_ANI, "&Draw Labels");
        menuItems->items->setItemEnabled(OPT_LABEL_NUMBERS,
            (graphWidgetToggleSet & (1<<OPT_PERIOD_ANI)) != 0);
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the File menu and all its items.
//
QPopupMenu *
MainWindow::buildFileMenu()
//
////////////////////////////////////////////////////////////////////////
{
    QPopupMenu *pulldown;

    pulldown = new QPopupMenu(this);
    pulldown->insertItem("&Open...", this, SLOT(fileMenuPick(int)),
                         Qt::CTRL+Qt::Key_O, OPEN_ITEM);
    pulldown->insertItem("&Export...", this, SLOT(fileMenuPick(int)),
                         Qt::CTRL+Qt::Key_S, SAVE_ITEM);
#ifdef R3B
    pulldown->insertItem("&Print...", this, SLOT(fileMenuPick(int)),
                         Qt::CTRL+Qt::Key_P, PRINT_ITEM);
#endif
    pulldown->insertSeparator();
    pulldown->insertItem("&Quit", this, SLOT(fileMenuPick(int)),
                         Qt::CTRL+Qt::Key_Q, QUIT_ITEM);
    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the Help menu and all its items.
//
QPopupMenu *
MainWindow::buildHelpMenu()
//
////////////////////////////////////////////////////////////////////////
{
    QPopupMenu *pulldown = new QPopupMenu(this);
    pulldown->insertItem("&About", this, SLOT(showAboutDialog()));
    pulldown->insertSeparator();
    pulldown->insertItem("&HELP", this, SLOT(showHelp()));
    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the Option menu and all its items.
//
QPopupMenu *
MainWindow::buildOptionMenu()
//
////////////////////////////////////////////////////////////////////////
{
    QPopupMenu *pulldown;
    pulldown = new QPopupMenu(this);
    pulldown->setCheckable(true);

    connect(pulldown, SIGNAL(aboutToShow()), this, SLOT(optMenuDisplay()));

    EditMenuItems *menuItems = new EditMenuItems;
    menuItems->items = pulldown;
    optMenuItems = menuItems;
#ifndef R3B
    pulldown->insertItem("&Highlight Orbit", this, SLOT(optMenuPick(int)),
                         0, OPT_PERIOD_ANI);
    pulldown->insertItem("Sho&w Label Numbers", this, SLOT(optMenuPick(int)),
                         0, OPT_LABEL_NUMBERS);
    pulldown->insertItem("&Orbit Animation", this, SLOT(optMenuPick(int)),
                         0, OPT_SAT_ANI);
#else
    pulldown->insertItem("Draw &Reference Plane", this, SLOT(optMenuPick(int)),
                         0, OPT_REF_PLAN);
    pulldown->insertItem("Draw &Primaries", this, SLOT(optMenuPick(int)),
                         0, OPT_PRIMARY);
    pulldown->insertItem("Draw &Libraration Pts", this, SLOT(optMenuPick(int)),
                         0, OPT_LIB_POINTS);
    pulldown->insertSeparator();
    pulldown->insertItem("&Orbit Animation", this, SLOT(optMenuPick(int)),
                         0, OPT_PERIOD_ANI);
    pulldown->insertItem("&Satellite Animation", this, SLOT(optMenuPick(int)),
                         0, OPT_SAT_ANI);
    pulldown->insertItem("Sho&w Label Numbers", this, SLOT(optMenuPick(int)),
                         0, OPT_LABEL_NUMBERS);
#endif
    pulldown->insertItem("Draw &Background", this, SLOT(optMenuPick(int)),
                         0, OPT_BACKGROUND);
    pulldown->insertItem("&Add Legend", this, SLOT(optMenuPick(int)),
                         0, OPT_LEGEND);
    pulldown->insertItem("&Normalize Data", this, SLOT(optMenuPick(int)),
                         0, OPT_NORMALIZE_DATA);
    pulldown->insertSeparator();
    pulldown->insertItem("&PREFERENCES", this, SLOT(createPreferDialog()));

    return pulldown;
}


#ifdef R3B
////////////////////////////////////////////////////////////////////////
//
//  This creates the Edit menu and all its items.
//
QPopupMenu *
MainWindow::buildCenterMenu()
//
////////////////////////////////////////////////////////////////////////
{
    QPopupMenu *pulldown;
    pulldown = new QPopupMenu(this);
    pulldown->setCheckable(true);

    connect(pulldown, SIGNAL(aboutToShow()), this, SLOT(centerMenuDisplay()));

    coordSystemMenuItems = new EditMenuItems;
    coordSystemMenuItems->items = pulldown;
    coordSystemMenuItems->which = whichCoordSystem;

    pulldown->insertItem("&Rotating Frame", this,
                         SLOT(editMenuPick(int)), 0, ROTATING_F);
    pulldown->insertSeparator();
    pulldown->insertItem("Bary &Centered", this,
                         SLOT(editMenuPick(int)), 0, INERTIAL_B);
    pulldown->insertItem("&Big Primary Centered", this,
                         SLOT(editMenuPick(int)), 0, INERTIAL_S);
    pulldown->insertItem("&Small Primary Centered", this,
                         SLOT(editMenuPick(int)), 0, INERTIAL_E);
    return pulldown;
}
#endif


////////////////////////////////////////////////////////////////////////
//
//  This creates the STYLE menu and all its items.
//
QPopupMenu *
MainWindow::buildStyleMenu()
//
////////////////////////////////////////////////////////////////////////
{
    QPopupMenu *pulldown;
    pulldown = new QPopupMenu(this);

    styleMenuItems = new EditMenuItems;
    styleMenuItems->items = pulldown;   
    styleMenuItems->which = whichStyle;
    connect(pulldown, SIGNAL(aboutToShow()), this, SLOT(styleMenuDisplay()));

    pulldown->setCheckable(true);
    pulldown->insertItem("&Line", this, SLOT(styleMenuPick(int)), 0, LINE);
    pulldown->insertItem("&Tube", this, SLOT(styleMenuPick(int)), 0, TUBE);
    pulldown->insertItem("&Surface", this, SLOT(styleMenuPick(int)),
                         0, SURFACE);
    pulldown->insertItem("&Mesh Points", this, SLOT(styleMenuPick(int)),
                         0, MESH_POINTS);
    pulldown->insertItem("&All Points", this, SLOT(styleMenuPick(int)),
                         0, ALL_POINTS);
    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the Coordinates menu and all its items.
//
QPopupMenu *
MainWindow::buildCoordMenu()
//
////////////////////////////////////////////////////////////////////////
{
    QPopupMenu *pulldown;

    pulldown = new QPopupMenu(this);
    coordMenuItems = new EditMenuItems;
    coordMenuItems->items = pulldown;
    coordMenuItems->which = whichCoord;

    connect(pulldown, SIGNAL(aboutToShow()), this, SLOT(coordMenuDisplay()));

    pulldown->setCheckable(true);
    pulldown->insertItem("&NONE", this, SLOT(coordMenuPick(int)), 0, NO_COORD);
    pulldown->insertItem("&Coord Center", this, SLOT(coordMenuPick(int)),
                         0, COORDORIGIN);
    pulldown->insertItem("Left and &Back", this, SLOT(coordMenuPick(int)),
                         0, LEFTBACK);
    pulldown->insertItem("Left and &Ahead", this, SLOT(coordMenuPick(int)),
                         0, LEFTAHEAD);
    pulldown->insertSeparator();
    pulldown->insertItem("&Draw Scale", this, SLOT(coordMenuPick(int)),
                         0, DRAW_TICKER);
    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the TYPE menu and all its items.
//
QPopupMenu *
MainWindow::buildTypeMenu()
//
////////////////////////////////////////////////////////////////////////
{
    QPopupMenu *pulldown;

    pulldown = new QPopupMenu(this);
    typeMenuItems = new EditMenuItems;
    typeMenuItems->items = pulldown;
    typeMenuItems->which = whichType;

    pulldown->setCheckable(true);

    connect(pulldown, SIGNAL(aboutToShow()), this, SLOT(typeMenuDisplay()));

    pulldown->insertItem("&Solution", this, SLOT(typeMenuPick(int)),
                         0, SOLUTION);
    pulldown->insertItem("&Bifurcation", this, SLOT(typeMenuPick(int)),
                         0, BIFURCATION);

    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the pulldown menu bar and its menus.
//
void
MainWindow::buildMenu()
//
////////////////////////////////////////////////////////////////////////
{
#ifdef R3B
    QPopupMenu *pulldown2;
#endif
    QPopupMenu *pulldown1, *pulldown3, *pulldown4, *pulldown5, *pulldown6, *pulldown7;
// menu bar
    pulldown1 = buildFileMenu();
#ifdef R3B
    pulldown2 = buildCenterMenu();
#endif
    pulldown3 = buildStyleMenu();
    pulldown4 = buildTypeMenu();
    pulldown7 = buildCoordMenu();
    pulldown5 = buildOptionMenu();
    pulldown6 = buildHelpMenu();

#ifdef USE_BK_COLOR
// set the background color for the pull down menus.
    pulldown1->setPaletteBackgroundColor("white");
#ifdef R3B
    pulldown2->setPaletteBackgroundColor("white");
#endif
    pulldown3->setPaletteBackgroundColor("white");
    pulldown4->setPaletteBackgroundColor("white");
    pulldown5->setPaletteBackgroundColor("white");
    pulldown6->setPaletteBackgroundColor("white");
#endif
// the text in the menubar for these menus
    QMenuBar *menubar = menuBar();
    menubar->insertItem("&File", pulldown1);
    menubar->insertItem("&Type", pulldown4);
    menubar->insertItem("&Style", pulldown3);
    menubar->insertItem("&Draw Coord", pulldown7);
#ifdef R3B
    menubar->insertItem("&Center", pulldown2);
#endif
    menubar->insertItem("&Options", pulldown5);
    menubar->insertItem("&Help", pulldown6);
}


////////////////////////////////////////////////////////////////////////
//
void 
MainWindow::dimensionToggledCB()
//
////////////////////////////////////////////////////////////////////////
{
    static bool buttonState = setShow3D;
    buttonState = !buttonState;
    if(buttonState)
    {
        setShow3D = true;
        zAxisList->setEnabled(true);
        setListValue();
    }
    else
    {
        setShow3D = false;
        for(int i=0; i<MAX_LIST; i++)
            zCoordIndices[i] = -1;
        zCoordIdxSize = 1;
        zAxisList->setEnabled(false);
        dimButton->setText("2D");
    }
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void 
MainWindow::createBdBoxCB()
//
////////////////////////////////////////////////////////////////////////
{
    static bool btnState = false;
    SoSeparator * scene = sceneGraph;
    btnState = !btnState;
    if(btnState)
        scene->addChild(createBoundingBox());
    else
    {
        SoSeparator * bdBox = (SoSeparator *)SoNode::getByName("bdBox");
        scene->removeChild(bdBox);
    }
}

////////////////////////////////////////////////////////////////////////
//
//  This creates the main window contents. In this case, we have a
//  menubar at the top of the window, and a render area filling out
//  the remainder. These widgets are layed out with a QMainWindow widget.
//
MainWindow::MainWindow() : QMainWindow()
//
////////////////////////////////////////////////////////////////////////
{
    struct ViewerAndScene
    {
        SoQtExaminerViewer *viewer;
        char               *filename;
        SoNode             *scene;
    } ;

    int  i;

    sceneGraph = new SoSeparator;
    sceneGraph->ref();

    SoEventCallback *mouseEventCB = new SoEventCallback;
    sceneGraph->addChild(mouseEventCB);
    sceneGraph->addChild(root);

#ifdef R3B
    updateScene();
#endif

    // build menubar
    buildMenu();

    // build carrier for the x, y, z, and label lists.
#if QT_VERSION >= 0x40000
    QToolBar *listCarrier = addToolBar("ToolBar");
    #define ADD_LISTCARRIER_WIDGET(x) listCarrier->addWidget(x)
#else
    QToolBar *listCarrier = new QToolBar( this );
    #define ADD_LISTCARRIER_WIDGET(x)
#endif

#ifdef USE_BK_COLOR
    setPaletteBackgroundColor("white");
    menuBar()->setPaletteBackgroundColor("white");
    listCarrier->setPaletteBackgroundColor("white");
#endif

// build the xAxis drop down list
    QLabel *xLbl = new QLabel( "  X", listCarrier );
    ADD_LISTCARRIER_WIDGET(xLbl);
    // Create an editable Combobox
    xAxisList = new QComboBox(true, listCarrier, "xAxis");
    ADD_LISTCARRIER_WIDGET(xAxisList);
    int nItems = (whichType != BIFURCATION) ? mySolNode.nar : myBifNode.nar;
    for ( i = 0; i < nItems; i++ )
        xAxisList->insertItem( xAxis[i] );
    xAxisList->setCurrentItem(xCoordIndices[0]);
    // Connect the activated SIGNALs of the Comboboxes with SLOTs
    connect(xAxisList, SIGNAL(activated(const QString &)),
            this, SLOT(xListCallBack(const QString &)));

#ifdef USE_BK_COLOR
    xAxisList->setPaletteBackgroundColor("white");
#endif

// build the yAxis drop down list
    // Create a editable Combobox
    QLabel *yLbl = new QLabel("  Y", listCarrier);
    ADD_LISTCARRIER_WIDGET(yLbl);
    yAxisList = new QComboBox(true, listCarrier, "yAxis");
    ADD_LISTCARRIER_WIDGET(yAxisList);
    for ( i = 0; i < nItems; i++ )
        yAxisList->insertItem( yAxis[i] );
    yAxisList->setCurrentItem(yCoordIndices[0]);
    // Connect the activated SIGNALs of the Comboboxes with SLOTs
    connect(yAxisList, SIGNAL(activated(const QString &)),
            this, SLOT(yListCallBack(const QString & )));

#ifdef USE_BK_COLOR
    yAxisList->setPaletteBackgroundColor("white");
#endif

    //build the zAxis drop down list
    // Create a editable Combobox
    QLabel *zLbl = new QLabel("  Z", listCarrier);
    ADD_LISTCARRIER_WIDGET(zLbl);
    zAxisList = new QComboBox(true, listCarrier, "zAxis");
    ADD_LISTCARRIER_WIDGET(zAxisList);
    for ( i = 0; i < nItems; i++ )
        zAxisList->insertItem( zAxis[i] );
    zAxisList->setCurrentItem(zCoordIndices[0]);
    // Connect the activated SIGNALs of the Comboboxes with SLOTs
    connect(zAxisList, SIGNAL(activated(const QString &)),
            this, SLOT(zListCallBack(const QString &)));

#ifdef USE_BK_COLOR
    zAxisList->setPaletteBackgroundColor("white");
#endif

// build the LABELs drop down list
    QLabel *lLbl = new QLabel("  Label", listCarrier);
    ADD_LISTCARRIER_WIDGET(lLbl);
    nItems = (whichType != BIFURCATION) ? 
                       mySolNode.totalLabels+SP_LBL_ITEMS : 
                       myBifNode.totalLabels+SP_LBL_ITEMS;
    labelsList = new QComboBox(true, listCarrier, "Labels");
    ADD_LISTCARRIER_WIDGET(labelsList);
    for ( i = 0; i < nItems; i++ )
        labelsList->insertItem( labels[i] );
    labelsList->setCurrentItem(lblChoice[0]+LBL_OFFSET-1); //lblIndices[0]

// Add Callback function for the LABELs drop down list
    connect(labelsList, SIGNAL(activated(const QString &)),
            this, SLOT(lblListCallBack(const QString &)));

#ifdef USE_BK_COLOR
    labelsList->setPaletteBackgroundColor("white");
    labelsList->setPaletteForegroundColor("red");
#endif

// build the COLORING Method drop down list
    QLabel *colorLbl = new QLabel("  Color", listCarrier);
    ADD_LISTCARRIER_WIDGET(colorLbl);
    nItems = (whichType != BIFURCATION) ? 
                      mySolNode.nar+mySolNode.npar+specialColorItems : 
                                  myBifNode.nar+specialColorItems;
    colorMethodSeletionList = new QComboBox(true, listCarrier,
                                            "coloringMethodlist");
    ADD_LISTCARRIER_WIDGET(colorMethodSeletionList);
    for ( i = 0; i < nItems; i++ )
        colorMethodSeletionList->insertItem(coloringMethodList[i]);
    colorMethodSeletionList->setCurrentItem(coloringMethod+specialColorItems);

// Add Callback function for the coloring method seletion drop down list
    connect(colorMethodSeletionList, SIGNAL(activated(const QString &)),
            this, SLOT(colorMethodSelectionCB(const QString &)));

//-----------------------------------------------------Nov 06

// build the numPeriodAnimated drop down list
    QLabel *numPeriodLbl = new QLabel("  Period", listCarrier);
    ADD_LISTCARRIER_WIDGET(numPeriodLbl);
    nItems = 7;
    int count = nItems;
    char (*numberPList)[5] = new char [count][5];

    int iam = 1;
    sprintf(numberPList[0], "%i", 0);
    for (i = 0; i < count-2; ++i)
    {
        sprintf(numberPList[i+1], "%i", iam);
        iam *= 2;
    }
    sprintf(numberPList[count-1], "%s", "inf");

    if (numPeriodAnimated > 0)
        i = ((int)(log(numPeriodAnimated)/log(2))) + 1;
    else if (numPeriodAnimated == 0)
        i = 0;
    else
        i = count - 1;

    QComboBox *numPeriodAnimatedList = new QComboBox(true, listCarrier, "list");
    ADD_LISTCARRIER_WIDGET(numPeriodAnimatedList);
    for ( int j = 0; j < nItems; j++ )
        numPeriodAnimatedList->insertItem(numberPList[j]);
    numPeriodAnimatedList->setCurrentItem(i);

// Add Callback function for the numberPeriodAnimated drop down list
    connect(numPeriodAnimatedList, SIGNAL(activated(const QString &)),
            this, SLOT(numPeriodAnimatedCB(const QString &)));

//    delete []numberPList;
//----------------------------------------------------------------> Nov 06 End

#ifdef USE_BK_COLOR
//set the background color for the labels
    xLbl->setPaletteBackgroundColor("white");
    yLbl->setPaletteBackgroundColor("white");
    zLbl->setPaletteBackgroundColor("white");
    lLbl->setPaletteBackgroundColor("white");
#endif

// create spinbox for the line width control.
    QLabel *spLbl = new QLabel("  Line Thickness", listCarrier);
    ADD_LISTCARRIER_WIDGET(spLbl);
    DecSpinBox *spinBox = new DecSpinBox(10, 100, 1, listCarrier, "spinBox");
    ADD_LISTCARRIER_WIDGET(spinBox);
    spinBox->setValue(10);

// Callbacks for the spinebox
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(lineWidthCB(int)));

// Create slider to control speed
#ifndef R3B
    QLabel *satSldLbl = new QLabel("  Anim", listCarrier);
#else
    QLabel *satSldLbl = new QLabel("  Sat ", listCarrier);
#endif
    ADD_LISTCARRIER_WIDGET(satSldLbl);
    satAniSpeedSlider = new QSlider(MIN_SAT_SPEED, MAX_SAT_SPEED, 1,
        (int)(satSpeed*100), Qt::Horizontal, listCarrier, "Speed");
    ADD_LISTCARRIER_WIDGET(satAniSpeedSlider);
    satAniSpeedSlider->setEnabled(options[OPT_SAT_ANI]);

#ifdef USE_BK_COLOR
    satAniSpeedSlider->setPaletteBackgroundColor("white");
#endif

// Callbacks for the slider
    connect(satAniSpeedSlider, SIGNAL(valueChanged(int)),
            this, SLOT(satSpeedCB(int)));

    QLabel *orbitSldLbl = new QLabel("  Orbit", listCarrier);
    ADD_LISTCARRIER_WIDGET(orbitSldLbl);
    orbitAniSpeedSlider = new QSlider(MIN_ORBIT_SPEED, MAX_ORBIT_SPEED, 1,
        (int)(orbitSpeed*50), Qt::Horizontal, listCarrier, "Speed2");
    ADD_LISTCARRIER_WIDGET(orbitAniSpeedSlider);
    orbitAniSpeedSlider->setEnabled(options[OPT_PERIOD_ANI]);

#ifdef USE_BK_COLOR
    orbitAniSpeedSlider->setPaletteBackgroundColor("white");
#endif

// Callbacks for the slider2
    connect(orbitAniSpeedSlider, SIGNAL(valueChanged(int)),
            this, SLOT(orbitSpeedCB(int)));

    QWidget *widget = new QWidget(this);
// create RENDER AREA FOR THE graphics.
#ifdef USE_EXAM_VIEWER
    SoQtExaminerViewer *renderArea = new SoQtExaminerViewer(widget);
#else
    SoQtRenderArea *renderArea = new SoQtRenderArea(widget);
#endif

    renderArea->setSize(SbVec2s(winWidth, winHeight));
    renderArea->setBackgroundColor(envColors[0]);
#ifdef R3B
    renderArea->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
#endif

#ifdef USE_EXAM_VIEWER
    QFont f("Helvetica", 8);
    QPushButton *newButton = new QPushButton("BOX", renderArea->getAppPushButtonParent());
    newButton->setFont(f);
    newButton->setFixedSize(27,27);

    connect(newButton, SIGNAL(clicked()), this, SLOT(createBdBoxCB()));
    renderArea->addAppPushButton(newButton);
#endif

    QString xString;
    zAxisList->setEnabled(setShow3D);
    xString = setShow3D ? "3D" : "2D";

    dimButton = new QPushButton(xString, renderArea->getAppPushButtonParent());
    dimButton->setFont(f);
    dimButton->setFixedSize(27,27);
    connect(dimButton, SIGNAL(clicked()), this, SLOT(dimensionToggledCB()));
    renderArea->addAppPushButton(dimButton);
    setCentralWidget(widget);

// used for printing  scene to ps files
    ViewerAndScene *vwrAndScene = new ViewerAndScene;
    vwrAndScene->scene  = renderArea->getSceneGraph();
    vwrAndScene->viewer = renderArea;

#ifndef R3B
    updateScene();
#endif
    renderArea->setSceneGraph(sceneGraph);

    resize(winWidth,winHeight);

    mouseEventCB->addEventCallback(
        SoMouseButtonEvent::getClassTypeId(),
        myMousePressCB,
	renderArea->getSceneManager()->getSceneGraph());

    // Set termination condition.
    connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
}


////////////////////////////////////////////////////////////////////////
//
LineColorSpinBox::LineColorSpinBox(int minValue, int maxValue, int step,
                             QWidget * parent, const char *name, int id)
      : DecSpinBox(minValue, maxValue, step, parent, name)
//
////////////////////////////////////////////////////////////////////////
{
    setWrapping(true);
    which = id;
    setValue((int)(lineColor[id/3][id%3]*10));
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(valueChangedCB(int)));
}

////////////////////////////////////////////////////////////////////////
//
//     When the line color changed, this function will be raised.
//
void
LineColorSpinBox::valueChangedCB(int value)
//
////////////////////////////////////////////////////////////////////////
{
    int lineNumber = which / 3;
    int columnNumber = which % 3;
    lineColorTemp[lineNumber][columnNumber] = value/10.0;
}


////////////////////////////////////////////////////////////////////////
//
LinePatternComboBox::LinePatternComboBox(bool rw, QWidget * parent,
                                         const char *name, int id)
      : QComboBox(rw, parent, name)
//
////////////////////////////////////////////////////////////////////////
{
    static const char *systemLinePatternLookAndFeel[] =
    {
        "SOLID LINE",   "--------",   ". . . . . ",    "_ . _ . _ .",
        "_ . . _ . .",  "_ . . . _",  "___ _ ___ _", "____ __ ____",
        "NULL "
    };

    which = id;
    int lengthOfSysPatternArray = LENGTH( systemLinePatternLookAndFeel );
    for (int i = 0; i < lengthOfSysPatternArray; i++)
        insertItem(systemLinePatternLookAndFeel[i]);
    connect(this, SIGNAL(activated(int)), this, SLOT(valueChangedCB(int)));
}

////////////////////////////////////////////////////////////////////////
//
//     When the line pattern selection changed, this function will be raised.
//
void
LinePatternComboBox::valueChangedCB(int position)
//
////////////////////////////////////////////////////////////////////////
{
    static const unsigned long systemLinePatternValue[] =
    {
        0xffff, 0x7777,  0x3333,  0xfafa, 0xeaea, 0xffcc, 0xffdc,0xff9c,0
    };

    linePatternTemp[which] = systemLinePatternValue[position];
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the COLOR and LINE preference sheet stuff.
//
void
MainWindow::createLineColorAndPatternPrefSheetGuts(QWidget *form, QGridLayout *layout, const char *name, int id)
//
////////////////////////////////////////////////////////////////////////
{
    QLabel *label = new QLabel(name, form);
    int column = (id%2) * 9;
    int row = 1 + id/2;

    layout->addWidget(label, row, column+1);

// Create the red field
    LineColorSpinBox *spinred =
      new LineColorSpinBox(0, 10, 1, form, "redText", id*3);
    layout->addWidget(spinred, row, column+3);

// Create the green field
    LineColorSpinBox *spingreen =
      new LineColorSpinBox(0, 10, 1, form, "greenText", id*3+1);
    layout->addWidget(spingreen, row, column+4);

// Create the blue field
    LineColorSpinBox *spinblue =
      new LineColorSpinBox(0, 10, 1, form, "blueText", id*3+2);
    layout->addWidget(spinblue, row, column+5);

// create the line pattern
    LinePatternComboBox *lpComboBox =
      new LinePatternComboBox(false, form, "lpSimple", id);
    layout->addWidget(lpComboBox, row, column+7);
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createColorAndLinePrefSheetHeader(QWidget *parent, 
        QGridLayout *layout, int column)
//
////////////////////////////////////////////////////////////////////////
{
    // create the first line
    //"|  PT TYPE  | RED  GREEN BLUE  | LINE PATTERN  |"
    const char *names[] = { "|", "PT TYPE", "|", "RED", "GREEN", "BLUE", "|",
                            "LINE PATTERN", "|" };
    for (int i = 0; i < 9; i++) {
        QLabel *label = new QLabel(names[i], parent);
        label->setAlignment(Qt::AlignHCenter);
        layout->addWidget(label, 0, column + i);
    }
}


///////////////////////////////////////////////////////////////////////
//
//  This simply creates the default parts of the pref dialog.
//
void
MainWindow::createLineAttrPrefSheetParts(QWidget *parent, QGridLayout *form,
   const char** name)
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePatternTemp[i] = linePattern[i];
        linePatternOld[i]  = linePattern[i];
        for(int j=0; j<3; ++j)
        {
            lineColorTemp[i][j] = lineColor[i][j];
            lineColorOld[i][j]  = lineColor[i][j];
        }
    }

    for(int i=0; i<NUM_SP_POINTS; ++i)
        createLineColorAndPatternPrefSheetGuts(parent, form, name[i], i);
    for(int i=1; i<(NUM_SP_POINTS+1)/2; ++i)
        form->setRowStretch(i, 1);
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createPreferActionFormControls(QWidget *parent)
//
////////////////////////////////////////////////////////////////////////
{
    QPushButton *saveBtn, *closeBtn, *applyBtn, *cancelBtn;
    QHBoxLayout *form = new QHBoxLayout(parent, 5, -1, "control form");

    saveBtn = new QPushButton(" &Save ", parent);
    connect(saveBtn, SIGNAL(clicked()),
                     this, SLOT(savePreferAndUpdateScene()));
    form->addWidget(saveBtn);

    closeBtn = new QPushButton(" &Update ", parent);
    form->addWidget(closeBtn);
    connect(closeBtn, SIGNAL(clicked()),
                     this, SLOT(closePreferDialogAndUpdateScene()));

    applyBtn = new QPushButton(" &Apply ", parent);
    form->addWidget(applyBtn);
    connect(applyBtn, SIGNAL(clicked()),
                     this, SLOT(applyPreferDialogChangeAndUpdateScene()));

    cancelBtn = new QPushButton(" &Cancel ", parent);
    form->addWidget(cancelBtn);
    connect(cancelBtn, SIGNAL(clicked()),
                     this, SLOT(closePreferDialogAndGiveUpChange()));
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::graphCoordinateSystemToggledCB(int which)
//
////////////////////////////////////////////////////////////////////////
{
#ifdef R3B
    whichCoordSystemTemp = which;
#endif
}


#ifdef R3B
////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createGraphCoordinateSystemFrameGuts(QGroupBox *frame)
//
////////////////////////////////////////////////////////////////////////
{
    const char *coordSysItems[]=
    {
        "Rotating Frame", "Barycenter " ,
        "Large Primary Center", "Small Primary Center"
    };

// create default selections
    whichCoordSystemOld  = whichCoordSystem;
    whichCoordSystemTemp = whichCoordSystem;

#if QT_VERSION >= 0x40000
    QHBoxLayout *layout = new QHBoxLayout(frame);
#endif
    QButtonGroup *group = new QButtonGroup(frame);
    connect(group, 
#if QT_VERSION >= 0x40000
        SIGNAL(buttonClicked(int)),
#else
        SIGNAL(clicked(int)),
#endif
                     this, SLOT(graphCoordinateSystemToggledCB(int)));
    for (int i = 0; i < LENGTH (coordSysItems); i++)
    {
        QRadioButton *w = new QRadioButton(coordSysItems[i], frame);
        group->addButton(w, i);
#if QT_VERSION >= 0x40000
        layout->addWidget(w);
#endif
        if (whichCoordSystem == i) w->setChecked(true);
    }
}
#endif


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::graphStyleWidgetToggledCB(int which)
//
////////////////////////////////////////////////////////////////////////
{
    whichStyleTemp = which;
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createGraphStyleFrameGuts(QGroupBox *frame)
//
////////////////////////////////////////////////////////////////////////
{
    const char * graphStyleItems[]=
    {
        "Line Style", "Tube Style" , "Surface Style"
    };

// create default selections
    whichStyleOld  = whichStyle;
    whichStyleTemp = whichStyle;

#if QT_VERSION >= 0x40000
    QHBoxLayout *layout = new QHBoxLayout(frame);
#endif
    QButtonGroup *group = new QButtonGroup;
    connect(group, 
#if QT_VERSION >= 0x40000
        SIGNAL(buttonClicked(int)),
#else
        SIGNAL(clicked(int)),
#endif
                     this, SLOT(graphStyleWidgetToggledCB(int)));
    for (int i = 0; i < LENGTH (graphStyleItems); i++)
    {
        QRadioButton *w = new QRadioButton(graphStyleItems[i], frame);
        group->addButton(w, i);
#if QT_VERSION >= 0x40000
        layout->addWidget(w);
#endif
        if (whichStyle == i) w->setChecked(true);
    }
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::graphTypeWidgetToggledCB(int which)
//
////////////////////////////////////////////////////////////////////////
{
    whichTypeTemp = which;
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::graphCoordWidgetToggledCB(int which)
//
////////////////////////////////////////////////////////////////////////
{
    whichCoordTemp = which;
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createGraphTypeFrameGuts(QGroupBox *frame)
//
////////////////////////////////////////////////////////////////////////
{
    const char *graphTypeItems[]={ "Solution Diagram" , "Bifurcation Diagram" };

    whichTypeOld  = whichType;
    whichTypeTemp = whichType;

#if QT_VERSION >= 0x40000
    QHBoxLayout *layout = new QHBoxLayout(frame);
#endif
    QButtonGroup *group = new QButtonGroup;
    connect(group, 
#if QT_VERSION >= 0x40000
        SIGNAL(buttonClicked(int)),
#else
        SIGNAL(clicked(int)),
#endif
                     this, SLOT(graphTypeWidgetToggledCB(int)));
    for (int i = 0; i < LENGTH (graphTypeItems); i++)
    {
        QRadioButton *w = new QRadioButton(graphTypeItems[i], frame);
        group->addButton(w, i);
#if QT_VERSION >= 0x40000
        layout->addWidget(w);
#endif
        if (whichType == i) w->setChecked(true);
    }
}


////////////////////////////////////////////////////////////////////////
//
// callback for all ToggleButtons.
//
void
MainWindow::defaultGraphWidgetToggledCB(int bit)
//
////////////////////////////////////////////////////////////////////////
{
    graphWidgetToggleSetTemp ^= (1 << bit);
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createOptionFrameGuts(QGroupBox *frame)
//
////////////////////////////////////////////////////////////////////////
{
// create default selections

#if QT_VERSION >= 0x40000
    QGridLayout *layout = new QGridLayout(frame, 3, 2);
#endif
    QButtonGroup *group = new QButtonGroup;
    connect(group,
#if QT_VERSION >= 0x40000
        SIGNAL(buttonClicked(int)),
#else
        SIGNAL(clicked(int)),
#endif
                     this, SLOT(defaultGraphWidgetToggledCB(int)));
    for (int i = 0; i < LENGTH (graphWidgetItems); i++)
    {
        graphWidgetToggleSetOld = graphWidgetToggleSet;
        graphWidgetToggleSetTemp= graphWidgetToggleSet;
        QCheckBox *w = new QCheckBox(graphWidgetItems[i], frame);
        group->addButton(w, i);
#if QT_VERSION >= 0x40000
        layout->addWidget(w, i%2, i/2);
#endif
        if (graphWidgetToggleSet & (1<<i)) w->setChecked(true);
    }
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createGraphCoordPartsFrameGuts(QGroupBox *frame)
//
////////////////////////////////////////////////////////////////////////
{
    const char *coordItems[]=
    {
        "No Coordinate", "At Origin" ,
        "At Left && Behind", "At Left && Ahead"  
    };

// create default selections

    whichCoordOld  = whichCoord;
    whichCoordTemp = whichCoord;

#if QT_VERSION >= 0x40000
    QHBoxLayout *layout = new QHBoxLayout(frame);
#endif
    QButtonGroup *group = new QButtonGroup;
    connect(group,
#if QT_VERSION >= 0x40000
        SIGNAL(buttonClicked(int)),
#else
        SIGNAL(clicked(int)),
#endif
                     this, SLOT(graphCoordWidgetToggledCB(int)));
    for (int i = 0; i < LENGTH (coordItems); i++)
    {
        QRadioButton *w = new QRadioButton(coordItems[i], frame);
        group->addButton(w, i);
#if QT_VERSION >= 0x40000
        layout->addWidget(w);
#endif
        if (whichCoord == (unsigned long)i) w->setChecked(true);
    }
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createPreferDefaultPages(QWidget *parent)
//
////////////////////////////////////////////////////////////////////////
{
    QGroupBox *frameList[5];
    int num;
    const char * frmNames[]=
    {
        "Optional Widgets", "Graph Type", "Graph Style",
#ifdef R3B
        "Coordinate System",
#endif
        "Coordinate Parts"
    };

    QVBoxLayout *layout = new QVBoxLayout(parent);
    for(int i=0; i<LENGTH(frmNames); ++i) {
#if QT_VERSION >= 0x40000
        frameList[i] = new QGroupBox(frmNames[i], parent);
#else
#ifdef R3B
        int rows[] = {2, 1, 1, 1, 1};
#else
        int rows[] = {2, 1, 1, 1};
#endif
	frameList[i] = new QGroupBox(rows[i], Qt::Vertical, frmNames[i], parent);
#endif
        layout->addWidget(frameList[i]);
    }
    num = 0;
    createOptionFrameGuts(frameList[num++]);
    createGraphTypeFrameGuts(frameList[num++]);
    createGraphStyleFrameGuts(frameList[num++]);
#ifdef R3B
    createGraphCoordinateSystemFrameGuts(frameList[num++]);
#endif
    createGraphCoordPartsFrameGuts(frameList[num++]);
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::createLineAttPages(QWidget *parent)
//
////////////////////////////////////////////////////////////////////////
{
    const char *tabName[] = { "Line Attributes", "Other Preferences" };
    const char *names[] =
    {
        "DEFAULTS", "BP (ALG)", "LP (ALG)",
        "HB      ", "UZ     4", "UZ    -4",
        "LP (DIF)", "BP (DIF)", "PD      ",
        "TR TORUS", "EP (NOR)", "MX (ABN)",
        "OTHERS  "
    };
    const char *names2[] =
    {
        "Color 1", "Color 2", "Color 3",
        "Color 4", "Color 5", "Color 6",
        "Color 7", "Color 8", "Color 9",
        "Color 10", "Color 11", "Color 12",
        "Color 13"
    };

    QGridLayout *layout = new QGridLayout(parent, 8, 18);
    createColorAndLinePrefSheetHeader(parent, layout, 0);
    createColorAndLinePrefSheetHeader(parent, layout, 9);
    if(coloringMethod == CL_BRANCH_NUMBER)
        createLineAttrPrefSheetParts(parent, layout, names2);
    else
        createLineAttrPrefSheetParts(parent, layout, names);
}


///////////////////////////////////////////////////////////////////////
//
void
MainWindow::createPreferNotebookPages(QTabWidget *notebook)
//
////////////////////////////////////////////////////////////////////////
{
// create the preference sheet shell and form widget
    const char *tabName[] = { "Menu Item Preferences", "Line Attributes" };

// create the first page.
    QWidget *pageForm0 = new QWidget(notebook, "pageForm0");
    createPreferDefaultPages(pageForm0);
    notebook->addTab(pageForm0, tabName[0]);

// create the second page.
    QWidget *pageForm1 = new QWidget(notebook, "pageForm1");
    createLineAttPages(pageForm1);
    notebook->addTab(pageForm1, tabName[1]);
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the preference sheet in a separate window. It
//  calls other routines to create the actual content of the sheet.
//
void
MainWindow::createPreferDialog()
//
////////////////////////////////////////////////////////////////////////
{
    static QDialog *shell;

//    if(!shell)
    {
        shell = new QDialog(this, "Preference Dialog");
        preferDialog = shell;
        shell->setCaption("Preference Dialog");

        QVBoxLayout *panedWin = new QVBoxLayout(shell, 5, -1, "pane");

        // create notebook to hold all the pages
        QTabWidget *notebook = new QTabWidget(shell, "Options");
        createPreferNotebookPages(notebook);
        panedWin->addWidget(notebook);

        QWidget *actionForm = new QWidget(shell, "action form");
        createPreferActionFormControls(actionForm);
        panedWin->addWidget(actionForm);
    }
    shell->show();
}


///////////////////////////////////////////////////////////////////
//         CANCEL CALL BACK
//
void
MainWindow::closePreferDialogAndGiveUpChange()
//
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePatternTemp[i] = linePatternOld[i];
        linePattern[i]     = linePatternOld[i];
        for(int j=0; j<3; ++j)
        {
            lineColorTemp[i][j] = lineColorOld[i][j];
            lineColor[i][j]     = lineColorOld[i][j];
        }
    }

    whichType     = whichTypeOld;
    whichTypeTemp = whichTypeOld;
    typeMenuItems->which            = whichTypeOld;

    whichStyle     = whichStyleOld;
    whichStyleTemp = whichStyleOld;
    styleMenuItems->which            = whichStyleOld;

#ifdef R3B
    whichCoordSystem     = whichCoordSystemOld;
    whichCoordSystemTemp = whichCoordSystemOld;
    coordSystemMenuItems->which           = whichCoordSystemOld;
#endif

    whichCoord     = whichCoordOld;
    whichCoordTemp = whichCoordOld;
    coordMenuItems->which           = whichCoordOld;

// cancel the selections and recover the original values.
    graphWidgetToggleSetTemp = graphWidgetToggleSetOld;
    graphWidgetToggleSet     = graphWidgetToggleSetOld;
    for (int i = 0; i < LENGTH (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetOld & (1<<i)) ? true : false;
    }

    satAniSpeedSlider->setEnabled(options[OPT_SAT_ANI]);
    orbitAniSpeedSlider->setEnabled(options[OPT_PERIOD_ANI]);
    delete preferDialog;
    updateScene();
}


///////////////////////////////////////////////////////////////////
//         OK & CLOSE CALL BACK
//
void
MainWindow::closePreferDialogAndUpdateScene()
//
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePattern[i]    = linePatternTemp[i];
        linePatternOld[i] = linePatternTemp[i];
        for(int j=0; j<3; ++j)
        {
            lineColor[i][j]    = lineColorTemp[i][j];
            lineColorOld[i][j] = lineColorTemp[i][j];
        }
    }

    whichType    = whichTypeTemp;
    whichTypeOld = whichTypeTemp;
    typeMenuItems->which           = whichTypeTemp;

    whichStyle    = whichStyleTemp;
    whichStyleOld = whichStyleTemp;
    styleMenuItems->which           = whichStyleTemp;

#ifdef R3B
    whichCoordSystem    = whichCoordSystemTemp;
    whichCoordSystemOld = whichCoordSystemTemp;
    coordSystemMenuItems->which          = whichCoordSystemTemp;

#endif
    whichCoord = whichCoordTemp;
    whichCoordOld = whichCoordTemp;
    coordMenuItems->which        = whichCoordTemp;

    graphWidgetToggleSet    = graphWidgetToggleSetTemp;
    graphWidgetToggleSetOld = graphWidgetToggleSetTemp;
    for (int i = 0; i < LENGTH (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetTemp & (1<<i)) ? true : false;
    }

    satAniSpeedSlider->setEnabled(options[OPT_SAT_ANI]);
    orbitAniSpeedSlider->setEnabled(options[OPT_PERIOD_ANI]);
    delete preferDialog;
    updateScene();
}


///////////////////////////////////////////////////////////////////
//         OK & SAVE CALL BACK
//
void
MainWindow::savePreferAndUpdateScene()
//
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePattern[i]    = linePatternTemp[i];
        linePatternOld[i] = linePatternTemp[i];
        for(int j=0; j<3; ++j)
        {
            lineColor[i][j]    = lineColorTemp[i][j];
            lineColorOld[i][j] = lineColorTemp[i][j];
        }
    }

    whichType    = whichTypeTemp;
    whichTypeOld = whichTypeTemp;
    typeMenuItems->which           = whichTypeTemp;

    whichStyle    = whichStyleTemp;
    whichStyleOld = whichStyleTemp;
    styleMenuItems->which           = whichStyleTemp;

#ifdef R3B
    whichCoordSystem    = whichCoordSystemTemp;
    whichCoordSystemOld = whichCoordSystemTemp;
    coordSystemMenuItems->which          = whichCoordSystemTemp;
#endif

    whichCoord = whichCoordTemp;
    whichCoordOld = whichCoordTemp;
    coordMenuItems->which        = whichCoordTemp;

    graphWidgetToggleSet    = graphWidgetToggleSetTemp;
    graphWidgetToggleSetOld = graphWidgetToggleSetTemp;
    for (int i = 0; i < LENGTH (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetTemp & (1<<i)) ? true : false;
    }

    satAniSpeedSlider->setEnabled(options[OPT_SAT_ANI]);
    orbitAniSpeedSlider->setEnabled(options[OPT_PERIOD_ANI]);
    updateScene();

    writePreferValuesToFile();
    delete preferDialog;
}


///////////////////////////////////////////////////////////////////
//         APPLY CALL BACK
//
void
MainWindow::applyPreferDialogChangeAndUpdateScene()
//
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePattern[i]=linePatternTemp[i];
        for(int j=0; j<3; ++j)
            lineColor[i][j]=lineColorTemp[i][j];
    }

    whichType = whichTypeTemp;
    typeMenuItems->which        = whichTypeTemp;

    whichStyle = whichStyleTemp;
    styleMenuItems->which        = whichStyleTemp;

#ifdef R3B
    whichCoordSystem = whichCoordSystemTemp;
    coordSystemMenuItems->which       = whichCoordSystemTemp;
#endif

    whichCoord = whichCoordTemp;
    coordMenuItems->which        = whichCoordTemp;

    graphWidgetToggleSet = graphWidgetToggleSetTemp;
    for (int i = 0; i < LENGTH (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetTemp & (1<<i)) ? true : false;
    }

    satAniSpeedSlider->setEnabled(options[OPT_SAT_ANI]);
    orbitAniSpeedSlider->setEnabled(options[OPT_PERIOD_ANI]);
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//        This routine is called to get a file name using the
// standard file dialog.
//
void
MainWindow::getFileName(int fileMode)
//
////////////////////////////////////////////////////////////////////////
{
    QString filename; 

    if(fileMode == SAVE_ITEM)
        filename = QFileDialog::getSaveFileName(QString::null,
                          "Inventor files (*.iv);;Any files (*)", this );
#ifdef R3B
    else if(fileMode == PRINT_ITEM)
        filename = QFileDialog::getSaveFileName(QString::null, QString::null,
                     this, "print file dialog", "Choose a file to print to" );
#endif
    else
        filename = QFileDialog::getOpenFileName(QString::null,
                          "AUTO files (b.* s.* d.*);;Any files (*)", this);
    SbBool okFile = TRUE;
    if(filename == QString::null)
        return;
    if(fileMode == SAVE_ITEM)
        writeToFile(filename);
#ifdef R3B
    else if(fileMode == PRINT_ITEM)
    {
        cropScene(filename);
    }
#endif
    else if(fileMode == OPEN_ITEM)
    {
        deleteScene();
        okFile = readFile(filename);
    }
}


////////////////////////////////////////////////////////////////////////
//
//        Brings up the "ABOUT" dialog
//
void
MainWindow::showAboutDialog()
//
////////////////////////////////////////////////////////////////////////
{
    QString str;
#ifndef R3B
    str  = "  AUTO plaut04\n\n";
#else
    str  = "  AUTO r3bplaut04\n\n";
#endif
    str += "  Zhang, Chenghai, Dr. Eusebius J. Doedel\n\n ";
    str += "  Computer Science Department\n";
    str += "  Concordia University\n\n";
#ifndef R3B
    str += "  Montreal, Quebec\n";
    str += "  CANADA\n\n";
    str += "  August, 2004 \n";
#else
    str += "  Montreal, CA\n\n";
    str += "  June, 2004 \n";
#endif

    QMessageBox::about(this, "About_popup", str);
}

void
MainWindow::showHelp()
{
    showHelpDialog();
}

////////////////////////////////////////////////////////////////////////
//
void
MainWindow::xListCallBack(const QString &str)
//
////////////////////////////////////////////////////////////////////////
{
    char *manyChoice = strdup(str);
    int i = 0;
    for(i=0; i<MAX_LIST; i++)
        xCoordIndices[i] = -1;

    i = 0;
    char * tmp;
    tmp = strtok(manyChoice, ",");
    do
    {
        xCoordIndices[i++] = (strcasecmp(tmp,"t")==0) ? 0 : atoi(tmp);
        tmp = strtok(NULL,",");
    }while(tmp != NULL && i < MAX_LIST); 
    xCoordIdxSize = i;
    updateScene();
    free(manyChoice);
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::yListCallBack(const QString &str)
//
////////////////////////////////////////////////////////////////////////
{
    char *manyChoice = strdup(str);

    int i = 0;
    for(i=0; i<MAX_LIST; i++)
        yCoordIndices[i] = -1;

    i = 0;
    char * tmp;
    tmp = strtok(manyChoice, ",");
    do
    {
        yCoordIndices[i++] = (strcasecmp(tmp,"t")==0) ? 0 : atoi(tmp);
        tmp = strtok(NULL,",");
    }while(tmp != NULL && i < MAX_LIST);
    yCoordIdxSize = i;
    updateScene();
    free(manyChoice);
}


////////////////////////////////////////////////////////////////////////
//
void 
MainWindow::zListCallBack(const QString &str)
//
////////////////////////////////////////////////////////////////////////
{
    char *manyChoice = strdup(str);

    int i = 0;
    for(i=0; i<MAX_LIST; i++)
        zCoordIndices[i] = -1;

    i = 0;
    char * tmp;
    tmp = strtok(manyChoice, ",");
    do
    {
        zCoordIndices[i++] = (strcasecmp(tmp,"t")==0) ? 0 : atoi(tmp);
        tmp = strtok(NULL,",");
    }while(tmp != NULL && i < MAX_LIST);
    zCoordIdxSize = i;
    updateScene();
    free(manyChoice);
}


////////////////////////////////////////////////////////////////////////
//
void
MainWindow::lblListCallBack(const QString &str)
//
////////////////////////////////////////////////////////////////////////
{
    char *manyChoice = strdup(str);
    int choice = labelsList->currentItem();
    int i = 0;
    int nItems = (whichType != BIFURCATION) ? 
                       mySolNode.totalLabels+SP_LBL_ITEMS : 
                       myBifNode.totalLabels+SP_LBL_ITEMS;
    char * tmp;
    static int half = 2;
    tmp = strtok(manyChoice, ",");
    if(choice < 1 || choice >= nItems)
    {
        do
        {
            lblIndices[i++] = (strcasecmp(tmp,"all")==0) ? 0 : atoi(tmp)-myLabels[1]+1;
            tmp = strtok(NULL,",");
        }while(tmp != NULL && i < MAX_LABEL);
        half = 2;
    }
    else if(choice == 1) 
    {
        int j = 1;
        do
        {
#ifdef R3B
            if(abs(clientData.labelIndex[j][2])!= 4 || j%half == 0)
#else
            if(abs(clientData.labelIndex[j-1][2])!= 4 || j%half == 0)
#endif
                lblIndices[i++] = j;
            j++;
        } while( j < numLabels-2 );
        half *= 2;
    }
    else if(choice == 2)
    {
        int j = 1;
        do
        {
#ifndef R3B
            if(clientData.labelIndex[j-1][2] !=  TYPE_UZ  && clientData.labelIndex[j-1][2] != TYPE_RG
            ) // &&
            // clientData.labelIndex[j-1][2] != TYPE_EP_ODE && clientData.labelIndex[j-1][2] != TYPE_MX)
#else
            if(clientData.labelIndex[j][2] !=  TYPE_UZ  && clientData.labelIndex[j][2] != TYPE_RG
            &&
                clientData.labelIndex[j][2] != TYPE_EP_ODE && clientData.labelIndex[j][2] != TYPE_MX)
#endif
                lblIndices[i++] = j;
            j++;
        } while( j < numLabels-2 );
        half = 2;
    }
    else if(choice == 3)
    {
        lblIndices[i++] = numLabels;
        half = 2;
    }
    else
    {
        lblIndices[i++] = choice-3;
        half = 2;
    }
    lblIdxSize = i;

    updateScene();
    free(manyChoice);
}


class FmDrawingArea : public QWidget
{
public:
    FmDrawingArea( QWidget *parent, const char *name, bool ev )
        : QWidget( parent, name ) { eigenvalue = ev; }

protected:
    void paintEvent( QPaintEvent * );
private:
    bool eigenvalue;
};

////////////////////////////////////////////////////////////////////////
//
void
FmDrawingArea::paintEvent( QPaintEvent * )
//
////////////////////////////////////////////////////////////////////////
{
    QPainter p( this );

    static const char *myText[9] =
    {
      "-inf", "-100", "-10", "-1", "0", "1", "10", "100", "+inf"
    };

// draw Y
    p.setPen(QPen(Qt::blue, 2, Qt::SolidLine));
    p.drawLine(200, 0, 200, 400);

// draw X
    p.setPen(QPen(Qt::red, 2, Qt::SolidLine));
    p.drawLine(0, 200, 400, 200);

// draw grid
    p.setPen(QPen(Qt::gray, 1, Qt::DotLine));
    for(int i=0; i<9; ++i)
        p.drawLine(0, 50*i, 400, 50*i);
    for(int i=0; i<9; ++i)
        p.drawLine(i*50, 0, i*50, 400);

// draw text
    p.setPen(Qt::black);
    for(int i = 0; i < 9; ++i)
        p.drawText(i*50+1 , 215, myText[i]);
    for(int i = 0; i < 9; ++i)
        p.drawText(210 , 413-i*50, myText[i]);

// draw a unit circle.
    if (!eigenvalue) {
        p.setPen(QPen(Qt::green, 1, Qt::SolidLine));
        p.drawArc(150, 150, 100, 100, 0, 360*16);
    }

    p.setPen(QPen(Qt::black, 2, Qt::SolidLine));

    int x, y;

    for(int j = 0; j<clientData.numFM; ++j)
    {
        float tmp = fmData[2*j];
        if(fabs(tmp) <= 1.1)
            x = (tmp>0.0) ? (int)(200+tmp*50) : (int)(200-fabs(tmp)*50);
        else
            x =(tmp>0.0) ? (int)(250+log10(tmp)*50) : (int)(150-log10(fabs(tmp))*50);

        tmp = fmData[2*j+1];
        if(fabs(tmp) <= 1.1)
            y = (tmp>0.0) ? (int)(200+tmp*50) : (int)(200-fabs(tmp)*50);
        else
            y =(tmp>0.0) ? (int)(150-log10(tmp)*50): (int)(log10(fabs(tmp))*50+250);

        if(x>390) x = 390; if(x<10) y = 10;
        if(y>390) y = 390; if(y<10) y = 10;

        p.drawLine(x-3, y-3, x+3, y+3);
        p.drawLine(x-3, y+3, x+3, y-3);
    }
}


////////////////////////////////////////////////////////////////////////
//
void popupFloquetMultiplierDialog(float data[], int size, bool eigenvalue)
//
////////////////////////////////////////////////////////////////////////
{
    static QDialog *dialog_shell = (QDialog *) 0;
    QVBoxLayout *pane = (QVBoxLayout *) 0;
    
    QString str = "";

    for(int i=0; i<size; ++i)
    {
        QString temp;
        temp.sprintf("Col[%2d ] = %+E", i+1, data[i]);
        str += temp;
        if(size<20 || (size>=20 && (i+1)%2==0)) str += "\n";
        else str += " | ";
    }

    QString tmpstr = eigenvalue ? "Eigenvalues:\n" : "Floquet multipliers:\n";
    for(int j=0; j<clientData.numFM; ++j)
    {
        QString temp;
        if (fmData[j*2+1] < 0)
            temp.sprintf(" [%2d] : %E - %Ei\n", j, fmData[j*2], -fmData[j*2+1]);
        else if (fmData[j*2+1] > 0)
            temp.sprintf(" [%2d] : %E + %Ei\n", j, fmData[j*2], fmData[j*2+1]);
        else
            temp.sprintf(" [%2d] : %E\n", j, fmData[j*2]);
        tmpstr += temp;
    }

    if (!pane)
    {
        FmDrawingArea *fmDrawingArea;

        dialog_shell = new QDialog(mainWindow, "Dialog", false);
        dialog_shell->setCaption("Dialog");

        pane = new QVBoxLayout(dialog_shell, 5, -1, "pane");

        fmDrawingArea = new FmDrawingArea(dialog_shell, "fmDrawingArea",
                                          eigenvalue);
        fmDrawingArea->setMinimumSize(450,450);
        pane->addWidget(fmDrawingArea);

        QWidget *form = new QWidget(dialog_shell, "form");
        QHBoxLayout *layout = new QHBoxLayout(form);
        layout->setSpacing(5);
        QLabel *label3 = new QLabel(str, form, "label");
        QLabel *label2 = new QLabel(tmpstr, form, "label");
        label2->setAlignment(Qt::AlignLeft);
        layout->addWidget(label3);
        layout->addWidget(label2);
        pane->addWidget(form);

        QPushButton *pushButton= new QPushButton("OK", dialog_shell);
        pushButton->connect(pushButton, SIGNAL(clicked()),
                            dialog_shell, SLOT(accept()));
        pane->addWidget(pushButton);
        dialog_shell->show();
    }
}


void soxtmain(char *argv[])
{
// Initialize Inventor and Qt.
    SoQt::init(argv[0]);

#ifndef R3B
    root = new SoSeparator;
#else
    root = new SoSelection;
#endif
#ifndef R3B
    root->ref();
#endif
    mainWindow = new MainWindow;

    SoQt::show(mainWindow);
    SoQt::mainLoop();
}
