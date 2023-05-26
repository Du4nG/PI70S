/**
 * Includes
 */
#include <stdio.h>
#include <string.h>
#include "TPTInterface.h"
#include "tpt_fusion.h"
#include "tpt_fusion_node.h"
#include "tpt_fusion_obsolete.h"

#include "Sedge_MsgDef.h"
#include "Sedge_CalibDef.h"

/**
 * Includes specific to pst
 */
/* BEGIN ECCo_AutoCode_Section ********************************************* */
bool isInterfaceOpt = TRUE;
int isSycActive = 0;
/* END   ECCo_AutoCode_Section */

#include "Sedge_UserProcs_package.h"

/*
 * ****************************************************************************
 * scheduler time trackers
 * ****************************************************************************
 */

double currTime; //current time in milli Second
double tptDT;
double sycTimeStartPostDrv;
double sycTimeStartPreDrv;
double tptStepSize;

double dEngSpeed;

const double  *ptrArray;

/* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */


/* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */


/* Default Syncro settings */

/* BEGIN ECCo_AutoCode_Section ********************************************* */

//Dummy Values

real32 SEDGeEngineSpeed = 1000.0;
int SEDGeCylinderCount = 4;
int SEDGeEngineType = 3;

int SEDGeSY_GRDWOF = 78;
/* END   ECCo_AutoCode_Section */


#ifdef INISYNCCALL
bool isIniSyncOver = 0;
real32 SEDGeEngineSpeed_Old;
real32 Epm_nEng_IniSync;
#endif

/* BEGIN ECCo_AutoCode_Section ********************************************* */
extern uint32 rba_OsShell_Cntr5msTask;
extern uint32 rba_OsShell_Cntr10msTask;
extern uint32 rba_OsShell_Cntr1000msTask;
extern uint32 rba_OsShell_CntrIniTask;
/* END   ECCo_AutoCode_Section */


/* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */


double next820usTask;
double next1msTask;
double next2msTask;
double next5msTask;
double next10msTask;
double next20msTask;
double next40msTask;
double next50msTask;
double next100msTask;
double next200msTask;
double next1000msTask;
double nextSync0Task;
double nextSync1Task;

int state;
int calPrm_Counter_local;

/* BEGIN ECCo_AutoCode_Section ********************************************* */
static bool POSTDRIVE_FLAG = FALSE;
static bool PROCINIT_FLAG = FALSE;
static bool iniOver = FALSE;
#define SYC_BOOT 0
#define SYC_PROC_INIT 1
#define SYC_PROC_EXEC   2
#define SYC_SHUTDOWN   3
#define SYC_INI 0
#define SYC_INISYN 1
#define SYC_INIEND 2
#define SYC_PREDRIVE 3
#define SYC_DRIVE 4
#define SYC_POSTDRIVE 5
#define SYC_POST_OS_EXIT 6
#define SYCOFF  1
#define SYCPROCINIT 2
#define SYCPREDRIVE 3
#define SYCDRIVE 4
#define SYCPOSTDRIVE 5
static int stateTypeProc = SYCOFF;
char sycType = 'D';
uint8 T15_st=0;
sint16 Epm_nEng=0;
uint8 SyC_stMn=0;
uint8 SyC_stSub=0;
real32 SYC_UserReqProlongTime_PreDrive;
real32 SYC_UserReqProlongTime_PostDrive;
/* END   ECCo_AutoCode_Section */


static bool bigSize = FALSE;

/*
 * ****************************************************************************
 * Enums for tasktype, datatype, messages type etc
 * ****************************************************************************
 */
typedef enum  {UNKNOWN_DIR=-1,   INPUT, OUTPUT, LOCAL} VarType;
typedef enum  {UNKNOWN_TYPE=-1,  PACKED_BIT, BIT, UINT8, UINT16, UINT32, SINT8, SINT16, SINT32, FLOAT32_TPT} DataType;
typedef enum  {UNKNOWN_TASK=-1,  TINI, TSYNCRO, TSYNCRO1, TSYNCRO2, T820US, T1MS, T2MS, T5MS, T10MS, T20MS, T40MS, T50MS, T100MS, T200MS, T1000MS} TPT_TaskType;
typedef enum  {BITREAD, BITWRITE} BitAccess;
typedef enum  {REINI, NOREINI} ReIni;
typedef union { double db; long integer; char* string;} Value;
//typedef enum  {UNKNOWN_COMPU=-1, RAT_FUNC, TAB_VERB,   } CONVERSIONTYPE;
//typedef enum  {UNKNOWN_CAL=-1, VALUE, VALUE_BLOCK, ASCII, CURVE_FIXED,MAP_FIXED, MAP_GROUPED, CURVE_GROUPED, AXIS_VALUES, CURVE_INDIVIDUAL, MAP_INDIVIDUAL} CalPrmType;

#ifndef b_BBasT
    #define b_BBasT  uint8
#endif

#ifndef w_BBasT
    #define w_BBasT  uint16
#endif

#ifndef l_BBasT
    #define l_BBasT  uint32
#endif

#ifndef b_MASK
    #define b_MASK   (uint8)0xFFu
#endif

#ifndef w_MASK
    #define w_MASK   (uint16)0xFFFFu
#endif

#ifndef l_MASK
    #define l_MASK   (uint32)0xFFFFFFFFul
#endif

/*
 * ****************************************************************************
 * Function Prototypes
 * ****************************************************************************
 */
long    getBaseType(DataType);
long    getFunctionIndex(const char*);
char*   getFunctionList();
char*   getFunctionName(long);
long    getFunctionTask(long);
char*   getProjectName();
double* getRatCoeff(long);
char*   getTabVerCoeff(long);
char*   getVariableList();
long    getVariableDirection(long);
long    getVariableIndex(const char*);
char*   getVariableName(long);
char*   getVariableUnit(long);
double  getVariableValuePhys(long);
double  getVariableValueInt(long);
long    setVariableValuePhys(long, double);
long    setVariableValueHex(long, double);
long    setVariableValuePhysString(long, char*);
long    setVariableValueIntern(long, double);
long    getVariableType(long);
long    getVariableCompuType(long);
double getVariableValuePhysArray(long, uint16);
long    setVariableValuePhysArray(long, uint16);
long    setVariableValueHexArray(long, uint16);
void    schedExecute();
void    schedInit(unsigned int);
void    executeIniTask();
void    executeIniSyncTask();
void    executeReIniTask();
void    executeIniEndTask();
void    executeIniDrvTask();
void    resetReIniMsgs();
void    updateVarsPreValues(void);
void    executeInterruptTasksBasedOnCond(void);
void    getDynMemInRuntime(int i);
/*
 * ****************************************************************************
 * Structure definitions for messages, processes, calibration params etc
 * ****************************************************************************
 */

typedef struct {
    char* varName;
    char* unit;
    Value physValue;
    Value intValue; //Not referred at the moment
    VarType varType;
    DataType datType;
    CONVERSIONTYPE compuMethod;
    double ratCoeff[6];
    int nthIndex; //Not referred at the moment
    char** verbArray; //Not referred at the moment
    int   sizeOfArray; 
    char *memLabelName;
    void* address;
    long bitPosition; //Not referred at the moment
    DataType bitBase; //Not referred at the moment
    ReIni isReIni;  
    tpt_fusion_signal_handle tptHandle;
} variable;


typedef struct {
    char* procName;
    void (*pointer)(void);
    TPT_TaskType taskType;
} process;


/*****************************************************************************
 * ECCo AutoCode: tpt_projectName
 *****************************************************************************/

/* BEGIN ECCo_AutoCode_Section ********************************************* */
const char* tpt_projectName = "VehC_SwSPSA_38_53_0_rev1";
/* END   ECCo_AutoCode_Section */


/******************************************************************************
 *  ECCo AutoCode: extern declarations for Inputs
 *****************************************************************************/

/* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */


/******************************************************************************
 *  ECCo AutoCode: extern declarations for Outputs
 *****************************************************************************/

/* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */


/******************************************************************************
 *  ECCo AutoCode: extern declarations for Locals
 *****************************************************************************/

/* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */


/******************************************************************************
    *  ECCo AutoCode: extern declarations for static class Instances
 *****************************************************************************/

  /* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */

  
/******************************************************************************
 *  ECCo AutoCode: Preparation of Array of Variable structs
 *****************************************************************************/

/* BEGIN ECCo_AutoCode_Section ********************************************* */


