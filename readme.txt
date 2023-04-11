1. How to run the program:
First type make, and makefile will automatically create 'all' which is the exectuable file. Then type ./all followed by the command line arguments.
Since I use printf("\033[H\033[J") to clear the screen, please make sure the screen is large and font is small, so the cursor can move to the top left and clears all that have been printed.


2. How I solve the problem: 

To make the program run concurrently, in each iteration the parent will fork 3 children (or less depending on the command line argument), and each children writes a specific information into the pipe. The parent will wait for all 3 children, and after they have all finished writing, the parent will read from corresponding pipe, and print them to screen. The parent has an array for cpu and memory with sample size, so for each iteration after it reads from the pipe, it will store them in correct position in the array, and use them for graphics. It has a long string for user session and the string stores all user information for the current iteration.

To make code modular, I move all functions related to getting and presenting the information into stat.c and let main.c to arrange how to get the information and the order to print.

There are no zombies or orphans because the parent always waits for all the children it has forked. If the user want to quit the program, before exit(0), the parent run while (wait(NULL)>0){} to make sure all children exits normally.

I created two signal handler functions, ctrlZ(int sig) and ctrlC(int sig), and use sigaction to set them up and change the behabvior of ctrl-Z and ctrl-C.

To show the graphical representation of cpu usage, except for the first 3 "|" set by default, each "|" represents a 1% above 0. For example, if the cpu usage of 3 samples are 0.00%, 2.31%, 11.23%, then the 3 samples will have 3, 5, 14 "|"s respectively. cpu_use_one_graphics() and cpu_use_graphics() prints out the result.

To get graphical representation of memory, I compare the 2 consecutive memory usage. If they differ by a negative value, ":" followed by "@" will be displayed. Otherwise, "|" followed by "*" will be displayed. Each symbol corresponds to a 0.1 difference. print_one_memory_graphics() prints the memory for a specific sample. print_memory_graphics() prints all the stored memory.

3. An overview of the functions:
In stat.h:

	1. void repeat(char *string, int num) prints string num of times

	2. void system_info() prints information about the system

	3. void write_memory(int fd) writes the current memory information to pipe specified by fd 

	4. void get_memory(int fd, struct memory memories[], int i) reads the current memory information from pipe specified by fd and stores in array memories at index i

	5. void print_one_memory(struct memory memories[], int j) prints the memory information at index j

	6. void print_memory(struct memory memories[], int i) prints all stored memory information from index 0 to i (inclusive)

	7. void print_one_memory_graphics(struct memory memories[], int j) prints the memory information with graphics at index j

	8. void print_memory_graphics(struct memory memories[], int i) prints all stored memory information with graphics from index 0 to i (inclusive)

	9. void write_user_session(int fd) writes the user information to pipe specified by fd

	10. void get_user_session(int fd, char* buf) reads the user information from pipe specified by fd and prints to screen

	11. void write_cpu_usage(int fd) writes the cpu usage of the current sample to pipe specified by fd

	12. void get_cpu_usage(int fd, float cpu[],float idle[],int i) reads the cpu usage of the current sample and stores it in array cpu[], idle[] at index i

	13. float cpu_use_value(float cpu[],float idle[],int i) gets cpu_usage by comparing the information stored at index i-1 and i of cpu[] and idle[]

	14. float cpu_use(float cpu[],float idle[],int i) prints and returns cpu usage given from cpu[] and idle[] at index i

	15. void cpu_use_one_graphics(float cpu[],float idle[],int i) prints one cpu usage and its graphics given from cpu[] and idle[] at index i

	16. void cpu_use_graphics(float cpu[],float idle[],int i) prints cpu usage and its graphics given by cpu[] and idle[] from index 0 to index i

	17. int cpu_core() gets number of cores by calculating how many iterations needed to get to line with "intr"

	18. void program_usage() prints program usage

In main.c:
	
	1. bool isnumber(char string[]) returns ture if stirng is made up of numbers

	2. void get_command(int argc, char **argv, struct option long_options[]) gets command from argc and argv according to long_options

	3. void sample_tdelay(int sample,int time) prints sample size and delay time

	4. void get_cpu_memory(struct memory memories[],float cpu[],float idle[],int iter) calls fork 2 times and creates 2 pipes. One child gets the current cpu information and writes to pipe, the other one gets the current memory and writes to the other pipe. The parent waits until both children have finished writing, then reads from pipe, and stores what it read to memories, cpu, idle at index iter

	5. void get_user(char *buf) creates one child. The child writes all users to pipe, and parent reads from pipe and stores the string in buf

	6. void get_cpu_memory_user(struct memory memories[],float cpu[],float idle[],char *buf,int iter) forks 3 times. Each child writes the information it gets to the pipe. After they have all finished writing, the parent reads from pipe and stores the information in memories, cpu, idle at index iter and in buf

	7. void sequential(int sample,int time,struct memory memories[], struct option long_options[],float cpu[],float idle[],char buf[102400],int graphics_flag) will create sample iterations. In each iteration, based on the long_options, it either calls get_cpu_memory() or get_user() or get_cpu_memory_user() to get current information and prints them in specified order

	8. void system_opt(int sample,int time, struct memory memories[], float cpu[], float idle[], int graphics_flag) creates sample iterations. In each iteration, it clears the screen, call get_cpu_memory() to get current information and prints them

	9. void user_opt(int sample,int time, char*buf) creates sample iterations, where it clears the screen, call get_user() to get current information and prints them

	10. void all(int sample, int time, struct memory memories[],float cpu[], float idle[],char *buf,int graphics_flag) creates sample iterations, where it clears the screen, call get_cpu_memory_user() to get current information and prints them

	11. ctrlZ(int sig) ignores the signal

	12. ctrlC(int sig) does nothing if the user don't want to quit the program, and will wait all children then exit if the user wants to quit the program

	13. In main, the program first gets all command line arguments, and based on it creates arrays and string to store information. Then it calls system_opt() or user_opt() or all() to prints the result