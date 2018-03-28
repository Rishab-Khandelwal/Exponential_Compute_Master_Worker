#include<stdio.h>
#include<string.h>
#include<sys/select.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<getopt.h>

void computeExpression(char *workerpath,char *mechanism,char* x, char* n , char* num_workers);

int main(int argc , char * argv[]){

  char *workerpath;
  char *wait_mechanism;
  char*  num_workers;
  int x_flag = 0;
  int n_flag = 0;
  int wait_mechanism_flag = 0;
  int num_workers_flag  = 0;
  int worker_path_flag = 0;
  char* n;
  char* x;
  int c;
    while (1)
      {
        static struct option long_options[] =
          {
            /* These options set a flag. */
            {"worker_path", required_argument,   NULL, 'w'},
            {"num_workers",   required_argument,      NULL, 'b'},
            /* These options don’t set a flag.
               We distinguish them by their indices. */
            {"wait_mechanism",    required_argument,       NULL, 'a'},
            {"x",  required_argument,       NULL, 'x'},
            {"n",  required_argument, NULL, 'n'},
            {0, 0, 0, 0}
          };
        /* getopt_long stores the option inx here. */
        int option_inx = 0;

        c = getopt_long (argc, argv, "w:b:a:x:n:",
                         long_options, &option_inx);
        /* Detect the end of the options. */
        if (c == -1)
          break;

        switch (c)
          {
          case 'w':
            if(optarg != NULL)
            {
                workerpath = optarg;
                worker_path_flag = 1;
            }
            break;

          case 'n':
            if (optarg != NULL)
            {
              n = optarg;
              n_flag = 1;
            }
            break;
          case 'b':
          if(optarg != NULL)
          {
            num_workers = optarg;
            num_workers_flag = 1;
          }
          break;

          case 'a':
           if(optarg != NULL)
           {
             wait_mechanism = optarg;
             wait_mechanism_flag = 1;
           }
            break;

          case 'x':
            if(optarg != NULL)
            {
              x = optarg;
              x_flag = 1;
            }
            break;

          case '?':
          if (optopt == 'x' || optopt == 'n'||optopt =='a' || optopt == 'w' || optopt == 'b')
          {
            printf("Insufficient Argument \n");
          }
            break;

          default:
          printf("Incorrect Arguments \n");
          exit(0);
          }
      }
      if (x_flag == 0 || n_flag == 0 || wait_mechanism_flag == 0 || num_workers_flag ==0 || worker_path_flag == 0)
      {
        printf("Did not enter all the arguments \n");
        return 0;
      }
      computeExpression(workerpath,wait_mechanism,x,n,num_workers);

}

