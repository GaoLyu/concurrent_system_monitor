#include "stat.h"
#include<sys/wait.h>

bool isnumber(char string[]){
   for(int i=0;i<strlen(string);i++){
      if(!(isdigit(string[i]))){
         return false;
      }
   }
   return true;
}

void get_command(int argc, char **argv, struct option long_options[]){
   int c;
   int option_index;
   int i=0;
   while((c=getopt_long(argc,argv,"",long_options,&option_index))!=-1){
      switch(c){
         case 0:
            if(strcmp(long_options[option_index].name, "samples")==0){
               if(optarg){
                  *(long_options[option_index].flag)=atoi(optarg);
               }
            }
            if(strcmp(long_options[option_index].name, "tdelay")==0){
               if(optarg){
                  *(long_options[option_index].flag)=atoi(optarg);
               }
            }
      }
   }
   if(*(long_options[3].flag)==-1 && *(long_options[4].flag)==-1){
      while(optind<argc && i<2){
         if(isnumber(argv[optind])){
            *(long_options[i+3].flag)=atoi(argv[optind]);
            i++;
         }
         optind++;
      }
   }
}

void sample_tdelay(int sample,int time){
   printf("Nbr of samples: %d -- every %d secs\n",sample,time);
}


void get_cpu_memory(struct memory memories[],float cpu[],float idle[],int iter){
   pid_t child_cpu, child_memory;
   int status0,status1;
   int fd[2][2];
   if(pipe(fd[0])==-1){
      fprintf(stderr,"error create pipe for child cpu");
      exit(1);
   }
   child_cpu=fork();
   if(child_cpu<0){
      fprintf(stderr,"fork child cpu process");
      exit(1);
   }
   else if(child_cpu==0){
      //child cpu
      //close all reading ends
      close(fd[0][0]);
      write_cpu_usage(fd[0][1]);
      exit(0);
   }
   else{
      //parent
      close(fd[0][1]);
      if(pipe(fd[1])==-1){
         fprintf(stderr,"error create pipe for child memory");
         exit(1);
      }
      child_memory=fork();
      if(child_memory<0){
         fprintf(stderr,"fork child memory process");
         exit(1);
      }
      else if(child_memory==0){
         //child memory
         close(fd[1][0]);
         close(fd[0][0]);
         write_memory(fd[1][1]);
         exit(0);
      }
      else{
         //parent
         close(fd[1][1]);
         while(waitpid(child_cpu,&status0,WNOHANG)==0 || waitpid(child_memory,&status1,WNOHANG)==0){

         }
         if(WEXITSTATUS(status0)==1 || WEXITSTATUS(status1)==1){
            exit(1);
         } 
         get_cpu_usage(fd[0][0],cpu,idle,iter);
         get_memory(fd[1][0],memories,iter);
      }
   }
}

void get_user(char *buf){
   pid_t child_user;
   int status;
   int fd[2];
   if(pipe(fd)==-1){
      fprintf(stderr,"error create pipe for child user");
      exit(1);
   }
   child_user=fork();
   if(child_user<0){
      fprintf(stderr,"fork child cpu process");
      exit(1);
   }
   else if(child_user==0){
      //child
      close(fd[0]);
      write_user_session(fd[1]);
      exit(0);
   }
   else{
      //parent
      close(fd[1]);
      waitpid(child_user,&status,0);
      if(WEXITSTATUS(status)==1){
         exit(1);
      }
      get_user_session(fd[0],buf);
   }
}