variable vars[] = {
{   "ACClntP_bCodAvl",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "ACClntP_bCodAvl",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Brk_flgRdntSnsrRaw",
    "",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Brk_flgRdntSnsrRaw",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ABSPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ABSPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ABSPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ABSPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ACC30PR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ACC30PR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ACC30PR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ACC30PR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ACCPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ACCPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ACCPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ACCPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ACCSGPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ACCSGPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ACCSGPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ACCSGPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_BBY1PR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_BBY1PR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_BBY1PR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_BBY1PR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_BPGAPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_BPGAPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_BPGAPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_BPGAPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_BVPR",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_BVPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_BVPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_BVPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CAPR",
    "",
    0.0,
    0,
    OUTPUT,
    UINT16,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_CAPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CAPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    UINT16,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_CAPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CAPTEAUPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_CAPTEAUPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CAPTEAUPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_CAPTEAUPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CHAPR",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_CHAPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CHAPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_CHAPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CHRG_PLUG_TYPEPR_RTE",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_CHRG_PLUG_TYPEPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CPK4PR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_CPK4PR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CPK4PR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_CPK4PR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CPR",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_CPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_CPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_CPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_DCMPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_DCMPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_DCMPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_DCMPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_EASYMOVEPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_EASYMOVEPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_EASYMOVEPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_EASYMOVEPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ESPPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ESPPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_ESPPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_ESPPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_FPR",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_FPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_FPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_FPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_GSRV2_ADASPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_GSRV2_ADASPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_GSRV2_ADASPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_GSRV2_ADASPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_HADCPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_HADCPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_HADCPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_HADCPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_LVVPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_LVVPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_LVVPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_LVVPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_LV_BATT_TECHNOPR_RTE",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_LV_BATT_TECHNOPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_PCPR",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_PCPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_PCPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "CMM_UDS_PCPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_RVVPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_RVVPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_RVVPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_RVVPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_VPPR",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_VPPR",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CMM_UDS_VPPR_RTE",
    "no unit",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "CMM_UDS_VPPR_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Cha_tqMSRReq",
    "Nm",
    0.0,
    0,
    OUTPUT,
    SINT16,
    RAT_FUNC,
    { /**/ 0.0, 16.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "Cha_tqMSRReq",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Cha_tqMSRReq_RTE",
    "N.m",
    0.0,
    0,
    INPUT,
    FLOAT32_TPT,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "Cha_tqMSRReq_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "CrCsHtr_swtEnaBlwBy1",
    "-",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "CrCsHtr_swtEnaBlwBy1",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ABR_38D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ABR_38D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ABR_38D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ABR_38D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ABR_44D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ABR_44D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ABR_44D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ABR_44D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ABR_50D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ABR_50D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ABR_50D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ABR_50D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_Acq_bNeut_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_Acq_bNeut_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_412",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_412",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_412_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_412_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_432",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_432",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_432_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_432_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_56E",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_56E",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_56E_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_56E_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_572",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_572",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_572_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_572_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_612",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_612",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BSI_612_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BSI_612_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BV_4E9",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BV_4E9",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BV_4E9_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BV_4E9_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_BlowBy1Hw_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_BlowBy1Hw_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CLIM_50E",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CLIM_50E",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CLIM_50E_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CLIM_50E_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_1E8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_1E8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_1E8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_1E8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_208",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_208",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_208_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_208_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_228",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_228",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_228_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_228_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_2B8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_2B8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_2B8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_2B8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_2D8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_2D8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_2D8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_2D8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_2F8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_2F8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_2F8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_2F8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_318",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_318",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_318_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_318_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_348",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_348",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_348_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_348_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_3B8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_3B8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_3B8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_3B8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_408",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_408",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_408_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_408_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_438",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_438",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_438_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_438_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_468",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_468",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_468_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_468_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_488",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_488",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_488_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_488_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_578",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_578",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_578_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_578_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_588",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_588",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_588_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_588_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_598",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_598",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_598_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_598_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_5A8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_5A8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_5A8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_5A8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_5B8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_5B8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_5B8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_5B8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_5F8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_5F8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_5F8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_5F8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_BV_2E8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_BV_2E8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CMM_BV_2E8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CMM_BV_2E8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CRASH_4C8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CRASH_4C8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_CRASH_4C8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_CRASH_4C8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_DAT_CMM_E_VCU_508",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_DAT_CMM_E_VCU_508",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_DAT_CMM_E_VCU_508_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_DAT_CMM_E_VCU_508_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ELECTRON_BSI_092",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ELECTRON_BSI_092",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ELECTRON_BSI_092_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ELECTRON_BSI_092_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ESC_355",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ESC_355",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ESC_355_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ESC_355_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ESM_4FE",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ESM_4FE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ESM_4FE_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ESM_4FE_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_IS_MAP_CMM_7A8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_IS_MAP_CMM_7A8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_IS_MAP_CMM_7A8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_IS_MAP_CMM_7A8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_MDD_ETAT_2B6",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_MDD_ETAT_2B6",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_MDD_ETAT_2B6_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_MDD_ETAT_2B6_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_SI_EASY_MOVE_3AD",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_SI_EASY_MOVE_3AD",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_SI_EASY_MOVE_3AD_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_SI_EASY_MOVE_3AD_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_STT_BV_329",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_STT_BV_329",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_STT_BV_329_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_STT_BV_329_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_STT_CMM_3C8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_STT_CMM_3C8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_STT_CMM_3C8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_STT_CMM_3C8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_SecBrkPedPrss",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_SecBrkPedPrss",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_SecBrkPedPrss_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_SecBrkPedPrss_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_StrtAuth",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_StrtAuth",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_StrtAuth_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_StrtAuth_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_UCF_MDD_32D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_UCF_MDD_32D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_UCF_MDD_32D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_UCF_MDD_32D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_UC_FREIN_5ED",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_UC_FREIN_5ED",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_UC_FREIN_5ED_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_UC_FREIN_5ED_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_VOL_305",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_VOL_305",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_VOL_305_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_VOL_305_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAPCLine",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAPCLine",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAPCLine_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAPCLine_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_bEngRStrtReq078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_bEngRStrtReq078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_bEngRStrtReq078_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_bEngRStrtReq078_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_bEngStopAuth078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_bEngStopAuth078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_bEngStopAuth078_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_bEngStopAuth078_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_bEngStopInh078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_bEngStopInh078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_bEngStopInh078_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_bEngStopInh078_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_ctClcProc078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_ctClcProc078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_ctClcProc4FE",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_ctClcProc4FE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_dstVehTot",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_dstVehTot",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_dstVehTot_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_dstVehTot_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noCks078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noCks078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noCks4FE",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noCks4FE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noIdFrame",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noIdFrame",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noIdFrame_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noIdFrame_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noJDDKm",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noJDDKm",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noJDDKm_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noJDDKm_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit1_00E",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit1_00E",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit1_00E_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit1_00E_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit2_00E",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit2_00E",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit2_00E_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit2_00E_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit3_02E",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit3_02E",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit3_02E_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit3_02E_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit4_02E",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit4_02E",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit4_02E_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit4_02E_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit_0A8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit_0A8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit_0A8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit_0A8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit_0E0",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit_0E0",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noKeyCtlUnit_0E0_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noKeyCtlUnit_0E0_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noRcvSrvTypImmo_0A8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noRcvSrvTypImmo_0A8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noRcvSrvTypImmo_0A8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noRcvSrvTypImmo_0A8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noRcvSrvTypImmo_0E0",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noRcvSrvTypImmo_0E0",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_noRcvSrvTypImmo_0E0_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_noRcvSrvTypImmo_0E0_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_sendDataTypImmo_0E8",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_sendDataTypImmo_0E8",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_sendDataTypImmo_0E8_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_sendDataTypImmo_0E8_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stGBxPad078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stGBxPad078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stGBxPad078_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stGBxPad078_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stLifeJDD",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stLifeJDD",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stLifeJDD_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stLifeJDD_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stLockECUCAN_128_1F9",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stLockECUCAN_128_1F9",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stLockECUCAN_128_1F9_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stLockECUCAN_128_1F9_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stLockECUCAN_128_578",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stLockECUCAN_128_578",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stLockECUCAN_128_578_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stLockECUCAN_128_578_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stNbFrame",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stNbFrame",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_stNbFrame_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_stNbFrame_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_tqMaxGBxADAS078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_tqMaxGBxADAS078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_tqMaxGBxADAS078_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_tqMaxGBxADAS078_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_volFuCnsTot",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_volFuCnsTot",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bAcv_volFuCnsTot_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bAcv_volFuCnsTot_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bCluPedPrssSen",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bCluPedPrssSen",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bCluPedPrssSen_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bCluPedPrssSen_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bDft",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bDft",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bDft_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bDft_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bEngRun_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bEngRun_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bFbStStaCmd_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bFbStStaCmd_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bKeyLine",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bKeyLine",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bKeyLine_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bKeyLine_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bLINNwVDAVoltCtlAlt",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bLINNwVDAVoltCtlAlt",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bLINNwVDAVoltCtlAlt_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bLINNwVDAVoltCtlAlt_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrss",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrss",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrssHSCha",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrssHSCha",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrssHSCha_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrssHSCha_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrssHSVeh",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrssHSVeh",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrssHSVeh_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrssHSVeh_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrssMes",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrssMes",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrssMes_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrssMes_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bMainBrkPedPrss_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bMainBrkPedPrss_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bPushLine",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bPushLine",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bPushLine_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bPushLine_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bRCDLine",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bRCDLine",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bRCDLine_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bRCDLine_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bRStrtAuthTra078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bRStrtAuthTra078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bRStrtAuthTra078_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bRStrtAuthTra078_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bSTTDft078",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bSTTDft078",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bSTTDft078_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bSTTDft078_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bSpdFanReqB2",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bSpdFanReqB2",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bSpdFanReqB2_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bSpdFanReqB2_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bSpdFanReqC",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bSpdFanReqC",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bSpdFanReqC_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bSpdFanReqC_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bpRelBrkAsi",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bpRelBrkAsi",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_bpRelBrkAsi_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_bpRelBrkAsi_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_btAirExtMes",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_btAirExtMes",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_btAirExtMes_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_btAirExtMes_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_buOpTrbAct",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_buOpTrbAct",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_buOpTrbAct_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_buOpTrbAct_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc2B6",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc2B6",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc2B6_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc2B6_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc329",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc329",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc329_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc329_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc34D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc34D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc34D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc34D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc38D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc38D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc38D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc38D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc3AD",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc3AD",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc3AD_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc3AD_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc50D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc50D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_ctClcProc50D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_ctClcProc50D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks2B6",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks2B6",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks2B6_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks2B6_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks329",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks329",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks329_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks329_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks38D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks38D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks38D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks38D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks3AD",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks3AD",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks3AD_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks3AD_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks50D",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks50D",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noCks50D_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noCks50D_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noJDD",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noJDD",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_noJDD_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_noJDD_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_pAC_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_pAC_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_rCluPCAN",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_rCluPCAN",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_rCluPCAN_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_rCluPCAN_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_rCluPedPrssMes",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_rCluPedPrssMes",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_rCluPedPrssMes_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_rCluPedPrssMes_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_stDftCod",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_stDftCod",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_stDftCod_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_stDftCod_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_tiVehCnt",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_tiVehCnt",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ctrl_tiVehCnt_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ctrl_tiVehCnt_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "DCDCMgt_stDCDCReq",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "DCDCMgt_stDCDCReq",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "DCDCMgt_stDCDCReq_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "DCDCMgt_stDCDCReq_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "EONV_bHldPwrl",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "EONV_bHldPwrl",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "EngM_rltMassAirScvCor",
    "-",
    0.0,
    0,
    OUTPUT,
    UINT16,
    RAT_FUNC,
    { /**/ 0.0, 65536.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "EngM_rltMassAirScvCor",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "EngM_rltMassAirScvCor_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    FLOAT32_TPT,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "EngM_rltMassAirScvCor_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "EngRot_bArch2010EcoPush",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "EngRot_bArch2010EcoPush",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "EngRot_stActvVrntCod",
    "-",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "EngRot_stActvVrntCod",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bABSCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bABSCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bABSCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bABSCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bASRESPCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bASRESPCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bASRESPCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bASRESPCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bBlowBy1Cf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bBlowBy1Cf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bBlowBy1Cf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bBlowBy1Cf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bBrkParkCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bBrkParkCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bBrkParkCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bBrkParkCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bCoReqVehCf_bEngStrtReq",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bCoReqVehCf_bEngStrtReq",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bCoReqVehCf_bEngStrtReq_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bCoReqVehCf_bEngStrtReq_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bEPBCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bEPBCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bEPBCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bEPBCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bEasyMoveCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bEasyMoveCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bEasyMoveCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bEasyMoveCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bEngRun_Archi2010EcoPush_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bEngRun_Archi2010EcoPush_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bSecBrkPedPrssRaw",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bSecBrkPedPrssRaw",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bStaCmdFctSt",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bStaCmdFctSt",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bStaCmdFctSt_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bStaCmdFctSt_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bStrtDrvlfCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bStrtDrvlfCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bStrtDrvlfCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bStrtDrvlfCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bTrbTypCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bTrbTypCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bTrbTypCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bTrbTypCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bVSCtlStbGcCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bVSCtlStbGcCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bVSCtlStbGcCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bVSCtlStbGcCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bVSLimCf",
    "",
    0.0,
    0,
    OUTPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bVSLimCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_bVSLimCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    BIT,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default */},
    0,
    NULL,
    0,
    "Ext_bVSLimCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_spdVehVSMaxPrmaReq1_RTE",
    "km/h",
    0.0,
    0,
    OUTPUT,
    FLOAT32_TPT,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "Ext_spdVehVSMaxPrmaReq1_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_spdVehVSMaxPrmaReq2_RTE",
    "km/h",
    0.0,
    0,
    OUTPUT,
    FLOAT32_TPT,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "Ext_spdVehVSMaxPrmaReq2_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stACTypCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stACTypCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stACTypCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stACTypCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stAltClasVarCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stAltClasVarCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stAltClasVarCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stAltClasVarCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stBrkArchiTypCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stBrkArchiTypCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stBrkArchiTypCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stBrkArchiTypCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stBrkAsiTypCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stBrkAsiTypCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stBrkAsiTypCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stBrkAsiTypCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stCfFan",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stCfFan",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stCfFan_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stCfFan_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stElProdCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stElProdCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stElProdCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stElProdCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stNwArchiCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stNwArchiCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stNwArchiCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stNwArchiCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stPosShunt",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stPosShunt",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stPosShunt_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stPosShunt_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stStgPmpCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stStgPmpCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stStgPmpCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stStgPmpCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stTraTypCf",
    "",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stTraTypCf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "Ext_stTraTypCf_RTE",
    "no unit",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1 /* SEDGe default for TAB_VERB compu method */},
    0,
    NULL,
    0,
    "Ext_stTraTypCf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_dstVehTotRec",
    "km",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 10.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_dstVehTotRec",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_dstVehTotRec_RTE",
    "km",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -10.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_dstVehTotRec_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiEngRunLf",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiEngRunLf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiEngRunLf_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiEngRunLf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiEngRunRec",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiEngRunRec",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiEngRunRec_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiEngRunRec_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlEngRunLf",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlEngRunLf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlEngRunLf_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlEngRunLf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlEngRunRec",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlEngRunRec",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlEngRunRec_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlEngRunRec_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlPwtAcvLf",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlPwtAcvLf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlPwtAcvLf_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlPwtAcvLf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlPwtAcvRec",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlPwtAcvRec",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiIdlPwtAcvRec_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiIdlPwtAcvRec_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvLf",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvLf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvLf_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvLf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvRec",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvRec",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvRec_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvRec_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvUrbLf",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvUrbLf",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvUrbLf_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvUrbLf_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvUrbRec",
    "s",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvUrbRec",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_tiStPwtAcvUrbRec_RTE",
    "s",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -1.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_tiStPwtAcvUrbRec_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_volFuCnsTotRec",
    "l",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 100.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_volFuCnsTotRec",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "FLIC_volFuCnsTotRec_RTE",
    "L",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, -100.0, 0.0, /**/ 0.0, 0.0,-1.0 /**/ },
    0,
    NULL,
    0,
    "FLIC_volFuCnsTotRec_RTE",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GbxNPos_stActvVrntCod",
    "-",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GbxNPos_stActvVrntCod",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_codVarCodBV",
    "-",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_codVarCodBV",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_codVarCodC",
    "-",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_codVarCodC",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_codVarCodCA",
    "-",
    0.0,
    0,
    INPUT,
    UINT16,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_codVarCodCA",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_codVarCodCHA",
    "-",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_codVarCodCHA",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_codVarCodF",
    "-",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_codVarCodF",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_codVarCodPC",
    "-",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_codVarCodPC",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_codVarCodUC",
    "-",
    0.0,
    0,
    INPUT,
    UINT32,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_codVarCodUC",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "GlbDa_vVehSpdLim",
    "km/h",
    0.0,
    0,
    INPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "GlbDa_vVehSpdLim",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "StrtrEna_stActvVrntCod",
    "-",
    0.0,
    0,
    OUTPUT,
    UINT8,
    RAT_FUNC,
    { /**/ 0.0, 1.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "StrtrEna_stActvVrntCod",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "TqRes_tqBlowByRes",
    "Nm",
    0.0,
    0,
    OUTPUT,
    UINT16,
    RAT_FUNC,
    { /**/ 0.0, 16.0, 0.0, /**/ 0.0, 0.0,1.0 /**/ },
    0,
    NULL,
    0,
    "TqRes_tqBlowByRes",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
/*ECCo_AutoStubbing*/
/*ECCo_AutoStubbing_Param*/
/*Task_Counters*/
{   "rba_OsShell_Cntr5msTask", 
    "-",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1},
    0,
    NULL,
    NULL,
    "rba_OsShell_Cntr5msTask",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "rba_OsShell_Cntr10msTask", 
    "-",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1},
    0,
    NULL,
    NULL,
    "rba_OsShell_Cntr10msTask",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "rba_OsShell_Cntr1000msTask", 
    "-",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1},
    0,
    NULL,
    NULL,
    "rba_OsShell_Cntr1000msTask",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
{   "rba_OsShell_CntrIniTask", 
    "-",
    0.0,
    0,
    OUTPUT,
    UINT32,
    RAT_FUNC,
    {0, 1, 0, 0, 0, 1},
    0,
    NULL,
    NULL,
    "rba_OsShell_CntrIniTask",
    NULL,
    0,
    UNKNOWN_TYPE,
    NOREINI,
    NULL},
};
#define NUMBER_OF_VARS (sizeof(vars)/sizeof(vars[0]))

#define NUMBER_OF_DSM_DEBOUNCE_CALPRMS 0

#define NUMBER_OF_DSM_DSBLMASK_CALPRMS 0

#define NUMBER_OF_CALPRMS 8

#define TOTAL_NUMBER_OF_CALPRMS 8 /* TOTAL_NUMBER_OF_CALPRMS = NUMBER_OF_CALPRMS + NUMBER_OF_DSM_DEBOUNCE_CALPRMS + NUMBER_OF_DSM_DSBLMASK_CALPRMS (if Functional DSM Emulation is enabled) + NUMBER_OF_DYSYC_CALPRMS */

CalPrm *calPrmArray[TOTAL_NUMBER_OF_CALPRMS+1];
/* END   ECCo_AutoCode_Section */


/******************************************************************************
 *  ECCo AutoCode: Preparation of Array of Process structs
 *****************************************************************************/

