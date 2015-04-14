/*++

Copyright (c) 1992-2002  Microsoft Corporation

Module Name:

    extsfns.h

Abstract:

    This header file must be included after "windows.h", "dbgeng.h", and
    "wdbgexts.h".

    This file contains headers for various known extension functions defined
    in different extension dlls. To use these functions, the appropriate
    extension dll must be loaded in the debugger. IDebugSymbols->GetExtension
    (declared in dbgeng.h) method could be used to retrieve these functions.

    Please see the Debugger documentation for specific information about how
    to write your own debugger extension DLL.

Environment:

    Win32 only.

Revision History:

--*/



#ifndef _EXTFNS_H
#define _EXTFNS_H

#define _EXTSAPI_VER_ 2

#ifndef _KDEXTSFN_H
#define _KDEXTSFN_H

/*
 *  Extension functions defined in kdexts.dll
 */

//
// device.c
//
typedef struct _DEBUG_DEVICE_OBJECT_INFO {
    ULONG      SizeOfStruct; // must be == sizeof(DEBUG_DEVICE_OBJECT_INFO)
    ULONG64    DevObjAddress;
    ULONG      ReferenceCount;
    BOOL       QBusy;
    ULONG64    DriverObject;
    ULONG64    CurrentIrp;
    ULONG64    DevExtension;
    ULONG64    DevObjExtension;
} DEBUG_DEVICE_OBJECT_INFO, *PDEBUG_DEVICE_OBJECT_INFO;


// GetDevObjInfo
typedef HRESULT
(WINAPI *PGET_DEVICE_OBJECT_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 DeviceObject,
    OUT PDEBUG_DEVICE_OBJECT_INFO pDevObjInfo);


//
// driver.c
//
typedef struct _DEBUG_DRIVER_OBJECT_INFO {
    ULONG     SizeOfStruct; // must be == sizef(DEBUG_DRIVER_OBJECT_INFO)
    ULONG     DriverSize;
    ULONG64   DriverObjAddress;
    ULONG64   DriverStart;
    ULONG64   DriverExtension;
    ULONG64   DeviceObject;
    struct {
        USHORT Length;
        USHORT MaximumLength;
        ULONG64 Buffer;
    } DriverName;
} DEBUG_DRIVER_OBJECT_INFO, *PDEBUG_DRIVER_OBJECT_INFO;

// GetDrvObjInfo
typedef HRESULT
(WINAPI *PGET_DRIVER_OBJECT_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 DriverObject,
    OUT PDEBUG_DRIVER_OBJECT_INFO pDrvObjInfo);

//
// irp.c
//
typedef struct _DEBUG_IRP_STACK_INFO {
    UCHAR     Major;
    UCHAR     Minor;
    ULONG64   DeviceObject;
    ULONG64   FileObject;
    ULONG64   CompletionRoutine;
    ULONG64   StackAddress;
} DEBUG_IRP_STACK_INFO, *PDEBUG_IRP_STACK_INFO;

typedef struct _DEBUG_IRP_INFO {
    ULONG     SizeOfStruct;  // Must be == sizeof(DEBUG_IRP_INFO)
    ULONG64   IrpAddress;
    ULONG     StackCount;
    ULONG     CurrentLocation;
    ULONG64   MdlAddress;
    ULONG64   Thread;
    ULONG64   CancelRoutine;
    DEBUG_IRP_STACK_INFO CurrentStack;
} DEBUG_IRP_INFO, *PDEBUG_IRP_INFO;

// GetIrpInfo
typedef HRESULT
(WINAPI * PGET_IRP_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 Irp,
    OUT PDEBUG_IRP_INFO IrpInfo
    );



//
// pool.c
//
typedef struct _DEBUG_POOL_DATA {
    ULONG   SizeofStruct;
    ULONG64 PoolBlock;
    ULONG64 Pool;
    ULONG   PreviousSize;
    ULONG   Size;
    ULONG   PoolTag;
    ULONG64 ProcessBilled;
    union {
        struct {
            ULONG   Free:1;
            ULONG   LargePool:1;
            ULONG   SpecialPool:1;
            ULONG   Pageable:1;
            ULONG   Protected:1;
            ULONG   Allocated:1;
            ULONG   Reserved:26;
        };
        ULONG AsUlong;
    };
    ULONG64 Reserved2[4];
    CHAR    PoolTagDescription[64];
} DEBUG_POOL_DATA, *PDEBUG_POOL_DATA;