void get_cpu_memory_user(struct memory memories[],float cpu[],float idle[],char *buf,int iter){
   pid_t child_cpu, child_memory, child_user;
   int status0,status1,status2;
   int fd[3][2];
   if(pipe(fd[0])==-1){
      fprintf(stderr,"error create pipe for child cpu");
      exit(1);
   }
   child_cpu=fork();
   if(child_cpu<0){
      fprintf(stderr,"fork child cpu process");
      exit(1);
   }
   else if(child_cpu==0){
      //child cpu
      //close all reading ends
      close(fd[0][0]);
      write_cpu_usage(fd[0][1]);
      exit(0);
   }
   else{
      //parent
      close(fd[0][1]);
      if(pipe(fd[1])==-1){
         fprintf(stderr,"error create pipe for child memory");
         exit(1);
      }
      child_memory=fork();
      if(child_memory<0){
         fprintf(stderr,"fork child memory process");
         exit(1);
      }
      else if(child_memory==0){
         //child memory
         close(fd[1][0]);
         close(fd[0][0]);
         write_memory(fd[1][1]);
         exit(0);
      }
      else{
         //parent
         close(fd[1][1]);
         if(pipe(fd[2])==-1){
            fprintf(stderr,"error create pipe for child user");
            exit(1);
         }
         child_user=fork();
         if(child_user<0){
            fprintf(stderr,"fork child user process");
            exit(1);
         }
         else if(child_user==0){
            //child memory
            close(fd[2][0]);
            close(fd[1][0]);
            close(fd[0][0]);
            write_user_session(fd[2][1]);
            exit(0);
         }
         else{
            //parent
            close(fd[2][1]);
            while(waitpid(child_cpu,&status0,WNOHANG)==0 || waitpid(child_memory,&status1,WNOHANG)==0 || waitpid(child_user,&status2,WNOHANG)==0){

            }
            if(WEXITSTATUS(status0)==1 || WEXITSTATUS(status1)==1 || WEXITSTATUS(status0)==1){
               exit(1);
            }
            get_cpu_usage(fd[0][0],cpu,idle,iter);
            get_memory(fd[1][0],memories,iter);
            get_user_session(fd[2][0],buf);
         }
      }
   }
}



void sequential(int sample,int time,struct memory memories[],
   struct option long_options[],float cpu[],float idle[],char buf[102400],int graphics_flag){
   
   sample_tdelay(sample,time);
   int i=0;
   int num=cpu_core();
   for(i=0;i<sample;i++){
      if(*(long_options[0].flag)==1 && *(long_options[0].flag)!=1){
         get_cpu_memory(memories,cpu,idle,i);
      }
      else if(*(long_options[0].flag)!=1 && *(long_options[0].flag)==1){
         get_user(buf);
      }
      else{
         get_cpu_memory_user(memories,cpu,idle,buf,i);
      }
      printf(">>> iteration %d\n",i);
      program_usage();
      printf("-----------------------------------\n");
      //print memory
      if(*(long_options[0].flag)==1 || 
         (*(long_options[0].flag)!=1 && *(long_options[1].flag)!=1)){
         printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
         repeat("\n",i);
         
         if(graphics_flag==0){
            print_one_memory(memories,i);
         }
         else{
            print_one_memory_graphics(memories,i);
         }
         repeat("\n",sample-1-i);
         printf("-----------------------------------\n");
         printf("Number of cores: %d\n",num);

         if(graphics_flag==0){
            cpu_use(cpu,idle,i);
         }
         else{
            cpu_use_graphics(cpu,idle,i);
         }

         if(*(long_options[0].flag)!=1 && *(long_options[1].flag)!=1){
            printf("-----------------------------------\n");
         }
      }
      if(*(long_options[1].flag)==1 || 
         (*(long_options[0].flag)!=1 && *(long_options[1].flag)!=1)){
         printf("### Sessions/users ###\n");
         printf("%s",buf);
      }
      sleep(time);
   }
}

void system_opt(int sample,int time, struct memory memories[], float cpu[], float idle[], int graphics_flag){
   int i=0;
   int num=cpu_core();
   for(i=0;i<sample;i++){
      get_cpu_memory(memories,cpu,idle,i);
      //system("clear");
      printf("\033[H\033[J");
      //printf("\x1b[H\x1b[2J");
      sample_tdelay(sample,time);
      program_usage();
      printf("-----------------------------------\n");
      printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
      if(graphics_flag==0){
         print_memory(memories,i);
      }
      else{
         print_memory_graphics(memories,i);
      }
      repeat("\n",sample-1-i);
      printf("-----------------------------------\n");
      printf("Number of cores: %d\n",num);
      if(graphics_flag==0){
         printf("cpu:%f\tidle:%f\tcpu use value:%f ",cpu[i],idle[i],cpu_use_value(cpu,idle,i));
         cpu_use(cpu,idle,i);
      }
      else{
         cpu_use_graphics(cpu,idle,i);
      }
      sleep(time);  
   }
}  

