// Gmsh - Copyright (C) 1997-2013 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.

#ifndef _OS_H_
#define _OS_H_

#include <string>
#include <stdio.h>

const char *GetEnvironmentVar(const char *var);
void SetEnvironmentVar(const char *var, const char *val);
double GetTimeInSeconds();
void SleepInSeconds(double s);
void CheckResources();
double Cpu();
double TotalRam();
long GetMemoryUsage();
int GetProcessId();
std::string GetHostName();
int UnlinkFile(const std::string &fileName);
int StatFile(const std::string &fileName);
int KillProcess(int pid);
int CreateDirectory(const std::string &dirName);
int SystemCall(const std::string &command, bool blocking=false);
std::string GetCurrentWorkdir();
void RedirectIOToConsole();
FILE *Fopen(const char* f, const char *mode);

#endif