/* BEGIN ECCo_AutoCode_Section ********************************************* */
void VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Init();
/* Begin of added by tool																						*/
/*	List of PROC Init:																							*/
void VehC_SwSPSA_Proc_Cha_tqMSRReq_Init();
void VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Init();
void VehC_SwSPSA_Proc_Ctrl_bRCDLine_Init();
void VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Init();
void VehC_SwSPSA_Proc_Ctrl_BSI_432_Init();
void VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Init();
void VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Init();
void VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_208_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_348_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_588_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Init();
void VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Init();
void VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Init();
void VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Init();
void VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Init();
void VehC_SwSPSA_Proc_Ext_bTrbTypCf_Init();
void VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Init();
void VehC_SwSPSA_Proc_Ext_stPosShunt_Init();
void VehC_SwSPSA_Proc_Ext_stStgPmpCf_Init();
/*	List of PROC Run:																							*/
/* End of added by tool																							*/
void VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Run();
void VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Init();
void VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Run();
void VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Init();
void VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Run();
void VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Init();
void VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Run();
void VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Init();
void VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Run();
void VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Init();
void VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Run();
void VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Init();
void VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Run();
void VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Init();
void VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Run();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Init();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Run();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Init();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Run();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Init();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Run();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Init();
void VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Run();
void VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Init();
void VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Run();
void VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Init();
void VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Run();
void VehC_SwSPSA_Proc_GlbDa_codVarCodC_Init();
void VehC_SwSPSA_Proc_GlbDa_codVarCodC_Run();
void VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Init();
void VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Run();
void VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Init();
void VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Run();
void VehC_SwSPSA_Proc_GlbDa_codVarCodF_Init();
void VehC_SwSPSA_Proc_GlbDa_codVarCodF_Run();
void VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Init();
void VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Run();
void VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Init();
void VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Run();
void VehC_SwSPSA_Proc_Cha_tqMSRReq_Run();
void VehC_SwSPSA_Proc_Ctrl_noJDD_Init();
void VehC_SwSPSA_Proc_Ctrl_noJDD_Run();
void VehC_SwSPSA_Proc_GlbDa_vVehSpdLim_Run();
void VehC_SwSPSA_Proc_Stub_Init();
void VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Run();
void VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Init();
void VehC_SwSPSA_Proc_Stub_Run();
void VehC_SwSPSA_Proc_Ctrl_ABR_38D_Init();
void VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Run();
void VehC_SwSPSA_Proc_Ctrl_ABR_44D_Init();
void VehC_SwSPSA_Proc_Ctrl_ABR_38D_Run();
void VehC_SwSPSA_Proc_Ctrl_ABR_50D_Init();
void VehC_SwSPSA_Proc_Ctrl_ABR_44D_Run();
void VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Init();
void VehC_SwSPSA_Proc_Ctrl_ABR_50D_Run();
void VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Run();
void VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Run();
void VehC_SwSPSA_Proc_Ctrl_bAPCLine_Init();
void VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Run();
void VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Init();
void VehC_SwSPSA_Proc_Ctrl_bAPCLine_Run();
void VehC_SwSPSA_Proc_Ctrl_bDft_Init();
void VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Run();
void VehC_SwSPSA_Proc_Ctrl_bEngRun_Init();
void VehC_SwSPSA_Proc_Ctrl_bDft_Run();
void VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Init();
void VehC_SwSPSA_Proc_Ctrl_bEngRun_Run();
void VehC_SwSPSA_Proc_Ctrl_bKeyLine_Init();
void VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Run();
void VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Init();
void VehC_SwSPSA_Proc_Ctrl_bKeyLine_Run();
void VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Init();
void VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Run();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Init();
void VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Run();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Init();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Run();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Init();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Run();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Init();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Run();
void VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Init();
void VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Run();
void VehC_SwSPSA_Proc_Ctrl_bPushLine_Init();
void VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Run();
void VehC_SwSPSA_Proc_Ctrl_bPushLine_Run();
void VehC_SwSPSA_Proc_Ctrl_bRCDLine_Run();
void VehC_SwSPSA_Proc_Ctrl_BSI_412_Init();
void VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Run();
void VehC_SwSPSA_Proc_Ctrl_BSI_412_Run();
void VehC_SwSPSA_Proc_Ctrl_BSI_56E_Init();
void VehC_SwSPSA_Proc_Ctrl_BSI_432_Run();
void VehC_SwSPSA_Proc_Ctrl_BSI_572_Init();
void VehC_SwSPSA_Proc_Ctrl_BSI_56E_Run();
void VehC_SwSPSA_Proc_Ctrl_BSI_612_Init();
void VehC_SwSPSA_Proc_Ctrl_BSI_572_Run();
void VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Init();
void VehC_SwSPSA_Proc_Ctrl_BSI_612_Run();
void VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Init();
void VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Run();
void VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Run();
void VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Run();
void VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Run();
void VehC_SwSPSA_Proc_Ctrl_BV_4E9_Init();
void VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Run();
void VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Init();
void VehC_SwSPSA_Proc_Ctrl_BV_4E9_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Init();
void VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_228_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_208_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_228_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_318_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_318_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_348_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_408_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_438_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_408_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_468_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_438_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_488_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_468_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_578_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_488_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_578_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_598_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_588_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_598_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Init();
void VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Run();
void VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Run();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Init();
void VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Run();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Init();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Run();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Init();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Run();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Init();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Run();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Init();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Run();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Init();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Run();
void VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Init();
void VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Run();
void VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Run();
void VehC_SwSPSA_Proc_Ctrl_ESC_355_Init();
void VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Run();
void VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Init();
void VehC_SwSPSA_Proc_Ctrl_ESC_355_Run();
void VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Init();
void VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Run();
void VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Init();
void VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Run();
void VehC_SwSPSA_Proc_Ctrl_noCks2B6_Init();
void VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Run();
void VehC_SwSPSA_Proc_Ctrl_noCks329_Init();
void VehC_SwSPSA_Proc_Ctrl_noCks2B6_Run();
void VehC_SwSPSA_Proc_Ctrl_noCks38D_Init();
void VehC_SwSPSA_Proc_Ctrl_noCks329_Run();
void VehC_SwSPSA_Proc_Ctrl_noCks3AD_Init();
void VehC_SwSPSA_Proc_Ctrl_noCks38D_Run();
void VehC_SwSPSA_Proc_Ctrl_noCks50D_Init();
void VehC_SwSPSA_Proc_Ctrl_noCks3AD_Run();
void VehC_SwSPSA_Proc_Ctrl_pAC_Init();
void VehC_SwSPSA_Proc_Ctrl_noCks50D_Run();
void VehC_SwSPSA_Proc_Ctrl_pAC_Run();
void VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Init();
void VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Run();
void VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Run();
void VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Init();
void VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Run();
void VehC_SwSPSA_Proc_Ctrl_stDftCod_Init();
void VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Run();
void VehC_SwSPSA_Proc_Ctrl_StrtAuth_Init();
void VehC_SwSPSA_Proc_Ctrl_stDftCod_Run();
void VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Init();
void VehC_SwSPSA_Proc_Ctrl_StrtAuth_Run();
void VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Init();
void VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Run();
void VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Init();
void VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Run();
void VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Init();
void VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Run();
void VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Init();
void VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Run();
void VehC_SwSPSA_Proc_Ctrl_VOL_305_Init();
void VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Run();
void VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Init();
void VehC_SwSPSA_Proc_Ctrl_VOL_305_Run();
void VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Init();
void VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Run();
void VehC_SwSPSA_Proc_Ext_bABSCf_Init();
void VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Run();
void VehC_SwSPSA_Proc_Ext_bASRESPCf_Init();
void VehC_SwSPSA_Proc_Ext_bABSCf_Run();
void VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Init();
void VehC_SwSPSA_Proc_Ext_bASRESPCf_Run();
void VehC_SwSPSA_Proc_Ext_bBrkParkCf_Init();
void VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Run();
void VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Init();
void VehC_SwSPSA_Proc_Ext_bBrkParkCf_Run();
void VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Init();
void VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Run();
void VehC_SwSPSA_Proc_Ext_bEPBCf_Init();
void VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Run();
void VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Init();
void VehC_SwSPSA_Proc_Ext_bEPBCf_Run();
void VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Run();
void VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Init();
void VehC_SwSPSA_Proc_Ext_bTrbTypCf_Run();
void VehC_SwSPSA_Proc_Ext_bVSLimCf_Init();
void VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Run();
void VehC_SwSPSA_Proc_Ext_stACTypCf_Init();
void VehC_SwSPSA_Proc_Ext_bVSLimCf_Run();
void VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Init();
void VehC_SwSPSA_Proc_Ext_stACTypCf_Run();
void VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Run();
void VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Init();
void VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Run();
void VehC_SwSPSA_Proc_Ext_stCfFan_Init();
void VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Run();
void VehC_SwSPSA_Proc_Ext_stElProdCf_Init();
void VehC_SwSPSA_Proc_Ext_stCfFan_Run();
void VehC_SwSPSA_Proc_Ext_stNwArchiCf_Init();
void VehC_SwSPSA_Proc_Ext_stElProdCf_Run();
void VehC_SwSPSA_Proc_Ext_stNwArchiCf_Run();
void VehC_SwSPSA_Proc_Ext_stPosShunt_Run();
void VehC_SwSPSA_Proc_Ext_stTraTypCf_Init();
void VehC_SwSPSA_Proc_Ext_stStgPmpCf_Run();
void VehC_SwSPSA_Proc_Ext_stTraTypCf_Run();
process procs[] = {
/* Begin of added by tool																						*/
/*	List of PROC Init:																							*/
{	"VehC_SwSPSA_Proc_Cha_tqMSRReq_Init",
	&VehC_SwSPSA_Proc_Cha_tqMSRReq_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Init",
	&VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Init",
	&VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bRCDLine_Init",
	&VehC_SwSPSA_Proc_Ctrl_bRCDLine_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Init",
	&VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_BSI_432_Init",
	&VehC_SwSPSA_Proc_Ctrl_BSI_432_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Init",
	&VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Init",
	&VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Init",
	&VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_CMM_208_Init",
	&VehC_SwSPSA_Proc_Ctrl_CMM_208_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_CMM_348_Init",
	&VehC_SwSPSA_Proc_Ctrl_CMM_348_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_CMM_588_Init",
	&VehC_SwSPSA_Proc_Ctrl_CMM_588_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Init",
	&VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Init",
	&VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Init",
	&VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Init",
	&VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Init",
	&VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ext_bTrbTypCf_Init",
	&VehC_SwSPSA_Proc_Ext_bTrbTypCf_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Init",
	&VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ext_stPosShunt_Init",
	&VehC_SwSPSA_Proc_Ext_stPosShunt_Init,
	TINI},
{	"VehC_SwSPSA_Proc_Ext_stStgPmpCf_Init",
	&VehC_SwSPSA_Proc_Ext_stStgPmpCf_Init,
	TINI},
/*	List of PROC Run:																							*/
/* End of added by tool																							*/
{   "VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Init",
    &VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Run",
    &VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Init",
    &VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Run",
    &VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Init",
    &VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Run",
    &VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Init",
    &VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Run",
    &VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Init",
    &VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Run",
    &VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Init",
    &VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Run",
    &VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Init",
    &VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Run",
    &VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Init",
    &VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Run",
    &VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Init",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Run",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Init",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Run",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Init",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Run",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Init",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Run",
    &VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Init",
    &VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Init,
    TINI},
{   "VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Run",
    &VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Init",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Init,
    TINI},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Run",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodC_Init",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodC_Init,
    TINI},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodC_Run",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodC_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Init",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Init,
    TINI},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Run",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Init",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Init,
    TINI},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Run",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodF_Init",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodF_Init,
    TINI},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodF_Run",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodF_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Init",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Init,
    TINI},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Run",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Init",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Init,
    TINI},
{   "VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Run",
    &VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Cha_tqMSRReq_Run",
    &VehC_SwSPSA_Proc_Cha_tqMSRReq_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_noJDD_Init",
    &VehC_SwSPSA_Proc_Ctrl_noJDD_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_noJDD_Run",
    &VehC_SwSPSA_Proc_Ctrl_noJDD_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_GlbDa_vVehSpdLim_Run",
    &VehC_SwSPSA_Proc_GlbDa_vVehSpdLim_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Stub_Init",
    &VehC_SwSPSA_Proc_Stub_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Run",
    &VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Init",
    &VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Stub_Run",
    &VehC_SwSPSA_Proc_Stub_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ABR_38D_Init",
    &VehC_SwSPSA_Proc_Ctrl_ABR_38D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Run",
    &VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_ABR_44D_Init",
    &VehC_SwSPSA_Proc_Ctrl_ABR_44D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ABR_38D_Run",
    &VehC_SwSPSA_Proc_Ctrl_ABR_38D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ABR_50D_Init",
    &VehC_SwSPSA_Proc_Ctrl_ABR_50D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ABR_44D_Run",
    &VehC_SwSPSA_Proc_Ctrl_ABR_44D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Init",
    &VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ABR_50D_Run",
    &VehC_SwSPSA_Proc_Ctrl_ABR_50D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Run",
    &VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Init",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Init",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Init",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Init",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bAPCLine_Init",
    &VehC_SwSPSA_Proc_Ctrl_bAPCLine_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Init",
    &VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bAPCLine_Run",
    &VehC_SwSPSA_Proc_Ctrl_bAPCLine_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bDft_Init",
    &VehC_SwSPSA_Proc_Ctrl_bDft_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Run",
    &VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bEngRun_Init",
    &VehC_SwSPSA_Proc_Ctrl_bEngRun_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bDft_Run",
    &VehC_SwSPSA_Proc_Ctrl_bDft_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Init",
    &VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bEngRun_Run",
    &VehC_SwSPSA_Proc_Ctrl_bEngRun_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bKeyLine_Init",
    &VehC_SwSPSA_Proc_Ctrl_bKeyLine_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Run",
    &VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Init",
    &VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bKeyLine_Run",
    &VehC_SwSPSA_Proc_Ctrl_bKeyLine_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Init",
    &VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Run",
    &VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Init",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Run",
    &VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Init",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Run",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Init",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Run",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Init",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Run",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Init",
    &VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Run",
    &VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bPushLine_Init",
    &VehC_SwSPSA_Proc_Ctrl_bPushLine_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Run",
    &VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bPushLine_Run",
    &VehC_SwSPSA_Proc_Ctrl_bPushLine_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bRCDLine_Run",
    &VehC_SwSPSA_Proc_Ctrl_bRCDLine_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_412_Init",
    &VehC_SwSPSA_Proc_Ctrl_BSI_412_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Run",
    &VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_412_Run",
    &VehC_SwSPSA_Proc_Ctrl_BSI_412_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_56E_Init",
    &VehC_SwSPSA_Proc_Ctrl_BSI_56E_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_432_Run",
    &VehC_SwSPSA_Proc_Ctrl_BSI_432_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_572_Init",
    &VehC_SwSPSA_Proc_Ctrl_BSI_572_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_56E_Run",
    &VehC_SwSPSA_Proc_Ctrl_BSI_56E_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_612_Init",
    &VehC_SwSPSA_Proc_Ctrl_BSI_612_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_572_Run",
    &VehC_SwSPSA_Proc_Ctrl_BSI_572_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Init",
    &VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_BSI_612_Run",
    &VehC_SwSPSA_Proc_Ctrl_BSI_612_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Init",
    &VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Run",
    &VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Run",
    &VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Run",
    &VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Run",
    &VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_BV_4E9_Init",
    &VehC_SwSPSA_Proc_Ctrl_BV_4E9_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Run",
    &VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Init",
    &VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_BV_4E9_Run",
    &VehC_SwSPSA_Proc_Ctrl_BV_4E9_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Run",
    &VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_228_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_228_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_208_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_208_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_228_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_228_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_318_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_318_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_318_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_318_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_348_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_348_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_408_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_408_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_438_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_438_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_408_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_408_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_468_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_468_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_438_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_438_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_488_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_488_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_468_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_468_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_578_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_578_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_488_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_488_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_578_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_578_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_598_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_598_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_588_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_588_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_598_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_598_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Init",
    &VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Init",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Run",
    &VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Init",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Run",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Init",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Run",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Init",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Run",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Init",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Run",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Init",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Run",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Init",
    &VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Run",
    &VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Run",
    &VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_ESC_355_Init",
    &VehC_SwSPSA_Proc_Ctrl_ESC_355_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Run",
    &VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Init",
    &VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ESC_355_Run",
    &VehC_SwSPSA_Proc_Ctrl_ESC_355_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Init",
    &VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Run",
    &VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Init",
    &VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Run",
    &VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_noCks2B6_Init",
    &VehC_SwSPSA_Proc_Ctrl_noCks2B6_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Run",
    &VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_noCks329_Init",
    &VehC_SwSPSA_Proc_Ctrl_noCks329_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_noCks2B6_Run",
    &VehC_SwSPSA_Proc_Ctrl_noCks2B6_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_noCks38D_Init",
    &VehC_SwSPSA_Proc_Ctrl_noCks38D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_noCks329_Run",
    &VehC_SwSPSA_Proc_Ctrl_noCks329_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_noCks3AD_Init",
    &VehC_SwSPSA_Proc_Ctrl_noCks3AD_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_noCks38D_Run",
    &VehC_SwSPSA_Proc_Ctrl_noCks38D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_noCks50D_Init",
    &VehC_SwSPSA_Proc_Ctrl_noCks50D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_noCks3AD_Run",
    &VehC_SwSPSA_Proc_Ctrl_noCks3AD_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_pAC_Init",
    &VehC_SwSPSA_Proc_Ctrl_pAC_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_noCks50D_Run",
    &VehC_SwSPSA_Proc_Ctrl_noCks50D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_pAC_Run",
    &VehC_SwSPSA_Proc_Ctrl_pAC_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Init",
    &VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Run",
    &VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Run",
    &VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Init",
    &VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Run",
    &VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ctrl_stDftCod_Init",
    &VehC_SwSPSA_Proc_Ctrl_stDftCod_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Run",
    &VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_StrtAuth_Init",
    &VehC_SwSPSA_Proc_Ctrl_StrtAuth_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_stDftCod_Run",
    &VehC_SwSPSA_Proc_Ctrl_stDftCod_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Init",
    &VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_StrtAuth_Run",
    &VehC_SwSPSA_Proc_Ctrl_StrtAuth_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Init",
    &VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Run",
    &VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Init",
    &VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Run",
    &VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Init",
    &VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Run",
    &VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Init",
    &VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Run",
    &VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ctrl_VOL_305_Init",
    &VehC_SwSPSA_Proc_Ctrl_VOL_305_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Run",
    &VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Init",
    &VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ctrl_VOL_305_Run",
    &VehC_SwSPSA_Proc_Ctrl_VOL_305_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Init",
    &VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Init,
    TINI},
{   "VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Run",
    &VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ext_bABSCf_Init",
    &VehC_SwSPSA_Proc_Ext_bABSCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Run",
    &VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ext_bASRESPCf_Init",
    &VehC_SwSPSA_Proc_Ext_bASRESPCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bABSCf_Run",
    &VehC_SwSPSA_Proc_Ext_bABSCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Init",
    &VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bASRESPCf_Run",
    &VehC_SwSPSA_Proc_Ext_bASRESPCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bBrkParkCf_Init",
    &VehC_SwSPSA_Proc_Ext_bBrkParkCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Run",
    &VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Init",
    &VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bBrkParkCf_Run",
    &VehC_SwSPSA_Proc_Ext_bBrkParkCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Init",
    &VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Run",
    &VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bEPBCf_Init",
    &VehC_SwSPSA_Proc_Ext_bEPBCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Run",
    &VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Init",
    &VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bEPBCf_Run",
    &VehC_SwSPSA_Proc_Ext_bEPBCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Run",
    &VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Init",
    &VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bTrbTypCf_Run",
    &VehC_SwSPSA_Proc_Ext_bTrbTypCf_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ext_bVSLimCf_Init",
    &VehC_SwSPSA_Proc_Ext_bVSLimCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Run",
    &VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stACTypCf_Init",
    &VehC_SwSPSA_Proc_Ext_stACTypCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_bVSLimCf_Run",
    &VehC_SwSPSA_Proc_Ext_bVSLimCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Init",
    &VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_stACTypCf_Run",
    &VehC_SwSPSA_Proc_Ext_stACTypCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Run",
    &VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Init",
    &VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Run",
    &VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ext_stCfFan_Init",
    &VehC_SwSPSA_Proc_Ext_stCfFan_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Run",
    &VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stElProdCf_Init",
    &VehC_SwSPSA_Proc_Ext_stElProdCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_stCfFan_Run",
    &VehC_SwSPSA_Proc_Ext_stCfFan_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stNwArchiCf_Init",
    &VehC_SwSPSA_Proc_Ext_stNwArchiCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_stElProdCf_Run",
    &VehC_SwSPSA_Proc_Ext_stElProdCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stNwArchiCf_Run",
    &VehC_SwSPSA_Proc_Ext_stNwArchiCf_Run,
    T1000MS},
{   "VehC_SwSPSA_Proc_Ext_stPosShunt_Run",
    &VehC_SwSPSA_Proc_Ext_stPosShunt_Run,
    T5MS},
{   "VehC_SwSPSA_Proc_Ext_stTraTypCf_Init",
    &VehC_SwSPSA_Proc_Ext_stTraTypCf_Init,
    TINI},
{   "VehC_SwSPSA_Proc_Ext_stStgPmpCf_Run",
    &VehC_SwSPSA_Proc_Ext_stStgPmpCf_Run,
    T10MS},
{   "VehC_SwSPSA_Proc_Ext_stTraTypCf_Run",
    &VehC_SwSPSA_Proc_Ext_stTraTypCf_Run,
    T1000MS},
};
#define NUMBER_OF_PROCS 269
/* END   ECCo_AutoCode_Section */


