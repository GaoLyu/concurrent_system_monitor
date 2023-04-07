#include "stat.h"

void repeat(char *string, int num){
   for(int i=0;i<num;i++){
      printf("%s",string);
   }
}

void system_info(){
   printf("-----------------------------------\n");
   printf("### System Information ###\n");
   struct utsname uts_name;
   uname(&uts_name);
   printf("System Name = %s\n",uts_name.sysname);
   printf("Machine Name = %s\n",uts_name.nodename);
   printf("Version = %s\n",uts_name.version);
   printf("Release = %s\n",uts_name.release);
   printf("Architecture = %s\n",uts_name.machine);
   printf("-----------------------------------\n");
}

void write_memory(int fd){
   struct sysinfo info;
   sysinfo(&info);
   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!stderr
   //convert byte to gitabyte
   float gb=1073741824.0;
   float p_used=(info.totalram-info.freeram)/gb;
   float p_tot=info.totalram/gb;
   float v_used=p_used+(info.totalswap-info.freeswap)/gb;
   float v_tot=p_tot+info.totalswap/gb;
   struct memory mem;
   mem.phy_used=p_used;
   mem.phy_tot=p_tot;
   mem.vir_used=v_used;
   mem.vir_tot=v_tot;
   if(write(fd, &mem, sizeof(struct memory))==-1){
      //????????????????????????child process write to stderr, can i direct it to main stderr
      fprintf(stderr,"child writes memory");
      exit(1);
   }
   close(fd);
}

void get_memory(int fd, struct memory memories[], int i){
   struct memory mem;
   if(read(fd, &mem, sizeof(struct memory))==-1){
      fprintf(stderr,"parent reads memory");
      //??????????????????????????????
   }
   memories[i].phy_used=mem.phy_used;
   memories[i].phy_tot=mem.phy_tot;
   memories[i].vir_used=mem.vir_used;
   memories[i].vir_tot=mem.vir_tot;
}

void print_one_memory(struct memory memories[], int j){
   printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n",
      memories[j].phy_used,
      memories[j].phy_tot,
      memories[j].vir_used,
      memories[j].vir_tot);
}

void print_memory(struct memory memories[], int i){
   for(int j=0;j<=i;j++){
      print_one_memory(memories,j);
   }
}

void print_one_memory_graphics(struct memory memories[], int j){
   printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB      |",
      memories[j].phy_used,
      memories[j].phy_tot,
      memories[j].vir_used,
      memories[j].vir_tot);
   if(j==0){
      printf("o  ");
      printf("%.2f (%.2f)\n",0.00,memories[j].phy_used);
   }
   else{
      float s=memories[j].phy_used-memories[j-1].phy_used;
      int sub=s*100;
      if(s<0){
         repeat(":",abs(sub));
         printf("@  ");
      }
      else{
         repeat("#",sub);
         printf("*  ");
      }
      printf("%.2f (%.2f)\n",memories[j].phy_used-memories[j-1].phy_used,memories[j].phy_used);
   }
}

void print_memory_graphics(struct memory memories[], int i){
   for(int j=0;j<=i;j++){
      print_one_memory_graphics(memories,j);
   }
}

void write_user_session(int fd){
   struct utmp *u;
   setutent();
   char string[1024];
   while((u=getutent())){
      if(u->ut_type==USER_PROCESS){
         sprintf(string, "%s      %s (%s)\n",
            u->ut_user,u->ut_line,u->ut_host);
         if(write(fd, string, strlen(string))==-1){
            fprintf(stderr,"child writes user session");
            exit(1);
         }
      }
   }
   close(fd);
}

void get_user_session(int fd,char* buf){
   memset(buf,'\0',102400);
   char string[1024];
   memset(string,'\0',1024);
   while(read(fd,string,1024)>0){
      strcat(buf,string);
   }
   close(fd);
}

void write_cpu_usage(int fd){
   FILE *fptr;
   struct cpuUsage cpu; 
   int a1,a2,a3,a4,a5,a6,a7;
   fptr = fopen("/proc/stat","r");
   if(fptr==NULL){
      fprintf(stderr,"Error opening the file.");
      exit(1);
   }
   else{
      char b[1024];
      fscanf(fptr,"%s %d %d %d %d %d %d %d",
         b, &a1, &a2, &a3, &a4, &a5, &a6, &a7);
      float totaluse=a1+a2+a3+a5+a6+a7;
      float idl=a4;
      cpu.util=totaluse;
      cpu.idle=idl;
      int i=fclose(fptr);
      if(i!=0){
         fprintf(stderr,"Error closing the file.");
         exit(1);
      }
      write(fd, &cpu, sizeof(struct cpuUsage));
      close(fd);
   }
}

void get_cpu_usage(int fd, float cpu[],float idle[],int i){
   struct cpuUsage cpu1;
   read(fd,&cpu1,sizeof(struct cpuUsage));
   close(fd);
   cpu[i]=cpu1.util;
   idle[i]=cpu1.idle;
}

float cpu_use_value(float cpu[],float idle[],int i){
   float cpu_usage;
   if(i==0){
      cpu_usage=0.0;
   }
   else{
      float dt_idle=cpu[i]-cpu[i-1];
      float dt=cpu[i]-cpu[i-1]+idle[i]-idle[i-1];
      cpu_usage=(dt_idle)/dt*100.0;
   }
   return cpu_usage;
}

float cpu_use(float cpu[],float idle[],int i){
   float cpu_usage=cpu_use_value(cpu,idle,i);
   printf("total cpu use = %.2f %% \n",cpu_usage);
   return cpu_usage;
}


void cpu_use_one_graphics(float cpu[],float idle[],int i){
   float cpu_usage=cpu_use_value(cpu,idle,i);
   printf("          |||");
   int n=cpu_usage;
   repeat("|",n);
   printf(" %.2f\n",cpu_usage);
}


void cpu_use_graphics(float cpu[],float idle[],int i){
   int j;
   cpu_use(cpu,idle,i);
   for(j=0;j<=i;j++){
      cpu_use_one_graphics(cpu,idle,j);
   }

}

int cpu_core(){
   int num=0;
   FILE *fptr;
   fptr = fopen("/proc/stat","r");

   if(fptr==NULL){
      fprintf(stderr,"Error opening the file.");
      exit(1);
   }
   else{
      char line[1024];
      fgets(line,1024,fptr);
      while(strncmp(line,"intr",4)!=0){
         fgets(line,1024,fptr);
         num++;
      }
      int i=fclose(fptr);
      if(i!=0){
         fprintf(stderr,"Error closing the file.");
         exit(1);
      }
      else{
         return num-1;
      }
   }
}

void program_usage(){
   struct rusage usage;
   getrusage(RUSAGE_SELF, &usage);
   printf("Memory usage: %ld kilobytes\n",usage.ru_maxrss);
}