// GetPoolData
typedef HRESULT
(WINAPI *PGET_POOL_DATA)(
    PDEBUG_CLIENT Client,
    ULONG64 Pool,
    PDEBUG_POOL_DATA PoolData
    );

typedef enum _DEBUG_POOL_REGION {
    DbgPoolRegionUnknown,
    DbgPoolRegionSpecial,
    DbgPoolRegionPaged,
    DbgPoolRegionNonPaged,
    DbgPoolRegionCode,
    DbgPoolRegionNonPagedExpansion,
    DbgPoolRegionSessionPaged,
    DbgPoolRegionMax,
} DEBUG_POOL_REGION;

// GetPoolRegion
typedef HRESULT
(WINAPI  *PGET_POOL_REGION)(
     PDEBUG_CLIENT Client,
     ULONG64 Pool,
     DEBUG_POOL_REGION *PoolRegion
     );

//
// Proces.c: FindMatchingThread
//
typedef struct _KDEXT_THREAD_FIND_PARAMS {
    ULONG    SizeofStruct;
    ULONG64  StackPointer;
    ULONG    Cid;
    ULONG64  Thread;
} KDEXT_THREAD_FIND_PARAMS, *PKDEXT_THREAD_FIND_PARAMS;

typedef HRESULT
(WINAPI *PFIND_MATCHING_THREAD)(
    PDEBUG_CLIENT Client,
    PKDEXT_THREAD_FIND_PARAMS ThreadInfo
    );

//
// FindFileLockOwnerInfo
//
typedef struct _KDEXT_FILELOCK_OWNER {
    ULONG Sizeofstruct;
    ULONG64 FileObject;            // IN  File object whose owner is to be searched
    ULONG64 OwnerThread;           // OUT Thread owning file object
    ULONG64 WaitIrp;               // OUT Irp associated with file object in hte thread
    ULONG64 DeviceObject;          // OUT Device object on which IRP is blocked
    CHAR    BlockingDirver[32];    // OUT Driver for the device object
} KDEXT_FILELOCK_OWNER, *PKDEXT_FILELOCK_OWNER;

typedef HRESULT
(WINAPI *PFIND_FILELOCK_OWNERINFO)(
    PDEBUG_CLIENT Client,
    PKDEXT_FILELOCK_OWNER pFileLockOwner
    );

#endif // _KDEXTSFN_H


#ifndef _KEXTFN_H
#define _KEXTFN_H

/*
 *  Extension functions defined in kext.dll
 */

/*****************************************************************************
        PoolTag definitions
 *****************************************************************************/

typedef struct _DEBUG_POOLTAG_DESCRIPTION {
    ULONG  SizeOfStruct; // must be == sizeof(DEBUG_POOLTAG_DESCRIPTION)
    ULONG  PoolTag;
    CHAR   Description[MAX_PATH];
    CHAR   Binary[32];
    CHAR   Owner[32];
} DEBUG_POOLTAG_DESCRIPTION, *PDEBUG_POOLTAG_DESCRIPTION;

// GetPoolTagDescription
typedef HRESULT
(WINAPI *PGET_POOL_TAG_DESCRIPTION)(
    ULONG PoolTag,
    PDEBUG_POOLTAG_DESCRIPTION pDescription
    );

#endif // _KEXTFN_H

#ifndef _EXTAPIS_H
#define _EXTAPIS_H

/*
 *  Extension functions defined in ext.dll
 */

/*****************************************************************************
        Failure analysis definitions
 *****************************************************************************/

typedef enum _DEBUG_FAILURE_TYPE {
    DEBUG_FLR_UNKNOWN,
    DEBUG_FLR_KERNEL,
    DEBUG_FLR_USER_CRASH,
    DEBUG_FLR_IE_CRASH,
} DEBUG_FAILURE_TYPE;

