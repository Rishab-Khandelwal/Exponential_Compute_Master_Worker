#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<getopt.h>
#include<sys/stat.h>

double compute(int x , int n);

int main(int argc , char * argv[]){

  //if(argc !=5)
  //return 0;

  struct stat sb;

  int aflag = 0;
  int bflag = 0;
  int c;
  int x, n;

  while((c= getopt(argc,argv,"x:n:")) != -1)
{
  switch (c)
      {
      case 'x':
      if (optarg != NULL)
      {
        x = atoi(optarg);
        aflag = 1;
      }
      break;

      case 'n':
        if (optarg != NULL)
        {
          n = atoi(optarg);
          bflag = 1;
        }
        break;

      case '?':
        if (optopt == 'x' || optopt == 'n')
        {
          printf("Insufficient Argument \n");
        }
        break;

      default :
        printf("Incorrext arguments \n");
        break;
      }
  }

if (aflag == 0 || bflag == 0)
{
  printf("Did not enter both the arguments");
  return 0;
}
  double result = compute(x,n);
  fstat(1,&sb);
  if(S_ISFIFO(sb.st_mode))
  {
    printf(" %d ^ %d /%d ! :%f \n",x,n,n,result);
  }
  else{
    printf("x^n / n! : %f ",result);
  }
}
double compute(int x , int n){

  if (n == 0)
  return 1;

  int fact = 1;
  int i;
  for(i = 1 ; i <= n ; i++)
  {

    fact = fact * i;
  }

  double result = pow(x,n)/fact;
  return result;
}
