/*  ProcProfile 1.0 - A Command Line Process Profiling Tool For Windows
    Written in 2013 by David Catt
    Modified by Bulat Ziganshin
    Placed into public domain */

/* #define BYTECNT */
#define STDPTIME
/* #define CCTERMINATE */
/* #define POLLING */
/* #define WAITPOLLING */
#define POLLINTERVAL 10
#define CHECKINTERVAL 10
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef STDPTIME
#include <time.h>
#endif

PROCESS_INFORMATION pi;
BOOL WINAPI breakHdl(DWORD dwCtrlType) {
#ifdef CCTERMINATE
	GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pi.dwProcessId);
	/* if(!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pi.dwProcessId)) TerminateProcess(pi.hProcess, 1); */
#else
	TerminateProcess(pi.hProcess, 1);
#endif
        return TRUE;
}

int main() {
	/* Declare variables */
	SYSTEM_INFO sysInfo;
	LPTSTR cl,cm;
	STARTUPINFO si;
	PROCESS_MEMORY_COUNTERS mc;
	IO_COUNTERS ic;
#ifdef STDPTIME
	clock_t bt,ft;
#endif
	FILETIME ct,et,kt,ut;
	ULONGLONG ctv,etv,ktv,utv;
	DWORD tec=0,pec=0;
	BOOL inq=0;
	/* Get command line and strip to only arguments */
	cm = cl = GetCommandLine();
	if(!cl) { fprintf(stderr, "Failed to get command line, error code %d.\n", GetLastError()); return 1; }
	while(inq || !((*cl == (TCHAR)' ') || (*cl == (TCHAR)'\0'))) {
		if(*cl == '"') inq = !inq;
		cl += sizeof(TCHAR);
	}
	while((*cl == (TCHAR) ' ') && (*cl != (TCHAR) '\0')) cl += sizeof(TCHAR);
	/* Print help on empty command line */
	if(*cl == (TCHAR) '\0') {
		fprintf(stdout, "usage: ProcProfile [commandline]\n");
		return 1;
	}
	/* Setup structures */
	GetSystemInfo(&sysInfo);
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	mc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
	/* Create process */
	if(CreateProcess(NULL, cl, NULL, NULL, 0, 0, NULL, NULL, &si, &pi)) {
		/* Retrieve start time */
#ifdef STDPTIME
		bt = clock();
#endif
		/* Add special control handler */
		SetConsoleCtrlHandler(breakHdl, 1);
		/* Wait for process exit */
#ifdef POLLING
#ifdef WAITPOLLING
		while(WaitForSingleObject(pi.hProcess, CHECKINTERVAL) == WAIT_TIMEOUT) {
#else
		while(GetExitCodeProcess(pi.hProcess) == STILL_ACTIVE) {
#endif
			Sleep(POLLINTERVAL);
		}
#else
		WaitForSingleObject(pi.hProcess, INFINITE);
#endif
		/* Retrieve end time */
#ifdef STDPTIME
		ft = clock();
#endif
		/* Get process information */
		GetProcessMemoryInfo(pi.hProcess, &mc, sizeof(PROCESS_MEMORY_COUNTERS));
		GetProcessIoCounters(pi.hProcess, &ic);
		GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut);
		GetExitCodeProcess(pi.hProcess, &pec);
		GetExitCodeThread(pi.hThread, &tec);
		/* Convert times into integers */
#ifdef STDPTIME
		ctv = bt;
		etv = ft;
#else
		ctv = ct.dwLowDateTime | ((ULONGLONG)ct.dwHighDateTime << 32);
		etv = et.dwLowDateTime | ((ULONGLONG)et.dwHighDateTime << 32);
#endif
		ktv = kt.dwLowDateTime | ((ULONGLONG)kt.dwHighDateTime << 32);
		utv = ut.dwLowDateTime | ((ULONGLONG)ut.dwHighDateTime << 32);
		/* Convert times into miliseconds */
#ifdef STDPTIME
		ctv = (ctv * 1000) / CLOCKS_PER_SEC;
		etv = (etv * 1000) / CLOCKS_PER_SEC;
#else
		ctv /= 10000;
		etv /= 10000;
#endif
		ktv /= 10000;
		utv /= 10000;
		/* Fix time disorder */
		if(etv < ctv) etv = ctv;
		/* Print process information */
		fprintf(stderr, "\n");
		fprintf(stderr, "Process ID       : %d\n", pi.dwProcessId);
		fprintf(stderr, "Thread ID        : %d\n", pi.dwThreadId);
		fprintf(stderr, "Process Exit Code: %d\n", pec);
		fprintf(stderr, "Thread Exit Code : %d\n", tec);
		/* fprintf(stderr, "\n");
		fprintf(stderr, "Start Date: \n");
		fprintf(stderr, "End Date  : \n"); */
		fprintf(stderr, "\n");
#ifdef BYTECNT
		fprintf(stderr, "User Time        : %14lld.%03llds\n", utv/1000, utv%1000);
		fprintf(stderr, "Kernel Time      : %14lld.%03llds\n", ktv/1000, ktv%1000);
		fprintf(stderr, "Process Time     : %14lld.%03llds\n", (utv+ktv)/1000, (utv+ktv)%1000);
		fprintf(stderr, "Clock Time       : %14lld.%03llds\n", (etv-ctv)/1000, (etv-ctv)%1000);
		fprintf(stderr, "\n");
		fprintf(stderr, "Working Set      : %18lld bytes\n", (ULONGLONG)mc.PeakWorkingSetSize);
		fprintf(stderr, "Paged Pool       : %18lld bytes\n", (ULONGLONG)mc.QuotaPeakPagedPoolUsage);
		fprintf(stderr, "Nonpaged Pool    : %18lld bytes\n", (ULONGLONG)mc.QuotaPeakNonPagedPoolUsage);
		fprintf(stderr, "Pagefile         : %18lld bytes\n", (ULONGLONG)mc.PeakPagefileUsage);
		fprintf(stderr, "Page Fault Count : %18lld\n", (ULONGLONG)mc.PageFaultCount);
		fprintf(stderr, "Page Size:       : %18lld bytes\n", (ULONGLONG)sysInfo.dwPageSize);
		fprintf(stderr, "Page Fault I/O   : %18lld bytes\n", (ULONGLONG)mc.PageFaultCount * sysInfo.dwPageSize);
		fprintf(stderr, "\n");
		fprintf(stderr, "IO Read          : %18lld bytes (in %15lld reads )\n", ic.ReadTransferCount, ic.ReadOperationCount);
		fprintf(stderr, "IO Write         : %18lld bytes (in %15lld writes)\n", ic.WriteTransferCount, ic.WriteOperationCount);
		fprintf(stderr, "IO Other         : %18lld bytes (in %15lld others)\n", ic.OtherTransferCount, ic.OtherOperationCount);
#else
		fprintf(stderr, "User Time        : %11lld.%03llds\n", utv/1000, utv%1000);
		fprintf(stderr, "Kernel Time      : %11lld.%03llds\n", ktv/1000, ktv%1000);
		fprintf(stderr, "Process Time     : %11lld.%03llds\n", (utv+ktv)/1000, (utv+ktv)%1000);
		fprintf(stderr, "Clock Time       : %11lld.%03llds\n", (etv-ctv)/1000, (etv-ctv)%1000);
		fprintf(stderr, "\n");
		fprintf(stderr, "Working Set      : %15lld KB\n", (ULONGLONG)mc.PeakWorkingSetSize>>10);
		fprintf(stderr, "Paged Pool       : %15lld KB\n", (ULONGLONG)mc.QuotaPeakPagedPoolUsage>>10);
		fprintf(stderr, "Nonpaged Pool    : %15lld KB\n", (ULONGLONG)mc.QuotaPeakNonPagedPoolUsage>>10);
		fprintf(stderr, "Pagefile         : %15lld KB\n", (ULONGLONG)mc.PeakPagefileUsage>>10);
		fprintf(stderr, "Page Fault Count : %15lld\n", (ULONGLONG)mc.PageFaultCount);
		fprintf(stderr, "Page Size:       : %15lld KB\n", (ULONGLONG)sysInfo.dwPageSize>>10);
		fprintf(stderr, "Page Fault I/O   : %15lld KB\n", ((ULONGLONG)mc.PageFaultCount * sysInfo.dwPageSize)>>10);
		fprintf(stderr, "\n");
		fprintf(stderr, "IO Read          : %15lld KB (in %15lld reads )\n", ic.ReadTransferCount>>10, ic.ReadOperationCount);
		fprintf(stderr, "IO Write         : %15lld KB (in %15lld writes)\n", ic.WriteTransferCount>>10, ic.WriteOperationCount);
		fprintf(stderr, "IO Other         : %15lld KB (in %15lld others)\n", ic.OtherTransferCount>>10, ic.OtherOperationCount);
#endif
		fprintf(stderr, "\n");
		/* Close process and thread handles */
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		/* Remove the special control handler */
		SetConsoleCtrlHandler(NULL, 0);
		LocalFree(cm);
		return 0;
	} else {
		/* An error occured, print the error */
		fprintf(stderr, "Failed to start process, error code %d.\n", GetLastError());
		LocalFree(cm);
		return 1;
	}
}