/*
    Each analysis entry can have associated data with it.  The
    analyzer knows how to handle each of these entries.
    For example it could do a !driver on a DEBUG_FLR_DRIVER_OBJECT
    or it could do a .cxr and k on a DEBUG_FLR_CONTEXT.
*/
typedef enum _DEBUG_FLR_PARAM_TYPE {
    DEBUG_FLR_INVALID = 0,
    DEBUG_FLR_RESERVED,
    DEBUG_FLR_DRIVER_OBJECT,
    DEBUG_FLR_DEVICE_OBJECT,
    DEBUG_FLR_INVALID_PFN,
    DEBUG_FLR_WORKER_ROUTINE,
    DEBUG_FLR_WORK_ITEM,
    DEBUG_FLR_INVALID_DPC_FOUND,
    DEBUG_FLR_PROCESS_OBJECT,
    // Address for which an instruction could not be executed,
    // such as invalid instructions or attempts to execute
    // non-instruction memory.
    DEBUG_FLR_FAILED_INSTRUCTION_ADDRESS,
    DEBUG_FLR_LAST_CONTROL_TRANSFER,
    DEBUG_FLR_ACPI_EXTENSION,
    DEBUG_FLR_ACPI_OBJECT,
    DEBUG_FLR_PROCESS_NAME,
    DEBUG_FLR_READ_ADDRESS,
    DEBUG_FLR_WRITE_ADDRESS,
    DEBUG_FLR_CRITICAL_SECTION,
    DEBUG_FLR_BAD_HANDLE,
    DEBUG_FLR_INVALID_HEAP_ADDRESS,

    DEBUG_FLR_IRP_ADDRESS = 0x100,
    DEBUG_FLR_IRP_MAJOR_FN,
    DEBUG_FLR_IRP_MINOR_FN,
    DEBUG_FLR_IRP_CANCEL_ROUTINE,
    DEBUG_FLR_IOSB_ADDRESS,
    DEBUG_FLR_INVALID_USEREVENT,

    // Previous mode 0 == KernelMode , 1 == UserMode
    DEBUG_FLR_PREVIOUS_MODE,

    // Irql
    DEBUG_FLR_CURRENT_IRQL = 0x200,
    DEBUG_FLR_PREVIOUS_IRQL,
    DEBUG_FLR_REQUESTED_IRQL,

    // Exceptions
    DEBUG_FLR_ASSERT_DATA = 0x300,
    DEBUG_FLR_ASSERT_FILE,
    DEBUG_FLR_EXCEPTION_PARAMETER1,
    DEBUG_FLR_EXCEPTION_PARAMETER2,
    DEBUG_FLR_EXCEPTION_PARAMETER3,
    DEBUG_FLR_EXCEPTION_PARAMETER4,
    DEBUG_FLR_EXCEPTION_RECORD,
    DEBUG_FLR_IO_ERROR_CODE,
    DEBUG_FLR_EXCEPTION_STR,

    // Pool
    DEBUG_FLR_POOL_ADDRESS = 0x400,
    DEBUG_FLR_SPECIAL_POOL_CORRUPTION_TYPE,
    DEBUG_FLR_CORRUPTING_POOL_ADDRESS,
    DEBUG_FLR_CORRUPTING_POOL_TAG,
    DEBUG_FLR_FREED_POOL_TAG,


    // Filesystem
    DEBUG_FLR_FILE_ID = 0x500,
    DEBUG_FLR_FILE_LINE,

    // bugcheck data
    DEBUG_FLR_BUGCHECK_STR = 0x600,
    DEBUG_FLR_BUGCHECK_SPECIFIER,

    // Constant values / exception code / bugcheck subtypes etc
    DEBUG_FLR_DRIVER_VERIFIER_IO_VIOLATION_TYPE = 0x1000,
    DEBUG_FLR_EXCEPTION_CODE,
    DEBUG_FLR_SPARE2,
    DEBUG_FLR_IOCONTROL_CODE,
    DEBUG_FLR_MM_INTERNAL_CODE,
    DEBUG_FLR_DRVPOWERSTATE_SUBCODE,
    DEBUG_FLR_STATUS_CODE,

    // Notification IDs, values under it doesn't have significance
    DEBUG_FLR_CORRUPT_MODULE_LIST = 0x2000,
    DEBUG_FLR_BAD_STACK,
    DEBUG_FLR_ZEROED_STACK,
    DEBUG_FLR_WRONG_SYMBOLS,
    DEBUG_FLR_FOLLOWUP_DRIVER_ONLY,   //bugcheckEA indicates a general driver failure
    DEBUG_FLR_UNUSED001,             //bucket include timestamp, so each drive is tracked
    DEBUG_FLR_CPU_OVERCLOCKED,
    DEBUG_FLR_POSSIBLE_INVALID_CONTROL_TRANSFER,
    DEBUG_FLR_POISONED_TB,
    DEBUG_FLR_UNKNOWN_MODULE,
    DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION,
    DEBUG_FLR_SINGLE_BIT_ERROR,
    DEBUG_FLR_TWO_BIT_ERROR,
    DEBUG_FLR_INVALID_KERNEL_CONTEXT,
    DEBUG_FLR_DISK_HARDWARE_ERROR,
    DEBUG_FLR_SHOW_ERRORLOG,
    DEBUG_FLR_MANUAL_BREAKIN,

    // Known analyzed failure cause or problem that bucketing could be
    // applied against.
    DEBUG_FLR_POOL_CORRUPTOR = 0x3000,
    DEBUG_FLR_MEMORY_CORRUPTOR,
    DEBUG_FLR_UNALIGNED_STACK_POINTER,
    DEBUG_FLR_OLD_OS_VERSION,
    DEBUG_FLR_BUGCHECKING_DRIVER,
    DEBUG_FLR_SOLUTION_ID,
    DEBUG_FLR_DEFAULT_SOLUTION_ID,
    DEBUG_FLR_SOLUTION_TYPE,

    // Internal data, retated to the OCA database
    DEBUG_FLR_INTERNAL_RAID_BUG = 0x4000,
    DEBUG_FLR_INTERNAL_BUCKET_URL,
    DEBUG_FLR_INTERNAL_SOLUTION_TEXT,
    DEBUG_FLR_INTERNAL_BUCKET_HITCOUNT,

    // Strings.
    DEBUG_FLR_BUCKET_ID = 0x10000,
    DEBUG_FLR_IMAGE_NAME,
    DEBUG_FLR_SYMBOL_NAME,
    DEBUG_FLR_FOLLOWUP_NAME,
    DEBUG_FLR_STACK_COMMAND,
    DEBUG_FLR_STACK_TEXT,
    DEBUG_FLR_MODULE_NAME,
    DEBUG_FLR_FIXED_IN_OSVERSION,
    DEBUG_FLR_DEFAULT_BUCKET_ID,
    DEBUG_FLR_MODULE_BUCKET_ID,         // Part of Bucket id specific to the culprit module
    DEBUG_FLR_MODULE_DEBUGTEXT,

    // User-mode specific stuff
    DEBUG_FLR_USERMODE_DATA = 0x100000,

    // Culprit module
    DEBUG_FLR_FAULTING_IP = 0x80000000,     // Instruction where failure occurred
    DEBUG_FLR_FAULTING_MODULE,
    DEBUG_FLR_IMAGE_TIMESTAMP,
    DEBUG_FLR_FOLLOWUP_IP,

    // To get faulting stack
    DEBUG_FLR_FAULTING_THREAD = 0xc0000000,
    DEBUG_FLR_CONTEXT,
    DEBUG_FLR_TRAP_FRAME,
    DEBUG_FLR_TSS,
    DEBUG_FLR_BLOCKING_THREAD, // Thread which is blocking others to execute by holding locks/critsec

    DEBUG_FLR_MASK_ALL = 0xFFFFFFFF

} DEBUG_FLR_PARAM_TYPE;

