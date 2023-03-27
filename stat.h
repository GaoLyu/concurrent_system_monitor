#include<stdio.h>
#include<stdlib.h>
#include<sys/utsname.h>
#include<sys/sysinfo.h>
#include <utmp.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <ctype.h>

struct memory{
   float phy_used,phy_tot,vir_used,vir_tot;
};

struct cpuUsage{
	float util, idle;
};

// print string num of times
void repeat(char *string, int num);

// print information about the system
void system_info();

// write the current memory information to pipe specified by fd
void write_memory(int fd);

// read the current memory information from pipe specified by fd
// and store in array memories at index i
void get_memory(int fd, struct memory memories[], int i);

// print the memory information at index j
void print_one_memory(struct memory memories[], int j);

// print all stored memory information 
// from index 0 to i (inclusive)
void print_memory(struct memory memories[], int i);

// print the memory information with graphics at index j
void print_one_memory_graphics(struct memory memories[], int j);

// print all stored memory information with graphics
// from index 0 to i (inclusive)
void print_memory_graphics(struct memory memories[], int i);

// write the user information to pipe specified by fd
void write_user_session(int fd);

// read the user information from pipe specified by fd
// and print to screen
void get_user_session(int fd, char* buf);

// write the cpu usage of the current sample 
// to pipe specified by fd
void write_cpu_usage(int fd);

// read the cpu usage of the current sample 
// and store it in array cpu[], idle[] at index i
void get_cpu_usage(int fd, float cpu[],float idle[],int i);

// get cpu_usage by comparing the information
// stored at index i-1 and i of cpu[] and idle[]
float cpu_use_value(float cpu[],float idle[],int i);

// print and return cpu usage given from cpu[] and idle[] at index i
float cpu_use(float cpu[],float idle[],int i);

// print one cpu usage and its graphics given from cpu[] and idle[] at index i
void cpu_use_one_graphics(float cpu[],float idle[],int i);

// print cpu usage and its graphics given by cpu[] and idle[] from index 0 to index i
void cpu_use_graphics(float cpu[],float idle[],int i);

// get number of cores by calculating how many iterations needed
// to get to line with "intr"
int cpu_core();

// print program usage
void program_usage();