void computeExpression(char *workerpath,char *mechanism,char* x, char* n , char* num_workers){

char *argv[6];
struct timeval tv;
int retval;
int pipefds[2];
int num = atoi(n);
int read_side[num+1];
int write_side[num+1];
int cid = -1;
int max_fd  = -1;
char worker_str[50];
int exec_res;
int numWorkers = atoi(num_workers);
argv[0] = "./worker";
argv[1] = "-x";
argv[2] = x;
argv[3] = "-n";
argv[4] = n;
argv[5] = (char*)NULL;
double final = 0;
int max_workers_allowed = 0;
int ctr ;
int leftover;
char buf;

if (strcmp(mechanism,"select") == 0)
{
  struct timeval tv;
  fd_set rdfs;
  FD_ZERO(&rdfs);

  if (numWorkers < num)
  {
    max_workers_allowed = numWorkers;
  }
  else{
    max_workers_allowed = num;
  }
  int start =0;
  while(start < num){
    ctr = max_workers_allowed;
    int i;
  for(i = start ; i < max_workers_allowed ; i++)
  {
    int st = pipe(pipefds);
    if(st == -1)
    perror("Error in pipe");
    FD_SET(pipefds[0],&rdfs);
    read_side[i] = pipefds[0];
    write_side[i] = pipefds[1];
    if (pipefds[0] > max_fd)
    {
      max_fd = pipefds[0];
    }
    int ss = sprintf(argv[4], "%d", i);
    if (ss < 0)
    perror("Error in sprintf");
    cid = fork();
    if (cid == -1)
    {
      perror("Error in fork");
    }
    if (cid == 0)
    {
      int cs = close(read_side[i]);
      if(cs == -1)
      perror("Error in closing");
      int ds = dup2(write_side[i], STDOUT_FILENO);
      if(ds == -1)
      perror("Error in dup2");
      exec_res = execv(workerpath,argv);
      if (exec_res == -1)
      perror("exec failed ff");
      _exit(EXIT_FAILURE);
    }
    else{
      int cs = close(write_side[i]);
      if(cs ==-1)
      perror("Error in closing files");
    }
  }

while(ctr > 0 && ctr > start)
{
  FD_ZERO(&rdfs);
  int i;
  for(i = start ;i < max_workers_allowed ; i++)
  {
    if(read_side[i] != 0){
      FD_SET(read_side[i],&rdfs);
    }
  }
tv.tv_sec = 0;
tv.tv_usec =10;
retval = select(max_fd + 1,&rdfs,NULL,NULL,&tv);
if (retval == -1)
{
  perror("error in select");
  return;
}
else if (retval){
  int i;
  for(i = start ; i < max_workers_allowed ; i++)
  {
    char temp[100] = "";
    int inx = 0,count = 0;
    char * res = "";
    if (FD_ISSET(read_side[i],&rdfs))
    {
      int ss = sprintf(worker_str,"worker %d : ",i);
      if (ss < 0)
      perror("error in sprintf");
      int ws = write(STDOUT_FILENO,worker_str,strlen(worker_str));
      if (ws == -1)
      perror("Error in writing");
      while(read(read_side[i],&buf,1) > 0)
      {
        int ws = write(STDOUT_FILENO,&buf,1);
        if(ws == -1)
        perror("Error in write");
        if (count == 1)
        {
          temp[inx++] = buf;
        }
        if (buf == ':' )
        count ++;
      }
      temp[inx] = '\0';
      double tmp = strtod(temp,NULL);
      final += tmp;
      int cs = close(read_side[i]);
      if (cs == -1)
      perror("Error in closing files");
      read_side[i] = -1;
      ctr --;
    }
  }
  }
}
start = max_workers_allowed;
leftover = num - max_workers_allowed;
if (leftover > numWorkers)
{
  max_workers_allowed += numWorkers;
}else{
  max_workers_allowed += leftover;
}
}
printf("Result after addition : %f \n",final);
exit(0);
}

if (strcmp(mechanism,"epoll") == 0){

  struct epoll_event ev , events[1000];
  int epfd;
  int ans ;
  int read_e,write_e;
  epfd = epoll_create1(0);
  if(epfd == -1){
    perror("epoll");
    exit(EXIT_FAILURE);
  }
  if (numWorkers < num)
  {
    max_workers_allowed = numWorkers;
  }
  else{
    max_workers_allowed = num;
  }
  int start =0;
  while(start < num){
    ctr = max_workers_allowed;
  int i;
  for(i = start ; i < max_workers_allowed ; i++)
  {
      int ps = pipe(pipefds);
      if (ps == -1)
      perror("Error in pipe");
      write_e = pipefds[1];
      read_e = pipefds[0];
      ev.data.fd = read_e;
      ev.events = EPOLLIN|EPOLLET;
      ans = epoll_ctl(epfd,EPOLL_CTL_ADD,read_e,&ev);
      if (ans == -1)
      {
        perror("error in epoll_ctl\n");
        exit(EXIT_FAILURE);
      }
      read_side[i] = read_e;
      write_side[i] = write_e;
      int ss = sprintf(argv[4], "%d", i);
      if (ss < 0)
      perror("Error in sprintf");
      cid = fork();
      fflush(stdout);
      if (cid == 0)
      {
        int cs = close(read_side[i]);
        if (cs == -1)
        perror("Error in closing the file");
        int ds = dup2(write_side[i], STDOUT_FILENO);
        if (ds == -1)
        perror("Error in dup2");
        exec_res = execv(workerpath,argv);
        if (exec_res == -1)
        perror("exec failed ff");
        _exit(EXIT_FAILURE);
      }
      else{
      int cs = close(write_side[i]);
      if(cs == -1)
      perror("error in closing the file");
      }
    }
      while(ctr > 0 && ctr > start)
      {
        int i_eve;
        int neve = epoll_wait(epfd,events, 1000,-1);
        if (neve == 0)
        {
          break;
        }
        for(i_eve = 0; i_eve < neve;i_eve++)
        {
          read_e = events[i_eve].data.fd;
          int i;
          for(i = start ; i < max_workers_allowed;i++)
          {
            if(read_side[i] == read_e)
            {
              int ss = sprintf(worker_str,"worker %d : ",i);
              if (ss < 0)
              perror("Error in sprintf");
              int ws = write(1,worker_str,strlen(worker_str));
              if (ws == -1)
              perror("Error in write");
              int count = 0, inx = 0;
              char temp[60];
              while(read(read_e,&buf,1) > 0)
              {
                write(STDOUT_FILENO,&buf,1);
                if (count == 1)
                {
                  temp[inx++] = buf;
                }
                if (buf == ':' )
                count ++;
              }
              temp[inx++] = '\0';
              double tmp = strtod(temp,NULL);
              final += tmp;
              read_side[i] = -1;
              ctr--;
            }
          }
          close(read_e);
        }
      }
      start = max_workers_allowed;
      leftover = num - max_workers_allowed;
      if (leftover > numWorkers)
      {
        max_workers_allowed += numWorkers;
      }else{
        max_workers_allowed += leftover;
      }
  }
  printf("Result after addition : %f \n",final);
  exit(0);
}
}