//----------------------------------------------------------------------------
//
// A failure analysis is a dynamic buffer of tagged blobs.  Values
// are accessed through the Get/Set methods.
//
// Entries are always fully aligned.
//
// Set methods throw E_OUTOFMEMORY exceptions when the data
// buffer cannot be extended.
//
//----------------------------------------------------------------------------

typedef DEBUG_FLR_PARAM_TYPE FA_TAG;

typedef struct _FA_ENTRY
{
    FA_TAG Tag;
    USHORT FullSize;
    USHORT DataSize;
} FA_ENTRY, *PFA_ENTRY;

#define FA_ENTRY_DATA(Type, Entry) ((Type)((Entry) + 1))

/* ed0de363-451f-4943-820c-62dccdfa7e6d */
DEFINE_GUID(IID_IDebugFailureAnalysis, 0xed0de363, 0x451f, 0x4943,
            0x82, 0x0c, 0x62, 0xdc, 0xcd, 0xfa, 0x7e, 0x6d);

typedef interface DECLSPEC_UUID("ed0de363-451f-4943-820c-62dccdfa7e6d")
    IDebugFailureAnalysis* PDEBUG_FAILURE_ANALYSIS;

#undef INTERFACE
#define INTERFACE IDebugFailureAnalysis
DECLARE_INTERFACE_(IDebugFailureAnalysis, IUnknown)
{
    // IUnknown.
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        ) PURE;
    STDMETHOD_(ULONG, AddRef)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, Release)(
        THIS
        ) PURE;

    // IDebugFailureAnalysis.
    STDMETHOD_(ULONG, GetFailureClass)(
        THIS
        ) PURE;
    STDMETHOD_(DEBUG_FAILURE_TYPE, GetFailureType)(
        THIS
        ) PURE;
    STDMETHOD_(ULONG, GetFailureCode)(
        THIS
        ) PURE;
    STDMETHOD_(PFA_ENTRY, Get)(
        THIS_
        FA_TAG Tag
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetNext)(
        THIS_
        PFA_ENTRY Entry,
        FA_TAG Tag,
        FA_TAG TagMask
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetString)(
        THIS_
        FA_TAG Tag,
        PSTR Str,
        ULONG MaxSize
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetBuffer)(
        THIS_
        FA_TAG Tag,
        PVOID Buf,
        ULONG Size
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetUlong)(
        THIS_
        FA_TAG Tag,
        PULONG Value
        ) PURE;
    STDMETHOD_(PFA_ENTRY, GetUlong64)(
        THIS_
        FA_TAG Tag,
        PULONG64 Value
        ) PURE;
    STDMETHOD_(PFA_ENTRY, NextEntry)(
        THIS_
        PFA_ENTRY Entry
        ) PURE;
};