/******************************************************************************
 * SYC State Machine
 *****************************************************************************/
void sycFunction(){
    double compRasterTime = 0;
    double currTimeIn1msResolution = next1msTask -1.0;
    SYC_UserReqProlongTime_PostDrive = (SYC_UserReqProlongTime_PostDrive>0)?SYC_UserReqProlongTime_PostDrive:5.00;
    switch(sycType)
    {
        case 'D':
            if(T15_st==1 && stateTypeProc ==SYCOFF) {
                stateTypeProc =SYCPROCINIT;
                SyC_stMn = SYC_PROC_INIT;
                SyC_stSub =SYC_INI;
                if(iniOver == FALSE ){
                    executeReIniTask();
                    executeIniTask();
                    executeIniSyncTask();
                    executeIniEndTask();
                }
                compRasterTime = currTime;
            }
            if((iniOver) && (stateTypeProc ==SYCPROCINIT) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc =SYCPREDRIVE;
                sycTimeStartPreDrv = currTimeIn1msResolution;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //PTEST-1715
                if(EEPROM_InitMode){
                    FILE *orgeepFilePtr = fopen("Org_EEPEmu_values.txt","r");
                    if(orgeepFilePtr)
                    {
                        loadEEPRomFile("Org_EEPEmu_values.txt");
                    }else{
                        printf("The Org_EEPEmu_values.txt is not available to load.");
                    }
                    fclose(orgeepFilePtr);
                }
                else{
                    loadEEPRomFile("EEPEmu_values.txt");
                }
                // copy blocks to RAM variable
                //loadEEPRam();
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub =SYC_PREDRIVE;
                iniOver = FALSE;
                compRasterTime = currTime;
            }
            if((T15_st==1) && (stateTypeProc ==SYCPREDRIVE) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc = SYCDRIVE;
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub =SYC_DRIVE;
                executeIniDrvTask();
                compRasterTime = currTime;
            }
            if((T15_st==0) && (stateTypeProc == SYCDRIVE) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc=SYCPOSTDRIVE;
                sycTimeStartPostDrv = currTime/1000;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //copy mirrored RAM blocks to virtual ROM
                //saveEEPRam();
                //save ROM to workspace variable
                saveEEPRomFile("EEPEmu_values.txt");
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub =SYC_POSTDRIVE;
                compRasterTime = currTime;
            }
            if((T15_st == 1) && (stateTypeProc == SYCPOSTDRIVE) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc=SYCDRIVE;
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub =SYC_DRIVE;
                executeReIniTask();
                executeIniDrvTask();
                compRasterTime = currTime;
            }
            if(((SEDGeEngineSpeed == 0) && (T15_st==0) && (((currTime/1000) - sycTimeStartPostDrv)>SYC_UserReqProlongTime_PostDrive)) && (stateTypeProc == SYCPOSTDRIVE)) {
                stateTypeProc =SYCPREDRIVE;
                sycTimeStartPreDrv = currTimeIn1msResolution;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //PTEST-1715
                if(EEPROM_InitMode){
                    FILE *orgeepFilePtr = fopen("Org_EEPEmu_values.txt","r");
                    if(orgeepFilePtr)
                    {
                        loadEEPRomFile("Org_EEPEmu_values.txt");
                    }else{
                        printf("The Org_EEPEmu_values.txt is not available to load.");
                    }
                    fclose(orgeepFilePtr);
                }
                else{
                    loadEEPRomFile("EEPEmu_values.txt");
                }
                // copy blocks to RAM variable
                //loadEEPRam();
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub =SYC_PREDRIVE;
                executeReIniTask();
                compRasterTime = currTime;
            }
            if((T15_st==0) && (stateTypeProc == SYCPREDRIVE)  && (currTimeIn1msResolution - sycTimeStartPreDrv)>SYC_UserReqProlongTime_PreDrive*1000) {
                stateTypeProc =SYCOFF;
                SyC_stMn = SYC_SHUTDOWN;
                SyC_stSub =SYC_POST_OS_EXIT;
            }
            break;

        case 'G':
            if(T15_st==1 && stateTypeProc ==SYCOFF) {
                stateTypeProc =SYCPROCINIT;
                SyC_stMn = SYC_PROC_INIT;
                SyC_stSub = SYC_INI;
                if(iniOver == FALSE ){
                    executeReIniTask();
                    executeIniTask();
                    executeIniSyncTask();
                    executeIniEndTask();
                }
                compRasterTime = currTime;
            }
            if((iniOver) && (stateTypeProc ==SYCPROCINIT) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc = SYCPREDRIVE;
                sycTimeStartPreDrv = currTimeIn1msResolution;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //PTEST-1715
                if(EEPROM_InitMode){
                    FILE *orgeepFilePtr = fopen("Org_EEPEmu_values.txt","r");
                    if(orgeepFilePtr)
                    {
                        loadEEPRomFile("Org_EEPEmu_values.txt");
                    }else{
                        printf("The Org_EEPEmu_values.txt is not available to load.");
                    }
                    fclose(orgeepFilePtr);
                }
                else{
                    loadEEPRomFile("EEPEmu_values.txt");
                }
                // copy blocks to RAM variable
                //  loadEEPRam();
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_PREDRIVE;
                iniOver = FALSE;
                compRasterTime = currTime;
            }
            if((T15_st==1) && (stateTypeProc ==SYCPREDRIVE) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc = SYCDRIVE;
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_DRIVE;
                executeIniDrvTask();
                compRasterTime = currTime;
            }
            if((T15_st==0) && (stateTypeProc == SYCDRIVE) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc=SYCPOSTDRIVE;
                sycTimeStartPostDrv = currTime/1000;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //copy mirrored RAM blocks to virtual ROM
                //saveEEPRam();
                //save ROM to workspace variable
                saveEEPRomFile("EEPEmu_values.txt");
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_POSTDRIVE;
                compRasterTime = currTime;
            }
            if((((SEDGeEngineSpeed == 0) && (T15_st==0) && (((currTime/1000) - sycTimeStartPostDrv)>SYC_UserReqProlongTime_PostDrive)) || (T15_st==1))&& (stateTypeProc == SYCPOSTDRIVE)) {
                stateTypeProc = SYCPROCINIT;
                SyC_stMn = SYC_PROC_INIT;
                SyC_stSub = SYC_INI;
                if(iniOver == FALSE ){
                    executeReIniTask();
                    executeIniTask();
                    executeIniSyncTask();
                    executeIniEndTask();
                }
                compRasterTime = currTime;
            }
            if((T15_st==0) && (stateTypeProc == SYCPREDRIVE) && (currTimeIn1msResolution - sycTimeStartPreDrv)>SYC_UserReqProlongTime_PreDrive * 1000) {
                stateTypeProc =SYCOFF;
                SyC_stMn = SYC_SHUTDOWN;
                SyC_stSub = SYC_POST_OS_EXIT;
            }
            break;

        case 'V':
            if(T15_st==1 && stateTypeProc ==SYCOFF) {
                stateTypeProc =SYCPROCINIT;
                SyC_stMn = SYC_PROC_INIT;
                SyC_stSub = SYC_INI;
                PROCINIT_FLAG = TRUE;
                if(iniOver == FALSE ){
                    executeReIniTask();
                    executeIniTask();
                    executeIniSyncTask();
                    executeIniEndTask();
                }
                compRasterTime = currTime;
            }
            if((iniOver) && (stateTypeProc ==SYCPROCINIT) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc =SYCPREDRIVE;
                sycTimeStartPreDrv = currTimeIn1msResolution;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //PTEST-1715
                if(EEPROM_InitMode){
                    FILE *orgeepFilePtr = fopen("Org_EEPEmu_values.txt","r");
                    if(orgeepFilePtr)
                    {
                        loadEEPRomFile("Org_EEPEmu_values.txt");
                    }else{
                        printf("The Org_EEPEmu_values.txt is not available to load.");
                    }
                    fclose(orgeepFilePtr);
                }
                else{
                    loadEEPRomFile("EEPEmu_values.txt");
                }
                // copy blocks to RAM variable
                // loadEEPRam();
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_PREDRIVE;
                iniOver = FALSE;
                compRasterTime = currTime;
            }
            if((T15_st==1) && (stateTypeProc ==SYCPREDRIVE) && (PROCINIT_FLAG) && ((currTime -compRasterTime)>tptStepSize) ) {
                stateTypeProc = SYCDRIVE;
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_DRIVE;
                PROCINIT_FLAG = FALSE;
                executeIniDrvTask();
                compRasterTime = currTime;
            }
            if((T15_st==0) && (stateTypeProc == SYCDRIVE) && ((currTime -compRasterTime)>tptStepSize)) {
                POSTDRIVE_FLAG = TRUE;
                stateTypeProc = SYCPOSTDRIVE;
                sycTimeStartPostDrv = currTime/1000;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //copy mirrored RAM blocks to virtual ROM
                //saveEEPRam();
                //save ROM to workspace variable
                saveEEPRomFile("EEPEmu_values.txt");
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_POSTDRIVE;
                compRasterTime = currTime;
            }
            if((T15_st == 1) && (stateTypeProc == SYCPOSTDRIVE) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc=SYCDRIVE;
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_DRIVE;
                executeReIniTask();
                executeIniDrvTask();
                compRasterTime = currTime*10;
            }
            if(((SEDGeEngineSpeed == 0) && (T15_st==0) && (((currTime/1000) - sycTimeStartPostDrv)>SYC_UserReqProlongTime_PostDrive)) && (stateTypeProc == SYCPOSTDRIVE)) {
                stateTypeProc =SYCPREDRIVE;
                // PTEST-688
#ifdef ECCO_EEP_EMU
                //PTEST-1715
                if(EEPROM_InitMode){
                    FILE *orgeepFilePtr = fopen("Org_EEPEmu_values.txt","r");
                    if(orgeepFilePtr)
                    {
                        loadEEPRomFile("Org_EEPEmu_values.txt");
                    }else{
                        printf("The Org_EEPEmu_values.txt is not available to load.");
                    }
                    fclose(orgeepFilePtr);
                }
                else{
                    loadEEPRomFile("EEPEmu_values.txt");
                }
                // copy blocks to RAM variable
                //loadEEPRam();
#endif
                SyC_stMn = SYC_PROC_EXEC;
                SyC_stSub = SYC_PREDRIVE;
                executeReIniTask();
                compRasterTime = currTime;
            }
            if((T15_st==0) && (stateTypeProc == SYCPREDRIVE) && ((currTimeIn1msResolution - sycTimeStartPreDrv)>SYC_UserReqProlongTime_PreDrive * 1000)) {
                stateTypeProc =SYCOFF;
                SyC_stMn = SYC_SHUTDOWN;
                SyC_stSub = SYC_POST_OS_EXIT;
                compRasterTime = currTime;
            }
            if((T15_st==1) && (stateTypeProc == SYCPREDRIVE) && (POSTDRIVE_FLAG) && ((currTime -compRasterTime)>tptStepSize)) {
                stateTypeProc =SYCPROCINIT;
                SyC_stMn = SYC_PROC_INIT;
                SyC_stSub = SYC_INI;
                POSTDRIVE_FLAG = FALSE;
                if(iniOver == FALSE ){
                    executeReIniTask();
                    executeIniTask();
                    executeIniSyncTask();
                    executeIniEndTask();
                }
            }
            break;

        default:
            break;
    }

}

void updateVarsPreValues(void){
#ifdef INISYNCCALL
    SEDGeEngineSpeed_Old = SEDGeEngineSpeed;
#endif
}


void executeInterruptTasksBasedOnCond(void){

#ifdef INISYNCCALL
    // To call IniSync process once whenever Epm_nEng falls below a threshold value (Epm_nEng_IniSync)
    if(!isIniSyncOver && SEDGeEngineSpeed_Old > Epm_nEng_IniSync && SEDGeEngineSpeed <= Epm_nEng_IniSync){
        executeIniSyncTask();
        isIniSyncOver = 1;
    }
    if(isIniSyncOver && SEDGeEngineSpeed > Epm_nEng_IniSync && SEDGeEngineSpeed_Old <= Epm_nEng_IniSync){
        isIniSyncOver = 0;
    }
#endif
}

/******************************************************************************
 *  ECCo AutoCode: Prepare list of Process calls for time dependent tasks
 *****************************************************************************/