void user_opt(int sample,int time, char*buf){
   int i;
   for(i=0;i<sample;i++){
      get_user(buf);
      //system("clear");
      printf("\033[H\033[J");
      //printf("\x1b[H\x1b[2J");
      sample_tdelay(sample,time);
      program_usage();
      printf("-----------------------------------\n");
      printf("### Sessions/users ###\n");
      printf("%s",buf);
      sleep(time);
   }
}

void all(int sample, int time, struct memory memories[],float cpu[], float idle[],char *buf,int graphics_flag){
   int i;
   int num=cpu_core();
   for(i=0;i<sample;i++){
      get_cpu_memory_user(memories,cpu,idle,buf,i);
      //system("clear");
      printf("\033[H\033[J");
      //printf("\x1b[H\x1b[2J");
      sample_tdelay(sample,time);
      program_usage(); 
      printf("-----------------------------------\n");
      printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
      if(graphics_flag==0){
         print_memory(memories,i);
      }
      else{
         print_memory_graphics(memories,i);
      }
      repeat("\n",sample-1-i);
      printf("-----------------------------------\n");
      printf("### Sessions/users ###\n");
      printf("%s",buf);
      printf("-----------------------------------\n");
      printf("Number of cores: %d\n",num);
      if(graphics_flag==0){
         cpu_use(cpu,idle,i);
      }
      else{
         cpu_use_graphics(cpu,idle,i);
      }
      sleep(time);
   }
}

void ctrlZ(int sig){
   return;
}
void ctrlC(int sig){
   char answer;
   printf("Do you want to quit the program? enter y/n: \n");
   scanf("%c",&answer);
   if(answer=='y'){
      while(wait(NULL)>0){}
      exit(0);
   }
   else{
      return;
   }
}

int main(int argc, char **argv){
   struct sigaction sa1;
   sa1.sa_handler = ctrlZ;
   sigemptyset(&sa1.sa_mask);
   sa1.sa_flags = 0;
   sigaction(SIGTSTP, &sa1, NULL);

   struct sigaction sa2;
   sa2.sa_handler = ctrlC;
   sigemptyset(&sa2.sa_mask);
   sa2.sa_flags = 0;
   sigaction(SIGINT, &sa2, NULL);

   int system_flag=0;
   int user_flag=0;
   int sequential_flag=0;
   int graphics_flag=0;
   int sample=-1;
   int time=-1;
   struct option long_options[]={
      {"system",0,&system_flag,1},
      {"user",0,&user_flag,1},
      {"sequential",0,&sequential_flag,1},
      {"samples",2,&sample,10},
      {"tdelay",2,&time,1},
      {"graphics",0,&graphics_flag,1},
      {0,0,0,0}
   };
   get_command(argc,argv,long_options);
   if(sample==-1){
      sample=10;
   }
   if(time==-1){
      time=1;
   }

   struct memory memories[sample]; 
   float cpu[sample];
   float idle[sample];
   char buf[102400];

   if(sequential_flag==0){
      if(system_flag==1 && user_flag==0){
         system_opt(sample,time,memories,cpu,idle,graphics_flag);
      }
      
      else if(user_flag==1 && system_flag==0){
         user_opt(sample,time,buf);
      }
      else{
         all(sample,time,memories,cpu,idle,buf,graphics_flag);
      }
   }
   else{
      sequential(sample,time,memories,long_options,cpu,idle,buf,graphics_flag);
   }
   system_info();
   return 0;
}