#define FAILURE_ANALYSIS_NO_DB_LOOKUP 0x0001
#define FAILURE_ANALYSIS_VERBOSE      0x0002

typedef HRESULT
(WINAPI* EXT_GET_FAILURE_ANALYSIS)(
    IN PDEBUG_CLIENT4 Client,
    IN ULONG Flags,
    OUT PDEBUG_FAILURE_ANALYSIS* Analysis
    );

/*****************************************************************************
   Target info
 *****************************************************************************/
typedef enum _TARGET_MODE {
    NoTarget = DEBUG_CLASS_UNINITIALIZED,
    KernelModeTarget = DEBUG_CLASS_KERNEL,
    UserModeTarget = DEBUG_CLASS_USER_WINDOWS,
    NumModes,
} TARGET_MODE;

typedef enum _OS_TYPE {
    WIN_95,
    WIN_98,
    WIN_ME,
    WIN_NT4,
    WIN_NT5,
    WIN_NT5_1,
    NUM_WIN,
} OS_TYPE;


//
// Info about OS installed
//
typedef struct _OS_INFO {
    OS_TYPE   Type;          // OS type such as NT4, NT5 etc.
    union {
        struct {
            ULONG Major;
            ULONG Minor;
        }       Version;     // 64 bit OS version number
        ULONG64 Ver64;
    };
    ULONG ProductType; // NT, LanMan or Server
    ULONG Suite;        // OS flavour - per, SmallBuisness etc.
    struct {
        ULONG Checked:1;     // If its a checked build
        ULONG Pae:1;         // True for Pae systems
        ULONG MultiProc:1;   // True for multiproc enabled OS
        ULONG Reserved:29;
    } s;
    ULONG   SrvPackNumber;   // Service pack number of OS
    TCHAR   Language[30];    // OS language
    TCHAR   OsString[64];    // Build string
    TCHAR   ServicePackString[64];
                             // Service pack string
} OS_INFO, *POS_INFO;

typedef struct _CPU_INFO {
    ULONG Type;              // Processor type as in IMAGE_FILE_MACHINE types
    ULONG NumCPUs;           // Actual number of Processors
    ULONG CurrentProc;       // Current processor
    DEBUG_PROCESSOR_IDENTIFICATION_ALL ProcInfo[32];
} CPU_INFO, *PCPU_INFO;

typedef enum _DATA_SOURCE {
    Debugger,
    Stress,
} DATA_SOURCE;

#define MAX_STACK_IN_BYTES 4096

typedef struct _TARGET_DEBUG_INFO {
    ULONG       SizeOfStruct;
    ULONG64     Id;          // ID unique to this debug info
    DATA_SOURCE Source;      // Source where this came from
    ULONG64     EntryDate;   // Date created
    ULONG64     SysUpTime;   // System Up time
    ULONG64     AppUpTime;   // Application up time
    ULONG64     CrashTime;   // Time system / app crashed
    TARGET_MODE Mode;        // Kernel / User mode
    OS_INFO     OsInfo;      // OS details
    CPU_INFO    Cpu;         // Processor details
    TCHAR       DumpFile[MAX_PATH]; // Dump file name if its a dump
    PVOID       FailureData; // Failure data collected by debugger
    CHAR       StackTr[MAX_STACK_IN_BYTES];
                                 // Contains stacks, with frames separated by newline
} TARGET_DEBUG_INFO, *PTARGET_DEBUG_INFO;