/* BEGIN ECCo_AutoCode_Section ********************************************* */
void execute820usTask()
{
	ESC_tiSampling = ((signed long) (0.82 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_820us();
	SEDGe_Post_Proc_820us();
}

void execute1msTask()
{
	ESC_tiSampling = ((signed long) (1 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_1ms();
	SEDGe_Post_Proc_1ms();
}

void execute2msTask()
{
	ESC_tiSampling = ((signed long) (2 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_2ms();
	SEDGe_Post_Proc_2ms();
}

void execute5msTask()
{
	ESC_tiSampling = ((signed long) (5 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_5ms();
	VehC_SwSPSA_Proc_Ext_stPosShunt_Run();
	SEDGe_Post_Proc_5ms();
    rba_OsShell_Cntr5msTask++;
}

void execute10msTask()
{
	ESC_tiSampling = ((signed long) (10 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_10ms();
/* Begin of added by tool																						*/
/*	List moved from PROC Sync:																					*/
/* End of added by tool																							*/
	VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Run();
	VehC_SwSPSA_Proc_GlbDa_vVehSpdLim_Run();
	VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Run();
	VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Run();
	VehC_SwSPSA_Proc_GlbDa_codVarCodF_Run();
	VehC_SwSPSA_Proc_GlbDa_codVarCodC_Run();
	VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Run();
	VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Run();
	VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Run();
	VehC_SwSPSA_Proc_Ext_stStgPmpCf_Run();
	VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Run();
	VehC_SwSPSA_Proc_Ext_bTrbTypCf_Run();
	VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Run();
	VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Run();
	VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Run();
	VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Run();
	VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Run();
	VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Run();
	VehC_SwSPSA_Proc_Ctrl_bRCDLine_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Run();
	VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Run();
	VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Run();
	VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_588_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_348_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_208_Run();
	VehC_SwSPSA_Proc_Ctrl_BSI_432_Run();
	VehC_SwSPSA_Proc_Cha_tqMSRReq_Run();
	VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Run();
	VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Run();

    // VehC_SwSPSA_Proc_Cha_tqMSRReq_Init();
    // VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Init();
    // VehC_SwSPSA_Proc_Ctrl_bEngRun_Init();
    // VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Init();
    // VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Init();
    // VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Init();
    
	SEDGe_Post_Proc_10ms();
    rba_OsShell_Cntr10msTask++;
}

void execute20msTask()
{
	ESC_tiSampling = ((signed long) (20 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_20ms();
	SEDGe_Post_Proc_20ms();
}

void execute40msTask()
{
	ESC_tiSampling = ((signed long) (40 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_40ms();
	SEDGe_Post_Proc_40ms();
}

void execute50msTask()
{
	ESC_tiSampling = ((signed long) (50 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_50ms();
	SEDGe_Post_Proc_50ms();
}

void execute100msTask()
{
	ESC_tiSampling = ((signed long) (100 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_100ms();
	SEDGe_Post_Proc_100ms();
}

void execute200msTask()
{
	ESC_tiSampling = ((signed long) (200 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_200ms();
	SEDGe_Post_Proc_200ms();
}

void execute1000msTask()
{
	ESC_tiSampling = ((signed long) (1000 * 1e3));
	ESC_tiSampling_s = ESC_tiSampling;
	SEDGe_Pre_Proc_1000ms();
	VehC_SwSPSA_Proc_Stub_Run();
	VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Run();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Run();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Run();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Run();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Run();
	VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Run();
	VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Run();
	VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Run();
	VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Run();
	VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Run();
	VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Run();
	VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Run();
	VehC_SwSPSA_Proc_Ext_stTraTypCf_Run();
	VehC_SwSPSA_Proc_Ext_stNwArchiCf_Run();
	VehC_SwSPSA_Proc_Ext_stElProdCf_Run();
	VehC_SwSPSA_Proc_Ext_stCfFan_Run();
	VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Run();
	VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Run();
	VehC_SwSPSA_Proc_Ext_stACTypCf_Run();
	VehC_SwSPSA_Proc_Ext_bVSLimCf_Run();
	VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Run();
	VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Run();
	VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Run();
	VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Run();
	VehC_SwSPSA_Proc_Ext_bEPBCf_Run();
	VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Run();
	VehC_SwSPSA_Proc_Ext_bBrkParkCf_Run();
	VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Run();
	VehC_SwSPSA_Proc_Ext_bASRESPCf_Run();
	VehC_SwSPSA_Proc_Ext_bABSCf_Run();
	VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Run();
	VehC_SwSPSA_Proc_Ctrl_stDftCod_Run();
	VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Run();
	VehC_SwSPSA_Proc_Ctrl_pAC_Run();
	VehC_SwSPSA_Proc_Ctrl_noJDD_Run();
	VehC_SwSPSA_Proc_Ctrl_noCks50D_Run();
	VehC_SwSPSA_Proc_Ctrl_noCks3AD_Run();
	VehC_SwSPSA_Proc_Ctrl_noCks38D_Run();
	VehC_SwSPSA_Proc_Ctrl_noCks329_Run();
	VehC_SwSPSA_Proc_Ctrl_noCks2B6_Run();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Run();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Run();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Run();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Run();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Run();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Run();
	VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Run();
	VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Run();
	VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Run();
	VehC_SwSPSA_Proc_Ctrl_bPushLine_Run();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Run();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Run();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Run();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Run();
	VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Run();
	VehC_SwSPSA_Proc_Ctrl_bKeyLine_Run();
	VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Run();
	VehC_SwSPSA_Proc_Ctrl_bEngRun_Run();
	VehC_SwSPSA_Proc_Ctrl_bDft_Run();
	VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Run();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Run();
	VehC_SwSPSA_Proc_Ctrl_bAPCLine_Run();
	VehC_SwSPSA_Proc_Ctrl_VOL_305_Run();
	VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Run();
	VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Run();
	VehC_SwSPSA_Proc_Ctrl_StrtAuth_Run();
	VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Run();
	VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Run();
	VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Run();
	VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Run();
	VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Run();
	VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Run();
	VehC_SwSPSA_Proc_Ctrl_ESC_355_Run();
	VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_598_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_578_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_488_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_468_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_438_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_408_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_318_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_228_Run();
	VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Run();
	VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Run();
	VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Run();
	VehC_SwSPSA_Proc_Ctrl_BV_4E9_Run();
	VehC_SwSPSA_Proc_Ctrl_BSI_612_Run();
	VehC_SwSPSA_Proc_Ctrl_BSI_572_Run();
	VehC_SwSPSA_Proc_Ctrl_BSI_56E_Run();
	VehC_SwSPSA_Proc_Ctrl_BSI_412_Run();
	VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Run();
	VehC_SwSPSA_Proc_Ctrl_ABR_50D_Run();
	VehC_SwSPSA_Proc_Ctrl_ABR_44D_Run();
	VehC_SwSPSA_Proc_Ctrl_ABR_38D_Run();
	SEDGe_Post_Proc_1000ms();
    rba_OsShell_Cntr1000msTask++;
}

void executeIniTask()
{
	SEDGe_Pre_Proc_INI();
/* Begin of added by tool																						*/
	VehC_SwSPSA_Proc_Cha_tqMSRReq_Init();
	VehC_SwSPSA_Proc_Ext_bStaCmdFctSt_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_bEngRStrtReq078_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopAuth078_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_bEngStopInh078_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_dstVehTot_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit1_00E_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit2_00E_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit3_02E_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit4_02E_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0A8_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noKeyCtlUnit_0E0_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0A8_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noRcvSrvTypImmo_0E0_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_sendDataTypImmo_0E8_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stGBxPad078_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_1F9_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stLockECUCAN_128_578_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_tqMaxGBxADAS078_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_volFuCnsTot_Init();
	VehC_SwSPSA_Proc_Ctrl_bRCDLine_Init();
	VehC_SwSPSA_Proc_Ctrl_bRStrtAuthTra078_Init();
	VehC_SwSPSA_Proc_Ctrl_BSI_432_Init();
	VehC_SwSPSA_Proc_Ctrl_bSTTDft078_Init();
	VehC_SwSPSA_Proc_Ctrl_btAirExtMes_Init();
	VehC_SwSPSA_Proc_Ctrl_buOpTrbAct_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_208_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_348_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_588_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_5F8_Init();
	VehC_SwSPSA_Proc_Ctrl_CRASH_4C8_Init();
	VehC_SwSPSA_Proc_Ctrl_ELECTRON_BSI_092_Init();
	VehC_SwSPSA_Proc_Ctrl_rCluPCAN_Init();
	VehC_SwSPSA_Proc_Ctrl_SecBrkPedPrss_Init();
	VehC_SwSPSA_Proc_Ext_bTrbTypCf_Init();
	VehC_SwSPSA_Proc_Ext_stBrkArchiTypCf_Init();
	VehC_SwSPSA_Proc_Ext_stPosShunt_Init();
	VehC_SwSPSA_Proc_Ext_stStgPmpCf_Init();
/* End of added by tool																							*/
	VehC_SwSPSA_Proc_GlbDa_codVarCodUC_Init();
	VehC_SwSPSA_Proc_GlbDa_codVarCodPC_Init();
	VehC_SwSPSA_Proc_GlbDa_codVarCodF_Init();
	VehC_SwSPSA_Proc_GlbDa_codVarCodC_Init();
	VehC_SwSPSA_Proc_GlbDa_codVarCodCHA_Init();
	VehC_SwSPSA_Proc_GlbDa_codVarCodCA_Init();
	VehC_SwSPSA_Proc_GlbDa_codVarCodBV_Init();
	VehC_SwSPSA_Proc_Brk_flgRdntSnsrRaw_Init();
	VehC_SwSPSA_Proc_Stub_Init();
	VehC_SwSPSA_Proc_FLIC_volFuCnsTotRec_Init();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbRec_Init();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvUrbLf_Init();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvRec_Init();
	VehC_SwSPSA_Proc_FLIC_tiStPwtAcvLf_Init();
	VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvRec_Init();
	VehC_SwSPSA_Proc_FLIC_tiIdlPwtAcvLf_Init();
	VehC_SwSPSA_Proc_FLIC_tiIdlEngRunRec_Init();
	VehC_SwSPSA_Proc_FLIC_tiIdlEngRunLf_Init();
	VehC_SwSPSA_Proc_FLIC_tiEngRunRec_Init();
	VehC_SwSPSA_Proc_FLIC_tiEngRunLf_Init();
	VehC_SwSPSA_Proc_FLIC_dstVehTotRec_Init();
	VehC_SwSPSA_Proc_EngM_rltMassAirScvCor_Init();
	VehC_SwSPSA_Proc_DCDCMgt_stDCDCReq_Init();
	VehC_SwSPSA_Proc_Ext_stTraTypCf_Init();
	VehC_SwSPSA_Proc_Ext_stNwArchiCf_Init();
	VehC_SwSPSA_Proc_Ext_stElProdCf_Init();
	VehC_SwSPSA_Proc_Ext_stCfFan_Init();
	VehC_SwSPSA_Proc_Ext_stBrkAsiTypCf_Init();
	VehC_SwSPSA_Proc_Ext_stAltClasVarCf_Init();
	VehC_SwSPSA_Proc_Ext_stACTypCf_Init();
	VehC_SwSPSA_Proc_Ext_bVSLimCf_Init();
	VehC_SwSPSA_Proc_Ext_bVSCtlStbGcCf_Init();
	VehC_SwSPSA_Proc_Ext_bStrtDrvlfCf_Init();
	VehC_SwSPSA_Proc_Ext_bEngRun_Archi2010EcoPush_Init();
	VehC_SwSPSA_Proc_Ext_bEasyMoveCf_Init();
	VehC_SwSPSA_Proc_Ext_bEPBCf_Init();
	VehC_SwSPSA_Proc_Ext_bCoReqVehCf_bEngStrtReq_Init();
	VehC_SwSPSA_Proc_Ext_bBrkParkCf_Init();
	VehC_SwSPSA_Proc_Ext_bBlowBy1Cf_Init();
	VehC_SwSPSA_Proc_Ext_bASRESPCf_Init();
	VehC_SwSPSA_Proc_Ext_bABSCf_Init();
	VehC_SwSPSA_Proc_Ctrl_tiVehCnt_Init();
	VehC_SwSPSA_Proc_Ctrl_stDftCod_Init();
	VehC_SwSPSA_Proc_Ctrl_rCluPedPrssMes_Init();
	VehC_SwSPSA_Proc_Ctrl_pAC_Init();
	VehC_SwSPSA_Proc_Ctrl_noJDD_Init();
	VehC_SwSPSA_Proc_Ctrl_noCks50D_Init();
	VehC_SwSPSA_Proc_Ctrl_noCks3AD_Init();
	VehC_SwSPSA_Proc_Ctrl_noCks38D_Init();
	VehC_SwSPSA_Proc_Ctrl_noCks329_Init();
	VehC_SwSPSA_Proc_Ctrl_noCks2B6_Init();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc50D_Init();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc3AD_Init();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc38D_Init();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc34D_Init();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc329_Init();
	VehC_SwSPSA_Proc_Ctrl_ctClcProc2B6_Init();
	VehC_SwSPSA_Proc_Ctrl_bpRelBrkAsi_Init();
	VehC_SwSPSA_Proc_Ctrl_bSpdFanReqC_Init();
	VehC_SwSPSA_Proc_Ctrl_bSpdFanReqB2_Init();
	VehC_SwSPSA_Proc_Ctrl_bPushLine_Init();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrss_Init();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssMes_Init();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSVeh_Init();
	VehC_SwSPSA_Proc_Ctrl_bMainBrkPedPrssHSCha_Init();
	VehC_SwSPSA_Proc_Ctrl_bLINNwVDAVoltCtlAlt_Init();
	VehC_SwSPSA_Proc_Ctrl_bKeyLine_Init();
	VehC_SwSPSA_Proc_Ctrl_bFbStStaCmd_Init();
	VehC_SwSPSA_Proc_Ctrl_bEngRun_Init();
	VehC_SwSPSA_Proc_Ctrl_bDft_Init();
	VehC_SwSPSA_Proc_Ctrl_bCluPedPrssSen_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stNbFrame_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_stLifeJDD_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noJDDKm_Init();
	VehC_SwSPSA_Proc_Ctrl_bAcv_noIdFrame_Init();
	VehC_SwSPSA_Proc_Ctrl_bAPCLine_Init();
	VehC_SwSPSA_Proc_Ctrl_VOL_305_Init();
	VehC_SwSPSA_Proc_Ctrl_UC_FREIN_5ED_Init();
	VehC_SwSPSA_Proc_Ctrl_UCF_MDD_32D_Init();
	VehC_SwSPSA_Proc_Ctrl_StrtAuth_Init();
	VehC_SwSPSA_Proc_Ctrl_STT_CMM_3C8_Init();
	VehC_SwSPSA_Proc_Ctrl_STT_BV_329_Init();
	VehC_SwSPSA_Proc_Ctrl_SI_EASY_MOVE_3AD_Init();
	VehC_SwSPSA_Proc_Ctrl_MDD_ETAT_2B6_Init();
	VehC_SwSPSA_Proc_Ctrl_IS_MAP_CMM_7A8_Init();
	VehC_SwSPSA_Proc_Ctrl_ESM_4FE_Init();
	VehC_SwSPSA_Proc_Ctrl_ESC_355_Init();
	VehC_SwSPSA_Proc_Ctrl_DAT_CMM_E_VCU_508_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_BV_2E8_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_5B8_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_5A8_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_598_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_578_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_488_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_468_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_438_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_408_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_3B8_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_318_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_2F8_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_2D8_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_2B8_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_228_Init();
	VehC_SwSPSA_Proc_Ctrl_CMM_1E8_Init();
	VehC_SwSPSA_Proc_Ctrl_CLIM_50E_Init();
	VehC_SwSPSA_Proc_Ctrl_BlowBy1Hw_Init();
	VehC_SwSPSA_Proc_Ctrl_BV_4E9_Init();
	VehC_SwSPSA_Proc_Ctrl_BSI_612_Init();
	VehC_SwSPSA_Proc_Ctrl_BSI_572_Init();
	VehC_SwSPSA_Proc_Ctrl_BSI_56E_Init();
	VehC_SwSPSA_Proc_Ctrl_BSI_412_Init();
	VehC_SwSPSA_Proc_Ctrl_Acq_bNeut_Init();
	VehC_SwSPSA_Proc_Ctrl_ABR_50D_Init();
	VehC_SwSPSA_Proc_Ctrl_ABR_44D_Init();
	VehC_SwSPSA_Proc_Ctrl_ABR_38D_Init();
	SEDGe_Post_Proc_INI();
    rba_OsShell_CntrIniTask++;
}

void executeIniSyncTask()
{
	SEDGe_Pre_Proc_INISYNC();
	SEDGe_Post_Proc_INISYNC();
}

void executeReIniTask()
{
	resetReIniMsgs();
	SEDGe_Pre_Proc_REINI();
	SEDGe_Post_Proc_REINI();
}

void executeIniDrvTask()
{
	SEDGe_Pre_Proc_INIDRV();
	SEDGe_Post_Proc_INIDRV();
}

void executeIniEndTask()
{
	SEDGe_Pre_Proc_INIEND();
	SEDGe_Post_Proc_INIEND();
    iniOver=TRUE;
}

void executeSyncroTask0()
{
	SEDGe_Pre_Proc_SYNC_TASK0();
	SEDGe_Post_Proc_SYNC_TASK0();
}

void executeSyncroTask1()
{
	SEDGe_Pre_Proc_SYNC_TASK1();
	SEDGe_Post_Proc_SYNC_TASK1();
}

/* END   ECCo_AutoCode_Section */


/*****************************************************************************/


/* Function: getBaseType =====================================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */
long getBaseType(DataType b)
{
    if(b == UNKNOWN_TYPE) return 1;
    if(b == PACKED_BIT) return 1;
    if(b == BIT) return 1;
    if(b == UINT8) return 1;
    if(b == UINT16) return 3;
    if(b == UINT32) return 5;
    if(b == SINT8) return 2;
    if(b == SINT16) return 4;
    if(b == SINT32) return 6;
    if(b == FLOAT32_TPT) return 7;
}

/* Function: getProjectName ===================================================
 * Abstract:
 * This function returns the name of the project for which the simulation is being
 * done.
 * @param
 * @return the project name in as in ECCo DLL
 * ============================================================================
 */
char* getProjectName()
{
    char *s;
    s = (char*) malloc((strlen(tpt_projectName)+1)* sizeof(char));
    strcpy(s,tpt_projectName);
    return (s);
}

/* Function: getVariableList ===================================================
 * Abstract:
 * This function returns list of messages/signals known to the function
 * @param
 * @return list of variables
 * ============================================================================
 */
char* getVariableList()
{
    int i, len=0;
    for(i=0;i<NUMBER_OF_VARS;i++)
        len = len + strlen(vars[i].varName) + 1;
    char *s = (char*) malloc((len + 1)* sizeof(char));
    strcpy(s,"");
    for(i=0;i<NUMBER_OF_VARS;i++)
    {
        strcat(s,vars[i].varName);
        strcat(s,";");
    }
    return (s);
}

/* Function: getFunctionList ==================================================
 * Abstract:
 * This function returns the list of processes in the function being tested
 * @param
 * @return list of ECU Processes
 * ============================================================================
 */
char* getFunctionList()
{
    int i, len=0;
    for(i=0;i<NUMBER_OF_PROCS;i++)
        len = len + strlen(procs[i].procName) + 1;
    char *s = (char*) malloc((len + 1)* sizeof(char));
    strcpy(s,"");
    for(i=0;i<NUMBER_OF_PROCS;i++)
    {
        strcat(s,procs[i].procName);
        strcat(s,";");
    }
    return (s);
}

/* Function: getVariableIndex =================================================
 * Abstract:
 * This function returns an index of supplied variable in the variable list
 * @param: name of the variable
 * @return: Index of variable
 * ============================================================================
 */
long getVariableIndex(const char* var)
{
    long i;
    for(i=0; i<NUMBER_OF_VARS; i++)
       if(strcmp(var,vars[i].varName)==0)
           return i;
    return (long)-1;
}

/* Function: getFunctionIndex =================================================
 * Abstract:
 * This function returns index of ECU Process in the process list
 * @param: Name of the ECU Process
 * @return: Index of process
 * ============================================================================
 */
long getFunctionIndex(const char* proc)
{
    long i;
    for(i=0; i<NUMBER_OF_PROCS; i++)
       if(strcmp(proc,procs[i].procName)==0)
           return i;
    return (long)-1;
}

/* Function: getVariableName ==================================================
 * Abstract:
 * This function returns the name of the variable at particular index in the variable list
 * @param: The index of variable
 * @return: The name of the variable at the given index
 * ============================================================================
 */
char* getVariableName(long i)
{
    char *s = (char*) malloc((strlen(vars[i].varName)+1)* sizeof(char));
    if(i<0 || i>=NUMBER_OF_VARS)
        return NULL;
    strcpy(s,vars[i].varName);
    return s;
}

/* Function: getFunctionName ==================================================
 * Abstract:
 * This function returns the name of the ECU Process at given index in the process list
 * @param: The index of Process
 * @return: The name of the ECU Process at the given index
 * ============================================================================
 */
char* getFunctionName(long i)
{
    char *s = (char*) malloc((strlen(procs[i].procName)+1)* sizeof(char));
    if(i<0 || i>=NUMBER_OF_PROCS)
        return NULL;
    strcpy(s,procs[i].procName);
    return s;
}

/* Function: getVariableUnit ==================================================
 * Abstract:
 * This function returns the unit of the given variable
 * @param: The index of variable whose unit is required
 * @return: unit of the variable
 * ============================================================================
 */
char* getVariableUnit(long i)
{
    char *s = (char*) malloc((strlen(vars[i].unit)+1)* sizeof(char));
    if(i<0 || i>=NUMBER_OF_VARS)
        return NULL;
    strcpy(s,vars[i].unit);
    return s;
}

/* Function: getVariableValuePhys =============================================
 * Abstract:
 * This function returns the physical value of the variable
 * @param: The index of variable whose physical value is required
 * @return: The physical value in double
 * ============================================================================
 */
double getVariableValuePhys(long i)
{
    int(*func)(BitAccess,int)=NULL;

    if(i<0 || i>=NUMBER_OF_VARS)
        return 0.0;

    if(vars[i].datType == BIT || vars[i].datType == PACKED_BIT)
    {
   		if(!isInterfaceOpt){
        	func = vars[i].address;
	        vars[i].physValue.db = (double)func(BITREAD, 0);
        }
        else{
                 if(vars[i].bitBase==UINT8 || vars[i].bitBase == UNKNOWN_TYPE){
                       uint8 tempBit = *((uint8*)(vars[i].address));
                       tempBit = (tempBit & ((b_BBasT)1u << vars[i].bitPosition)) != (b_BBasT)0u;
                       vars[i].physValue.db = (double)(tempBit);
                   }
                   if(vars[i].bitBase==UINT16){
                       uint16 tempBit = *((uint16*)(vars[i].address));
                       tempBit = (tempBit & ((w_BBasT)1u << vars[i].bitPosition)) != (w_BBasT)0u;
                       vars[i].physValue.db = (double)(tempBit);
                   }
                   if(vars[i].bitBase==UINT32){
                       uint32 tempBit = *((uint32*)(vars[i].address));
                       tempBit = (tempBit & ((l_BBasT)1u << vars[i].bitPosition)) != (l_BBasT)0u;
                       vars[i].physValue.db = (double)(tempBit);
                   }
                   if(vars[i].bitBase==SINT8){
                       sint8 tempBit = *((sint8*)(vars[i].address));
                       tempBit = (tempBit & ((b_BBasT)1u << vars[i].bitPosition)) != (b_BBasT)0u;
                       vars[i].physValue.db = (double)(tempBit);
                   }
                   if(vars[i].bitBase==SINT16){
                       sint16 tempBit = *((sint16*)(vars[i].address));
                       tempBit = (tempBit & ((w_BBasT)1u << vars[i].bitPosition)) != (w_BBasT)0u;
                       vars[i].physValue.db = (double)(tempBit);
                   }
                   if(vars[i].bitBase==SINT32){
                       sint32 tempBit = *((sint32*)(vars[i].address));
                       tempBit = (tempBit & ((l_BBasT)1u << vars[i].bitPosition)) != (l_BBasT)0u;
                       vars[i].physValue.db = (double)(tempBit);
                   }
                   }

    }
    else
    {

        if(vars[i].compuMethod == TAB_VERB || vars[i].compuMethod == UNKNOWN_COMPU)
        {
            if(vars[i].datType==UINT8)
                vars[i].physValue.db = (double)(*((uint8*)(vars[i].address)));
            if(vars[i].datType==UINT16)
                vars[i].physValue.db = (double)(*((uint16*)(vars[i].address)));
            if(vars[i].datType==UINT32)
                vars[i].physValue.db = (double)(*((uint32*)(vars[i].address)));
            if(vars[i].datType==SINT8)
                vars[i].physValue.db = (double)(*((sint8*)(vars[i].address)));
            if(vars[i].datType==SINT16)
                vars[i].physValue.db = (double)(*((sint16*)(vars[i].address)));
            if(vars[i].datType==SINT32)
                vars[i].physValue.db = (double)(*((sint32*)(vars[i].address)));
        }
        else
        {
            if(vars[i].datType==UINT8)
                vars[i].physValue.db = internalToPhysical((double)*((uint8*)(vars[i].address)),
                    vars[i].ratCoeff[0],
                    vars[i].ratCoeff[1],
                    vars[i].ratCoeff[2],
                    vars[i].ratCoeff[3],
                    vars[i].ratCoeff[4],
                    vars[i].ratCoeff[5]);
            if(vars[i].datType==UINT16)
                vars[i].physValue.db = internalToPhysical((double)*((uint16*)(vars[i].address)),
                    vars[i].ratCoeff[0],
                    vars[i].ratCoeff[1],
                    vars[i].ratCoeff[2],
                    vars[i].ratCoeff[3],
                    vars[i].ratCoeff[4],
                    vars[i].ratCoeff[5]);
            if(vars[i].datType==UINT32)
                vars[i].physValue.db = internalToPhysical((double)*((uint32*)(vars[i].address)),
                    vars[i].ratCoeff[0],
                    vars[i].ratCoeff[1],
                    vars[i].ratCoeff[2],
                    vars[i].ratCoeff[3],
                    vars[i].ratCoeff[4],
                    vars[i].ratCoeff[5]);
            if(vars[i].datType==SINT8)
                vars[i].physValue.db = internalToPhysical((double)*((sint8*)(vars[i].address)),
                    vars[i].ratCoeff[0],
                    vars[i].ratCoeff[1],
                    vars[i].ratCoeff[2],
                    vars[i].ratCoeff[3],
                    vars[i].ratCoeff[4],
                    vars[i].ratCoeff[5]);
            if(vars[i].datType==SINT16)
                vars[i].physValue.db = internalToPhysical((double)*((sint16*)(vars[i].address)),
                    vars[i].ratCoeff[0],
                    vars[i].ratCoeff[1],
                    vars[i].ratCoeff[2],
                    vars[i].ratCoeff[3],
                    vars[i].ratCoeff[4],
                    vars[i].ratCoeff[5]);
            if(vars[i].datType==SINT32)
                vars[i].physValue.db = internalToPhysical((double)*((sint32*)(vars[i].address)),
                    vars[i].ratCoeff[0],
                    vars[i].ratCoeff[1],
                    vars[i].ratCoeff[2],
                    vars[i].ratCoeff[3],
                    vars[i].ratCoeff[4],
                    vars[i].ratCoeff[5]);
           if(vars[i].datType==FLOAT32_TPT)
                //vars[i].physValue.db = (double)(*((real32*)(vars[i].address)));
                vars[i].physValue.db = f2p((double)*((real32*)(vars[i].address)),
                    vars[i].ratCoeff[0],
                    vars[i].ratCoeff[1],
                    vars[i].ratCoeff[2],
                    vars[i].ratCoeff[3],
                    vars[i].ratCoeff[4],
                    vars[i].ratCoeff[5],0);

        }
    }
    return vars[i].physValue.db;



}

/* Function: getVariableValuePhysArray =============================================
 * Abstract:
 * This function returns the physical value of the array variables
 * @param: i The index of variable whose physical value is required
 * @param: size The size of the array variable whose physical value is required 
 * @return: The physical value in double
 * ============================================================================
 */
double getVariableValuePhysArray(long i,uint16 index )
{
    double phyValueArray;

    if(vars[i].datType==UINT8 || vars[i].datType==BIT)
    {
          phyValueArray = internalToPhysical((double)*((uint8*)vars[i].address+index),
                 vars[i].ratCoeff[0],
                 vars[i].ratCoeff[1],
                 vars[i].ratCoeff[2],
                 vars[i].ratCoeff[3],
                 vars[i].ratCoeff[4],
                 vars[i].ratCoeff[5]);
    }

    if(vars[i].datType==UINT16)
    {
          phyValueArray = internalToPhysical((double)*((uint16*)vars[i].address+index),
                 vars[i].ratCoeff[0],
                 vars[i].ratCoeff[1],
                 vars[i].ratCoeff[2],
                 vars[i].ratCoeff[3],
                 vars[i].ratCoeff[4],
                 vars[i].ratCoeff[5]);
    }

    if(vars[i].datType==UINT32)
    {
          phyValueArray = internalToPhysical((double)*((uint32*)vars[i].address+index),
                 vars[i].ratCoeff[0],
                 vars[i].ratCoeff[1],
                 vars[i].ratCoeff[2],
                 vars[i].ratCoeff[3],
                 vars[i].ratCoeff[4],
                 vars[i].ratCoeff[5]);
    }

    if(vars[i].datType==SINT8)
    {
          phyValueArray = internalToPhysical((double)*((sint8*)vars[i].address+index),
                 vars[i].ratCoeff[0],
                 vars[i].ratCoeff[1],
                 vars[i].ratCoeff[2],
                 vars[i].ratCoeff[3],
                 vars[i].ratCoeff[4],
                 vars[i].ratCoeff[5]);
    }

    if(vars[i].datType==SINT16)
    {
          phyValueArray = internalToPhysical((double)*((sint16*)vars[i].address+index),
                 vars[i].ratCoeff[0],
                 vars[i].ratCoeff[1],
                 vars[i].ratCoeff[2],
                 vars[i].ratCoeff[3],
                 vars[i].ratCoeff[4],
                 vars[i].ratCoeff[5]);
    }

    if(vars[i].datType==SINT32)
    {
         phyValueArray = internalToPhysical((double)*((sint32*)vars[i].address+index),
                 vars[i].ratCoeff[0],
                 vars[i].ratCoeff[1],
                 vars[i].ratCoeff[2],
                 vars[i].ratCoeff[3],
                 vars[i].ratCoeff[4],
                 vars[i].ratCoeff[5]);
    }

    if(vars[i].datType==FLOAT32_TPT)
    {
         phyValueArray = f2p((double)*((real32*)vars[i].address+index),
                 vars[i].ratCoeff[0],
                 vars[i].ratCoeff[1],
                 vars[i].ratCoeff[2],
                 vars[i].ratCoeff[3],
                 vars[i].ratCoeff[4],
                 vars[i].ratCoeff[5],0);
    }
    return phyValueArray;
}


/* Function: setVariableValuePhys =============================================
 * Abstract:
 * This function sets the physical value for a variable in variable list
 * @param: The index of variable, the value which needed to be set
 * @return:
 * ============================================================================
 */
long setVariableValuePhys(long i, double value)
{
    long long temp;
    float tempfloat;
    int(*func)(BitAccess,int)=NULL;

    if(i<0 || i>=NUMBER_OF_VARS)
        return 0;

    if(vars[i].datType == BIT || vars[i].datType == PACKED_BIT)
    {
    	if(!isInterfaceOpt){
        	func = vars[i].address;
        	func(BITWRITE, (int)value);
        }
        else{

        // to check if base type of paked bit variable.. hint bit - uint8
        if(vars[i].bitBase==UINT8 || vars[i].bitBase == UNKNOWN_TYPE){
            uint8 tempBit = *((uint8*)(vars[i].address));
            tempBit = (value ==1)?(tempBit |= (b_BBasT)1u << vars[i].bitPosition) : (tempBit &= (b_BBasT)(((b_BBasT) b_MASK) - ((b_BBasT)1u << vars[i].bitPosition)));
            *((uint8*)(vars[i].address))  = (uint8)tempBit;
        }
        if(vars[i].bitBase==UINT16){
            uint16 tempBit = *((uint16*)(vars[i].address));
            tempBit = (value ==1)?(tempBit |= (w_BBasT)1u << vars[i].bitPosition) : (tempBit &= (w_BBasT)(((w_BBasT) w_MASK) - ((w_BBasT)1u << vars[i].bitPosition)));
            *((uint16*)(vars[i].address)) = (uint16)tempBit;
        }
        if(vars[i].bitBase==UINT32){
            uint32 tempBit = *((uint32*)(vars[i].address));
            tempBit = (value ==1)?(tempBit |= (l_BBasT)1u << vars[i].bitPosition) : (tempBit &= (l_BBasT)(((l_BBasT) l_MASK) - ((l_BBasT)1u << vars[i].bitPosition)));
            *((uint32*)(vars[i].address)) = (uint32)tempBit;
        }
        if(vars[i].bitBase==SINT8){
            uint8 tempBit = *((uint8*)(vars[i].address));
            tempBit = (value ==1)?(tempBit |= (b_BBasT)1u << vars[i].bitPosition) : (tempBit &= (b_BBasT)(((b_BBasT) b_MASK) - ((b_BBasT)1u << vars[i].bitPosition)));
            *((sint8*)(vars[i].address))  = (sint8)tempBit;
        }
        if(vars[i].bitBase==SINT16){
            uint8 tempBit = *((uint8*)(vars[i].address));
            tempBit = (value ==1)?(tempBit |= (w_BBasT)1u << vars[i].bitPosition) : (tempBit &= (w_BBasT)(((w_BBasT) w_MASK) - ((w_BBasT)1u << vars[i].bitPosition)));
            *((sint16*)(vars[i].address)) = (sint16)tempBit;
        }
        if(vars[i].bitBase==SINT32){
            uint8 tempBit = *((uint8*)(vars[i].address));
            tempBit = (value ==1)?(tempBit |= (l_BBasT)1u << vars[i].bitPosition) : (tempBit &= (l_BBasT)(((l_BBasT) l_MASK) - ((l_BBasT)1u << vars[i].bitPosition)));
            *((sint32*)(vars[i].address)) = (sint32)tempBit;
        }

	}

    }
    else
    {
        vars[i].physValue.db = value;
        if(vars[i].compuMethod == TAB_VERB || vars[i].compuMethod == UNKNOWN_COMPU)
        {
            temp = value;
        }
        else
        {
          if(vars[i].datType==FLOAT32_TPT)
           {
            tempfloat = p2f(value,vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],0);
           }
          else
           {
           // PTEST-795 - Start
            if(vars[i].datType==UINT8)
             temp = p2i(value,vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],1);
            if(vars[i].datType==SINT8)
             temp = p2i(value,vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],2);
            if(vars[i].datType==UINT16)
             temp = p2i(value,vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],3);
            if(vars[i].datType==SINT16)
             temp = p2i(value,vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],4);
            if(vars[i].datType==UINT32)
             temp = p2i(value,vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],5);
            if(vars[i].datType==SINT32)
             temp = p2i(value,vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],6);
           // PTEST-795 - End
           }
        }
        // PTEST-302
        if(temp != *((sint32 *)(vars[i].address))){
        // PTEST-302
        if(vars[i].datType==UINT8)
            *((uint8*)(vars[i].address))  = (uint8)temp;
        if(vars[i].datType==UINT16)
            *((uint16*)(vars[i].address)) = (uint16)temp;
        if(vars[i].datType==UINT32)
            *((uint32*)(vars[i].address)) = (uint32)temp;
        if(vars[i].datType==SINT8)
            *((sint8*)(vars[i].address))  = (sint8)temp;
        if(vars[i].datType==SINT16)
            *((sint16*)(vars[i].address)) = (sint16)temp;
        if(vars[i].datType==SINT32)
            *((sint32*)(vars[i].address)) = (sint32)temp;
        if(vars[i].datType==FLOAT32_TPT)
            *((real32*)(vars[i].address)) = (real32)tempfloat;
}
}

}

/* Function: setVariableValuePhysArray =============================================
 * Abstract:
 * This function sets the physical value for a variable in variable list
 * @param: i The index of variable whose physical value is to be set
 * ============================================================================
 */
long setVariableValuePhysArray(long i, uint16 index)
{
    long long tempArr,value;
    float tempFloatArr;

    uint16 j;
    int(*func)(BitAccess,int)=NULL;

    if(i<0 || i>=NUMBER_OF_VARS)
        return 0;

    if(vars[i].datType == FLOAT32_TPT)
    {
        tempFloatArr = p2f(ptrArray[index],vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],0);
    }  
    else 
    {
        if(vars[i].datType==UINT8 || vars[i].datType==BIT)
            tempArr = p2i(ptrArray[index],vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],1);
        if(vars[i].datType==SINT8)
            tempArr = p2i(ptrArray[index],vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],2);
        if(vars[i].datType==UINT16)
            tempArr = p2i(ptrArray[index],vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],3);
        if(vars[i].datType==SINT16)
            tempArr = p2i(ptrArray[index],vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],4);
        if(vars[i].datType==UINT32)
            tempArr = p2i(ptrArray[index],vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],5);
        if(vars[i].datType==SINT32)
            tempArr = p2i(ptrArray[index],vars[i].ratCoeff[0],vars[i].ratCoeff[1],vars[i].ratCoeff[2],vars[i].ratCoeff[3],vars[i].ratCoeff[4],vars[i].ratCoeff[5],6);
    }
    
    if(vars[i].datType==FLOAT32_TPT)
            *((real32*)vars[i].address+index) = (real32)tempFloatArr;
    else
    {
        if(vars[i].datType==UINT8 || vars[i].datType==BIT)
            *((uint8*)vars[i].address+index)  = (uint8)tempArr;
        if(vars[i].datType==UINT16)
            *((uint16*)vars[i].address+index) = (uint16)tempArr;
        if(vars[i].datType==UINT32)
            *((uint32*)vars[i].address+index) = (uint32)tempArr;
        if(vars[i].datType==SINT8)
            *((sint8*)vars[i].address+index)  = (sint8)tempArr;
        if(vars[i].datType==SINT16)
            *((sint16*)vars[i].address+index) = (sint16)tempArr;
        if(vars[i].datType==SINT32)
            *((sint32*)vars[i].address+index) = (sint32)tempArr;
    }  
}

/* Function: setVariableValueHex =============================================
 * Abstract:
 * This function resets the internal value for a variable in variable list
 * @param: The index of variable, the value which needed to be set
 * @return:
 * ============================================================================
 */
long setVariableValueHex(long i, double value)
{
    long long temp;
    float tempfloat;
    int(*func)(BitAccess,int)=NULL;
    if(i<0 || i>=NUMBER_OF_VARS)
        return 0;
    if(vars[i].datType == BIT || vars[i].datType == PACKED_BIT)
    {
        func = vars[i].address;
        func(BITWRITE, (int)value);
    }
    else
    {
        vars[i].intValue.db = value;
        if(vars[i].compuMethod == TAB_VERB || vars[i].compuMethod == UNKNOWN_COMPU)
        {
            temp = value;
        }
        else
        {
            if(vars[i].datType==FLOAT32_TPT)
            {
                tempfloat = value;
            }
            else
            {
                temp = value;
            }
        }
        // PTEST-302
        if(temp != *((sint32 *)(vars[i].address))){
            // PTEST-302
            if(vars[i].datType==UINT8)
                *((uint8*)(vars[i].address))  = (uint8)temp;
            if(vars[i].datType==UINT16)
                *((uint16*)(vars[i].address)) = (uint16)temp;
            if(vars[i].datType==UINT32)
                *((uint32*)(vars[i].address)) = (uint32)temp;
            if(vars[i].datType==SINT8)
                *((sint8*)(vars[i].address))  = (sint8)temp;
            if(vars[i].datType==SINT16)
                *((sint16*)(vars[i].address)) = (sint16)temp;
            if(vars[i].datType==SINT32)
                *((sint32*)(vars[i].address)) = (sint32)temp;
            if(vars[i].datType==FLOAT32_TPT)
                *((real32*)(vars[i].address)) = (real32)tempfloat;
        }
    }
}
/* Function: setVariableValueHexArray =============================================
 * Abstract:
 * This function sets the internal value for a variable in variable list
 * @param: i The index of variable whose physical value is to be set
 * ============================================================================
 */
long setVariableValueHexArray(long i, uint16 index)
{
    long long tempArr,value;
    float tempFloatArr;
    uint16 j;
    int(*func)(BitAccess,int)=NULL;
    if(i<0 || i>=NUMBER_OF_VARS)
        return 0;
    if(vars[i].datType == FLOAT32_TPT)
    {
        tempFloatArr = ptrArray[index];
    }
    else
    {
        tempArr = ptrArray[index];
    }
    if(vars[i].datType==FLOAT32_TPT)
        *((real32*)vars[i].address+index) = (real32)tempFloatArr;
    else
    {
        if(vars[i].datType==UINT8)
            *((uint8*)vars[i].address+index)  = (uint8)tempArr;
        if(vars[i].datType==UINT16)
            *((uint16*)vars[i].address+index) = (uint16)tempArr;
        if(vars[i].datType==UINT32)
            *((uint32*)vars[i].address+index) = (uint32)tempArr;
        if(vars[i].datType==SINT8)
            *((sint8*)vars[i].address+index)  = (sint8)tempArr;
        if(vars[i].datType==SINT16)
            *((sint16*)vars[i].address+index) = (sint16)tempArr;
        if(vars[i].datType==SINT32)
            *((sint32*)vars[i].address+index) = (sint32)tempArr;
    }
}




/* Function: getDynMemInRuntime=====================================================
 * Abstract:
 * This is a dummy function that gets the pointer and sends back the incremented pointer.
 * @param int i : nth var properties dataType : Data Type of Given Var
 * ====================================================================
*/
void getDynMemInRuntime(int i){

    int nthIndex = vars[i].nthIndex;
    int incByte = 1;

    char *labelName =  vars[i].memLabelName;
    if(labelName[0] == '\0'){
        labelName = vars[i].varName;
    }
    vars[i].address = (void *)getMemLoc(labelName);
    
    if(vars[i].address == NULL || nthIndex == 0){
        return;
    }
    if(vars[i].datType==UINT8 || vars[i].datType==SINT8)
        incByte = 1;
    if(vars[i].datType==UINT16 || vars[i].datType==SINT16)
        incByte = 2;
    if(vars[i].datType==UINT32 || vars[i].datType==SINT32 || vars[i].datType==FLOAT32_TPT)
        incByte = 4;

    while(nthIndex >0){
        vars[i].address = vars[i].address + incByte;
        nthIndex--;
    }
    return;
}


/* Function: resetReIniMsgs =============================================
 * Abstract:
 * This function resets the internal value for a Re-Ini variable in variable list
 * ============================================================================
 */
void resetReIniMsgs(void)
{
    int i,index;
    double resetValue = 0;
    for(i=0;i<NUMBER_OF_VARS;i++)
    {
        if(vars[i].address == NULL){
            continue;
        }
        if( vars[i].isReIni == REINI && (vars[i].varType == OUTPUT || vars[i].varType == LOCAL)) {
            if(vars[i].sizeOfArray > 0)
            {
                ptrArray = (double *)vars[i].address;
                for(index=0;index<vars[i].sizeOfArray;index++)
                {
                    ptrArray[index] = resetValue;
                    setVariableValuePhysArray(i,index);
                }
            }
            else
            {
                setVariableValuePhys(i, resetValue);
            }
        }
    }
}
/* Function: executeFunction ==================================================
 * Abstract:
 * This function executes all the processes in the process list
 * @param: The index of process
 * @return: none
 * ============================================================================
 */
void executeFunction(long i)
{
    if(i<0 || i>=NUMBER_OF_PROCS)
        return;
    (*(procs[i].pointer))();

}

/* Function: getVariableDirection =============================================
 * Abstract:
 * This function returns the type for the given variable
 * @param: The index of variable in variable list
 * @return: The variable type
 * ============================================================================
 */
long getVariableDirection(long i)
{
    if(i<0 || i>=NUMBER_OF_VARS)
        return -2;
    return vars[i].varType;
}

/* Function: getFunctionTask ==================================================
 * Abstract:
 * This function returns the task type for the given ECU Process
 * @param: The index of the process in process list
 * @return: The Task type
 * ============================================================================
 */
long getFunctionTask(long i)
{
    if(i<0 || i>=NUMBER_OF_PROCS)
        return -2;
    return procs[i].taskType;
}


/* Function: getVariableType ==================================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */
long getVariableType(long i)
{
    if(i<0 || i>=NUMBER_OF_VARS)
        return -2;
    return vars[i].datType;
}

/* Function: getVariableCompuType =============================================
 * Abstract:
 * This function returns the compuMethod for a given variable in variable list
 * @param: The index of the variable in variable list
 * @return: The Compumethod type
 * ============================================================================
 */
long getVariableCompuType(long i)
{
    if(i<0 || i>=NUMBER_OF_VARS)
        return -2;
    return vars[i].compuMethod;
}


/* TPT part */


/* SIGNAL HANDLES */



/* Function: tpt_fusion_node_startup ==========================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */

TPT_INTERFACE_EXPORT tpt_fusion_node_result tpt_fusion_node_startup(tpt_fusion_node_handle handle, int argc, char **argv)
{
    // To initialize the DLL handle
    getDllHandle();
    
    // To get Default calparam to FUSION and Initialize CalParam
    state = 1;
    updateCalParamCharacteristics();
    setCalibrationParameter(handle);

    return TPT_FUSION_NODE_RESULT_OK;
}

/* Function: tpt_fusion_node_initfcn ==========================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */
TPT_INTERFACE_EXPORT tpt_fusion_node_result tpt_fusion_node_initfcn(tpt_fusion_node_handle handle)
{
     int i,index;
    unsigned int temp;
    const double *address;
    int arraySize;
    address = malloc(sizeof(double));
    
    install_tptFuntion(handle); 

     for(i=0;i<NUMBER_OF_VARS;i++)
    {
        if(vars[i].varType == INPUT || vars[i].varType == OUTPUT || vars[i].varType == LOCAL) {
        	if(isInterfaceOpt){
            //update address of Vars.
                getDynMemInRuntime(i);
            	if(vars[i].address == NULL){
                	continue;
           	 	}
            }
            if(((!strcmp("nmot_w",vars[i].varName)) || (!strcmp("nmot",vars[i].varName)) || (!strcmp("Epm_nEng",vars[i].varName)))) {
                // TPT Fusion function to declare scalar signals
                vars[i].tptHandle = tpt_fusion_declareScalar(handle, vars[i].varName,TPT_FUSION_VARIABLE_ROLE_SIGNAL);
                if(*tpt_fusion_readValue(handle, vars[i].tptHandle) > (double) 0) {
                    SEDGeEngineSpeed = *tpt_fusion_readValue(handle, vars[i].tptHandle);
                }
                else{
                    SEDGeEngineSpeed = 0.0;
                }
            }

            if(vars[i].sizeOfArray > 0)
            {
                // TPT Fusion function to declare array signals
                vars[i].tptHandle = tpt_fusion_declareArray(handle,vars[i].varName,TPT_FUSION_VARIABLE_ROLE_SIGNAL);

                // TPT Fusion function to read array values
                ptrArray = tpt_fusion_readValues(handle, vars[i].tptHandle, &arraySize);
                for(index=0;index<arraySize;index++)
                {
                    setVariableValuePhysArray(i,index);
                }
            }
            else
            {
                // TPT Fusion function to declare scalar signals
                vars[i].tptHandle = tpt_fusion_declareScalar(handle, vars[i].varName,TPT_FUSION_VARIABLE_ROLE_SIGNAL);
                setVariableValuePhys(i, *tpt_fusion_readValue(handle, vars[i].tptHandle));
            }
        }
    }
    
    
    #ifdef ECCO_EEP_EMU
    {
        //Check whether file for EEPROMEmulation already exists ?

        FILE *eepFilePtr = fopen("EEPEmu_values.txt","r");
        if(!eepFilePtr)
        {
            //File doesn't exists, call init function to create file and
            //initialize the eep blocks
            initEEPRomFile("EEPEmu_values.txt");
        }

        if(EEP_Load == 0)
        {
            if(EEPROM_InitMode){
                FILE *orgeepFilePtr = fopen("Org_EEPEmu_values.txt","r");
                if(orgeepFilePtr)
                {
                    loadEEPRomFile("Org_EEPEmu_values.txt");
                }else{
                    printf("The Org_EEPEmu_values.txt is not available to load.");
                }
                fclose(orgeepFilePtr);
            }
            else{
                loadEEPRomFile("EEPEmu_values.txt");
            }
            EEP_Load++;
        }
    }
#endif

   if(state == 1)
    {
        state = 2;
        temp = tpt_fusion_getStepSize(handle);

        tptStepSize = temp/1000;
        //Initialize scheduler timers
        schedInit(temp);
        printf("Step size: %i\n",temp);
    }
    setCalibrationParameter(handle);
    
   
    
  /*This function call will be implemented in future version of TPT to read calParms and channels -PTEST-2247 */
    /*if(isSycActive == 0) {
        executeReIniTask();
        executeIniTask();
        executeIniSyncTask();
        executeIniDrvTask();
        executeIniEndTask();
    }*/
    
    for(i=0;i<NUMBER_OF_VARS;i++)
    {
        if(vars[i].address == NULL){
            continue;
        }
            
        if(vars[i].varType == OUTPUT || vars[i].varType == LOCAL) {
            if(vars[i].sizeOfArray > 0) {
               // TPT Fusion function to write array values
                ptrArray = malloc(sizeof(double) * vars[i].sizeOfArray);
                for(index=0; index<vars[i].sizeOfArray; index++)
                {
                    ptrArray[index] = getVariableValuePhysArray(i,index);
                }
                tpt_fusion_writeValues(handle, vars[i].tptHandle, ptrArray, vars[i].sizeOfArray); //tpt_fusion_getVariableLength(handle, vars[i].tptHandle));
                free(ptrArray);
            } 
            else {
                // TPT Fusion function to write scalar values
                *address = (double)getVariableValuePhys(i);
                tpt_fusion_writeValue(handle, vars[i].tptHandle, address);
            }
        }
    }

       free (address);

    return TPT_FUSION_NODE_RESULT_OK;
}

/* Function: tpt_fusion_node_callfcn ==========================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */
TPT_INTERFACE_EXPORT tpt_fusion_node_result tpt_fusion_node_callfcn(tpt_fusion_node_handle handle)
{

    int i,index;
    unsigned int temp;
    const double *address;
    int arraySize;
    address = malloc(sizeof(double));
    // PTEST-206-------------------------------START
    // PTEST-405----------------------------------START
  
    // PTEST-405----------------------------------END
    // PTEST-206-------------------------------END
    tpt_fusion_param_handle changed_Parameter_Handle;// PTEST-12

    for(i=0;i<NUMBER_OF_VARS;i++)
    {
        if(vars[i].address == NULL){
            continue;
        }
        if(vars[i].varType == INPUT) {
            if(((!strcmp("nmot_w",vars[i].varName)) || (!strcmp("nmot",vars[i].varName)) || (!strcmp("Epm_nEng",vars[i].varName)))) {
                 if(*tpt_fusion_readValue(handle, vars[i].tptHandle) > (double) 0) {
                    SEDGeEngineSpeed = *tpt_fusion_readValue(handle, vars[i].tptHandle);
                }
                // PTEST - 1956
                else{
                SEDGeEngineSpeed = 0.0;
                }
                // PTEST - 1956 END
            }


        if(vars[i].sizeOfArray > 0) 
        {
            // TPT Fusion function to read array values
            ptrArray = tpt_fusion_readValues(handle, vars[i].tptHandle, &arraySize);
            for(index=0;index<arraySize;index++)
            {
                setVariableValuePhysArray(i,index);
            } 
        }
        else 
        {
                setVariableValuePhys(i, *tpt_fusion_readValue(handle, vars[i].tptHandle));
        }
    }
}
 
    /* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */
    

    //Update the application parameters that are changed online
    // PTEST-12
    changed_Parameter_Handle = tpt_fusion_getNextChangedParam(handle);
    if(changed_Parameter_Handle != 0)
        setCalibrationParameter(handle);
        if(bigSize == TRUE) {
          return TPT_FUSION_NODE_RESULT_ERROR; 
        }
    // PTEST-12 END

    schedExecute();

    for(i=0;i<NUMBER_OF_VARS;i++)
    {
        if(vars[i].address == NULL){
            continue;
        }
        if(vars[i].varType == OUTPUT || vars[i].varType == LOCAL) {
             if(vars[i].sizeOfArray > 0) {
                // TPT Fusion function to write array values
                 ptrArray = malloc(sizeof(double) * vars[i].sizeOfArray);
                 for(index=0; index<vars[i].sizeOfArray; index++)
                 {
                     ptrArray[index] = getVariableValuePhysArray(i,index);
                 }
                 tpt_fusion_writeValues(handle, vars[i].tptHandle, ptrArray, vars[i].sizeOfArray); //tpt_fusion_getVariableLength(handle, vars[i].tptHandle));
                 free(ptrArray);
                 } else {
                    // TPT Fusion function to write scalar values
                    *address = (double)getVariableValuePhys(i);
                    tpt_fusion_writeValue(handle, vars[i].tptHandle, address);
                }
            }
        }
    updateVarsPreValues();
    free (address);
    return TPT_FUSION_NODE_RESULT_OK;
}


/* Function: tpt_fusion_node_shutdown =========================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */
TPT_INTERFACE_EXPORT tpt_fusion_node_result tpt_fusion_node_shutdown(tpt_fusion_node_handle handle)
{
    // PTEST-206-----------------------START
    // PTEST-405----------------------------------START
    #ifdef ECCO_EEP_EMU
    {
      //copy mirrored RAM blocks to virtual ROM
      //saveEEPRam();
      
      if(EEP_Load == 1){ //this condition is required as the shutdowm is getting called at the begining before startup

      //save ROM to workspace variable
      saveEEPRomFile("EEPEmu_values.txt");
      }
    }
    #endif
    // PTEST-405----------------------------------END
    // PTEST-206-------------------------END
  return TPT_FUSION_NODE_RESULT_OK;
}

/* Scheduler */
/* Function: schedInit ========================================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */
// delta in ms
void schedInit(unsigned int delta)
{
    currTime = 0.0;
    tptDT = (double)delta/1000.0;

    next820usTask = 0.0;
    next1msTask = 0.0;
    next2msTask = 0.0;
    next5msTask = 0.0;
    next10msTask = 0.0;
    next20msTask = 0.0;
	next40msTask = 0.0;
    next50msTask = 0.0;
    next100msTask = 0.0;
    next200msTask = 0.0;
    next1000msTask = 0.0;
    nextSync0Task = 0.0;
    nextSync1Task = 0.0;

    // PTEST-262
    //TIMER initialization
    SrvB_SWTmrInc_10ms_Ini();
    //
}


/* Function: schedExecute =====================================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */
void schedExecute()
{
    double temp;
    int taskExecuted;
    double sync_Diff_Factor_s1;
    double sync_Diff_Factor_s0;
    

    double timeElapsed = 0.0;
    double sync_Diff_Factor = 0.5;// PTEST-173
    //Calculate first segment time
    	
	if(SEDGeCylinderCount == 0.0){
		SEDGeCylinderCount = 4.0;
    }
	
	if(SEDGeEngineType == 0.0){
		SEDGeEngineType = 3.0;
    }
	if(SEDGeSY_GRDWOF == 0.0){
		SEDGeSY_GRDWOF = 78;
    }
    
    if(SEDGeEngineSpeed > 0.0)
    {
      
         temp = (60.0 / (double)SEDGeEngineSpeed)  * 1e3;  //time in ms per rotation (one rotation is equal to 360
         //Calculation of angle speed degrees per ms
         
         //S0 interrupt will be @ 78 deg
         sync_Diff_Factor = (temp/SEDGeCylinderCount);
         
         //S1 interrupt will be [78 + (360 / No of cylinder) ] degree.
         
         //198 deg for 3 cylinder
         //168 deg for 4 cylinder
         //150 deg for 5 cylinder
         //138 deg for 6 cylinder
         
         sync_Diff_Factor_s1 = (temp/360) * ((720/SEDGeCylinderCount) - (SEDGeSY_GRDWOF + (360/SEDGeCylinderCount)));
         sync_Diff_Factor_s0 = sync_Diff_Factor_s1 + sync_Diff_Factor;
         
         if(sync_Diff_Factor_s1 <=0){
            sync_Diff_Factor_s1 = sync_Diff_Factor_s0 + sync_Diff_Factor;
           }

        if(sync_Diff_Factor_s0<=0 || sync_Diff_Factor_s1 <=0){
           sync_Diff_Factor_s0 = 0.5; //dummy value. Will never be used ideally.
           sync_Diff_Factor_s1 = 7.5;
          }

        if((nextSync0Task == (double)(0.0)) && (nextSync1Task == (double)(0.0))){
           nextSync0Task = sync_Diff_Factor_s0;
           nextSync1Task = sync_Diff_Factor_s1;
          }      
        	
    }

    if(currTime == 0.0)
    {
        if(isSycActive == 0) {
           /*PTEST - 2247 - Start*/
            executeReIniTask();
            executeIniTask();
            executeIniSyncTask();
            executeIniDrvTask();
            executeIniEndTask();
            /*PTEST - 2247 - End*/
            execute1000msTask();
            execute200msTask();
            execute100msTask();
            execute50msTask();
			execute40msTask();
            execute20msTask();
            execute820usTask();
            execute1msTask();
            execute2msTask();
            execute5msTask();
            execute10msTask();
            executeSyncroTask0();
            executeSyncroTask1();
        }
        next820usTask = 0.82;
        next1msTask = 1.0;
        next2msTask = 2.0;
        next5msTask = 5.0;
        next10msTask = 10.0;
        next20msTask = 20.0;
		next40msTask = 40.0;
        next50msTask = 50.0;
        next100msTask = 100.0;
        next200msTask = 200.0;
        next1000msTask = 1000.0;

    }
    else
    {
        do
        {
            taskExecuted = 0;
            
            // To call tasks based on conditions            
           	executeInterruptTasksBasedOnCond();            
           	
            if(nextSync1Task <= currTime  && nextSync1Task <= nextSync0Task
                                          && nextSync1Task <= next820usTask
                                          && nextSync1Task <= next1msTask
                                          && nextSync1Task <= next2msTask
                                          && nextSync1Task <= next5msTask
                                          && nextSync1Task <= next10msTask
                                          && nextSync1Task <= next20msTask
										  && nextSync1Task <= next40msTask	
                                          && nextSync1Task <= next50msTask
                                          && nextSync1Task <= next100msTask
                                          && nextSync1Task <= next200msTask
                                          && nextSync1Task <= next1000msTask)
            {
                ESC_tiSampling = ((signed long) (2 * sync_Diff_Factor * 1e3));
                ESC_tiSampling_s = ESC_tiSampling;
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				if(SEDGeEngineSpeed>0.0){
				executeSyncroTask1();
								}	
				/* END   ECCo_AutoCode_Section */

                nextSync1Task = nextSync0Task + sync_Diff_Factor;
                taskExecuted++;
                continue;    
            }
            
            if(nextSync0Task <= currTime  && nextSync1Task >= nextSync0Task
                                          && nextSync0Task <= next820usTask
                                          && nextSync0Task <= next1msTask
                                          && nextSync0Task <= next2msTask
                                          && nextSync0Task <= next5msTask
                                          && nextSync0Task <= next10msTask
                                          && nextSync0Task <= next20msTask
                                          && nextSync0Task <= next40msTask
                                          && nextSync0Task <= next50msTask
                                          && nextSync0Task <= next100msTask
                                          && nextSync0Task <= next200msTask
                                          && nextSync0Task <= next1000msTask)
            {
                ESC_tiSampling = ((signed long) (2 * sync_Diff_Factor * 1e3));
                ESC_tiSampling_s = ESC_tiSampling;
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				if(SEDGeEngineSpeed>0.0){
				executeSyncroTask0();
								}	
				/* END   ECCo_AutoCode_Section */

                nextSync0Task = nextSync1Task + sync_Diff_Factor;
                taskExecuted++;
                continue;    
            }

            if(next820usTask <= currTime && next820usTask <= next1msTask
                                         && next1msTask <= next2msTask
                                         && next1msTask <= next5msTask
                                         && next1msTask <= next10msTask
                                         && next1msTask <= next20msTask
                                         && next1msTask <= next50msTask
                                         && next1msTask <= next100msTask
                                         && next1msTask <= next200msTask
                                         && next1msTask <= next1000msTask)
            {
                execute820usTask();
                next820usTask += 0.82;
                taskExecuted++;
                continue;
            }
            
            if(next1msTask <= currTime && next1msTask <= next2msTask
                                       && next1msTask <= next5msTask
                                       && next1msTask <= next10msTask
                                       && next1msTask <= next20msTask
									   && next1msTask <= next40msTask	
                                       && next1msTask <= next50msTask
                                       && next1msTask <= next100msTask
                                       && next1msTask <= next200msTask
                                       && next1msTask <= next1000msTask)
            {
                execute1msTask();
                next1msTask += 1.0;
                taskExecuted++;
                continue;
            }
            if(next2msTask <= currTime && next2msTask <= next5msTask
                                       && next2msTask <= next10msTask
                                       && next2msTask <= next20msTask
									   && next2msTask <= next40msTask
                                       && next2msTask <= next50msTask
                                       && next2msTask <= next100msTask
                                       && next2msTask <= next200msTask
                                       && next2msTask <= next1000msTask)
            {
                execute2msTask();
                next2msTask += 2.0;
                taskExecuted++;
                continue;
            }
            if(next5msTask <= currTime && next5msTask <= next10msTask
                                       && next5msTask <= next20msTask
									   && next5msTask <= next40msTask	
                                       && next5msTask <= next50msTask
                                       && next5msTask <= next100msTask
                                       && next5msTask <= next200msTask
                                       && next5msTask <= next1000msTask)
            {
                execute5msTask();
                next5msTask += 5.0;
                taskExecuted++;
                continue;
            }
            if(next10msTask <= currTime && next10msTask <= next20msTask
                                       && next10msTask <= next40msTask
                                       && next10msTask <= next50msTask
                                       && next10msTask <= next100msTask
                                       && next10msTask <= next200msTask
                                       && next10msTask <= next1000msTask)
            {
                // PTEST-262
                SrvB_SWTmrInc_10ms_Proc();
                //
                // PTEST-494
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				execute10msTask();
				/* END   ECCo_AutoCode_Section */

                next10msTask += 10.0;
                taskExecuted++;
                continue;
            }
            if(next20msTask <= currTime && next20msTask <= next40msTask
			                            && next20msTask <= next50msTask
                                       && next20msTask <= next100msTask
                                       && next20msTask <= next200msTask
                                       && next20msTask <= next1000msTask)
            {
                // PTEST-494
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				execute20msTask();
				/* END   ECCo_AutoCode_Section */

                next20msTask += 20.0;
                taskExecuted++;
                continue;
            }
		    if(next40msTask <= currTime && next40msTask <= next50msTask
                                       && next40msTask <= next100msTask
                                       && next40msTask <= next200msTask
                                       && next40msTask <= next1000msTask)
            {
                // PTEST-494
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				execute40msTask();
				/* END   ECCo_AutoCode_Section */

                next40msTask += 40.0;
                taskExecuted++;
                continue;
            }
            if(next50msTask <= currTime && next50msTask <= next100msTask
                                       && next50msTask <= next200msTask
                                       && next50msTask <= next1000msTask)
            {
                // PTEST-494
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				execute50msTask();
				/* END   ECCo_AutoCode_Section */

                next50msTask += 50.0;
                taskExecuted++;
                continue;
            }
            if(next100msTask <= currTime && next100msTask <= next200msTask
                                       && next100msTask <= next1000msTask)
            {
                // PTEST-494
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				execute100msTask();
				/* END   ECCo_AutoCode_Section */

                next100msTask += 100.0;
                taskExecuted++;
                continue;
            }
            if(next200msTask <= currTime && next200msTask <= next1000msTask)
            {
                // PTEST-494
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				execute200msTask();
				/* END   ECCo_AutoCode_Section */

                next200msTask += 200.0;
                taskExecuted++;
                continue;
            }
            if(next1000msTask <= currTime)
            {
                // PTEST-494
                /* BEGIN ECCo_AutoCode_Section ********************************************* */
				execute1000msTask();
				/* END   ECCo_AutoCode_Section */

                next1000msTask += 1000.0;
                taskExecuted++;
                continue;
            }

        }
        while(taskExecuted!=0);

    }
    currTime+=tptDT;
}

    /* BEGIN ECCo_AutoCode_Section ********************************************* */
void initCalPrm__CMM_UDS_CHRG_PLUG_TYPEPR_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: CMM_UDS_CHRG_PLUG_TYPEPR_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_CMM_UDS_CHRG_PLUG_TYPEPR_C;
		paramHandle = tpt_fusion_declareScalar(handle, "CMM_UDS_CHRG_PLUG_TYPEPR_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

void initCalPrm__CMM_UDS_LV_BATT_TECHNOPR_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: CMM_UDS_LV_BATT_TECHNOPR_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_CMM_UDS_LV_BATT_TECHNOPR_C;
		paramHandle = tpt_fusion_declareScalar(handle, "CMM_UDS_LV_BATT_TECHNOPR_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

void initCalPrm__Ctrl_bAcv_ctClcProc078_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: Ctrl_bAcv_ctClcProc078_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_Ctrl_bAcv_ctClcProc078_C;
		paramHandle = tpt_fusion_declareScalar(handle, "Ctrl_bAcv_ctClcProc078_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

void initCalPrm__Ctrl_bAcv_ctClcProc4FE_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: Ctrl_bAcv_ctClcProc4FE_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_Ctrl_bAcv_ctClcProc4FE_C;
		paramHandle = tpt_fusion_declareScalar(handle, "Ctrl_bAcv_ctClcProc4FE_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

void initCalPrm__Ctrl_bAcv_noCks078_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: Ctrl_bAcv_noCks078_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_Ctrl_bAcv_noCks078_C;
		paramHandle = tpt_fusion_declareScalar(handle, "Ctrl_bAcv_noCks078_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

void initCalPrm__Ctrl_bAcv_noCks4FE_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: Ctrl_bAcv_noCks4FE_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_Ctrl_bAcv_noCks4FE_C;
		paramHandle = tpt_fusion_declareScalar(handle, "Ctrl_bAcv_noCks4FE_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

void initCalPrm__EONV_bHldPwrl_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: EONV_bHldPwrl_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_EONV_bHldPwrl_C;
		paramHandle = tpt_fusion_declareScalar(handle, "EONV_bHldPwrl_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

void initCalPrm__TqRes_tqBlowByRes_C(tpt_fusion_node_handle handle){
	static tpt_fusion_variable_handle paramHandle = 0;
	static tpt_fusion_variable_handle xAxisHandle = 0;
	static tpt_fusion_variable_handle yAxisHandle = 0;
	static tpt_fusion_variable_handle valAxisHandle = 0;
	CalPrm *calPrm;
	/*Calibration Parameter: TqRes_tqBlowByRes_C*/
	if(state ==1){
		calPrm = (CalPrm *) calloc (1, sizeof(CalPrm));
		calPrm->c = calPrm_TqRes_tqBlowByRes_C;
		paramHandle = tpt_fusion_declareScalar(handle, "TqRes_tqBlowByRes_C", TPT_FUSION_VARIABLE_ROLE_PARAMETER);
		calPrm->handle = paramHandle;

		calPrmArray[calPrm_Counter_local++] = calPrm;
		initCharWithDef(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_INIT);
		initInpStructInTPT(calPrm, handle);
	} else if(state == 2){ 
		calPrm = calPrmArray[calPrm_Counter_local++];
		initInpStructFrmTPT(calPrm, handle);
		initCharacteristic(&calPrm->c, (void *)(calPrm->c.memloc), 0, TPT_CALL);
	}

}

/* END   ECCo_AutoCode_Section */


/* Function: setCalibrationParameter ==========================================
 * Abstract:
 * This function
 * @param
 * @return
 * ============================================================================
 */

setCalibrationParameter(tpt_fusion_node_handle handle)
{
    calPrm_Counter_local = 1;

    /* BEGIN ECCo_AutoCode_Section ********************************************* */
	initCalPrm__CMM_UDS_CHRG_PLUG_TYPEPR_C(handle);
	initCalPrm__CMM_UDS_LV_BATT_TECHNOPR_C(handle);
	initCalPrm__Ctrl_bAcv_ctClcProc078_C(handle);
	initCalPrm__Ctrl_bAcv_ctClcProc4FE_C(handle);
	initCalPrm__Ctrl_bAcv_noCks078_C(handle);
	initCalPrm__Ctrl_bAcv_noCks4FE_C(handle);
	initCalPrm__EONV_bHldPwrl_C(handle);
	initCalPrm__TqRes_tqBlowByRes_C(handle);
/* END   ECCo_AutoCode_Section */

}

// To store Axis Value - Typesst ref Variable
/* BEGIN ECCo_AutoCode_Section ********************************************* */
/* END   ECCo_AutoCode_Section */

    
/* ========================== VERSION INFO ================================ */
TPT_INTERFACE_EXPORT const char *versioninfo_get_package() 
{
    /* BEGIN ECCo_AutoCode_Section ********************************************* */
	return "MG1CS051_H_RQONE034318060/SEDGe 2019.1.1";
/* END   ECCo_AutoCode_Section */

}

        
TPT_INTERFACE_EXPORT const char *versioninfo_get_version() 
{ 
    /* BEGIN ECCo_AutoCode_Section ********************************************* */
	char *ver = "VehC_SwSPSA/38.53.0;1";
/* END   ECCo_AutoCode_Section */

    return ver;
}


