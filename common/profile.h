#ifndef PROFILE_H
#define PROFILE_H

#include <Windows.h>
#include <cassert>
#include <psapi.h>
#include <stdio.h>

PROCESS_MEMORY_COUNTERS get_mem_info(DWORD processID) {
  HANDLE hProcess;
  PROCESS_MEMORY_COUNTERS pmc;

  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                         processID);
  assert(hProcess != nullptr);

  if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
    return pmc;
  } else {
    return {};
  }

  CloseHandle(hProcess);
}

PROCESS_MEMORY_COUNTERS get_mem_info() {
  return get_mem_info(GetCurrentProcessId());
}

void print_mem_info(const PROCESS_MEMORY_COUNTERS &pmc) {
  printf("\tPageFaultCount: %d\n", pmc.PageFaultCount);
  printf("\tPeakWorkingSetSize (bytes): %lld\n", pmc.PeakWorkingSetSize);
  printf("\tWorkingSetSize (bytes): %lld\n", pmc.WorkingSetSize);
}

PROCESS_MEMORY_COUNTERS print_mem_info() {
  auto res = get_mem_info();
  print_mem_info(res);
  return res;
}

size_t get_mem_delta(const PROCESS_MEMORY_COUNTERS &a,
                     const PROCESS_MEMORY_COUNTERS &b) {
  return b.WorkingSetSize - a.WorkingSetSize;
}

#endif // PROFILE_H