// GetTargetInfo
typedef HRESULT
(WINAPI* EXT_TARGET_INFO)(
    PDEBUG_CLIENT4  Client,
    PTARGET_DEBUG_INFO pTargetInfo
    );


typedef struct _DEBUG_DECODE_ERROR {
    ULONG     SizeOfStruct;   // Must be == sizeof(DEBUG_DECODE_ERROR)
    ULONG     Code;           // Error code to be decoded
    BOOL      TreatAsStatus;  // True if code is to be treated as Status
    CHAR      Source[64];     // Source from where we got decoded message
    CHAR      Message[MAX_PATH]; // Message string for error code
} DEBUG_DECODE_ERROR, *PDEBUG_DECODE_ERROR;

/*
   Decodes and prints the given error code - DecodeError
*/
typedef VOID
(WINAPI *EXT_DECODE_ERROR)(
    PDEBUG_DECODE_ERROR pDecodeError
    );

//
// ext.dll: GetTriageFollowupFromSymbol
//
//       This returns owner info from a given symbol name
//
typedef struct _DEBUG_TRIAGE_FOLLOWUP_INFO {
    ULONG SizeOfStruct;      // Must be == sizeof (DEBUG_TRIAGE_FOLLOWUP_INFO)
    ULONG OwnerNameSize;     // Size of allocated buffer
    PCHAR OwnerName;         // Followup owner name returned in this
                             // Caller should initialize the name buffer
} DEBUG_TRIAGE_FOLLOWUP_INFO, *PDEBUG_TRIAGE_FOLLOWUP_INFO;

#define TRIAGE_FOLLOWUP_FAIL    0
#define TRIAGE_FOLLOWUP_IGNORE  1
#define TRIAGE_FOLLOWUP_DEFAULT 2
#define TRIAGE_FOLLOWUP_SUCCESS 3

typedef DWORD
(WINAPI *EXT_TRIAGE_FOLLOWUP)(
    IN PDEBUG_CLIENT4 Client,
    IN PSTR SymbolName,
    OUT PDEBUG_TRIAGE_FOLLOWUP_INFO OwnerInfo
    );

//
// Struct to receive data from syzdata.XML file cabbed along with the dump
//
typedef struct _EXT_CAB_XML_DATA {
    ULONG SizeOfStruct;       // Must be == sizeof(_EXT_CAB_XML_DATA)
    PCWSTR XmlObjectTag;      // Look for text under this tag
    ULONG NumSubTags;         // Number of subtags
    struct _SUBTAGS {
        PCWSTR SubTag;        // Look for text under this sub-tag of XmlObjectTag
        PCWSTR MatchPattern;  // Match only the text starting with MatchPattern
        PWSTR  ReturnText;    // Return the matched text in ReturnText, multiple
                              // matches are returned in multistring
        ULONG ReturnTextSize; // Size of ReturnText in bytes
        ULONG Reserved;
        ULONG Reserved2;
    } SubTags[1];
} EXT_CAB_XML_DATA, *PEXT_CAB_XML_DATA;

typedef HRESULT
(WINAPI *EXT_XML_DATA)(
    PDEBUG_CLIENT4 Client,
    PEXT_CAB_XML_DATA pXmpData
    );

//
// Extension function type definition for dlls which want to export analyzer
// function to be used by !analyze to gather component specific data
//
typedef HRESULT
(WINAPI *EXT_ANALYZER)(
   IN PDEBUG_CLIENT Client,
   OUT PSTR BucketSuffix,     // The additional suffix analyzer wants to be added to !analyze BUGCKET_ID to better
                              // distinguish this bucket
   IN ULONG cbBucketSuffix,   // byte count of BucketSuffix buffer supplied
   OUT PSTR DebugText,        // The debugging text (optional) which !analyze should print out to help people
                              // debugging this failure
   IN ULONG cbDebugText       // byte count of DebugText buffer supplied
   );

#endif // _EXTAPIS_H


//
// Function exported fron ntsdexts.dll
//
typedef HRESULT
(WINAPI *EXT_GET_HANDLE_TRACE)(
    PDEBUG_CLIENT Client,
    ULONG TraceType,
    ULONG StartIndex,
    PULONG64 HandleValue,
    PULONG64 StackFunctions,
    ULONG StackTraceSize
    );

#endif // _EXTFNS_H
