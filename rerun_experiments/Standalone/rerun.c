/* Rerun driver: reuses the paper's auth/asra.c, auth/authentication.c, auth/vehicle.c unchanged. */
#include "vehicle.h"
#include "authentication.h"
#include "asra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>
#include <oqs/oqs.h>

static double now_s(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static double cpu_s(void){struct rusage r;getrusage(RUSAGE_SELF,&r);return r.ru_utime.tv_sec+r.ru_utime.tv_usec*1e-6+r.ru_stime.tv_sec+r.ru_stime.tv_usec*1e-6;}

/* Exp 1: cost and decision behaviour of ASRA configurations */
static void run_exp1(Vehicle *v,int runs,int trials,int n,const char *out){
  FILE *f=fopen(out,"w"); fprintf(f,"i,eval_ms,cv,lrs,reject\n");
  for(int i=0;i<n;i++){
    double t0=now_s(); ASRA_Result r=asra_evaluate_signing_robust(v,runs,trials); double t1=now_s();
    bool acc=asra_authentication_decision(true,&r,0.7);
    fprintf(f,"%d,%.4f,%.6f,%.6f,%d\n",i,(t1-t0)*1e3,r.coeff_of_variation,r.leakage_risk_score,!acc);
  }
  fclose(f);
}

/* Exp 2: one receiver core, fixed arrival rate, verify + ASRA per message, FIFO queue */
static void run_exp2(Vehicle *rx,int rate,int runs,int trials,double dur,FILE *f){
  Vehicle tx; initialize_vehicle(&tx,"TX"); generate_dsa_keys(&tx);
  enum{POOL=64}; uint8_t *sig[POOL]; size_t sl[POOL]; char msg[POOL][64];
  for(int k=0;k<POOL;k++){snprintf(msg[k],64,"V2V_AUTH_CHALLENGE:%.6f",k*0.1);sign_message(&tx,(uint8_t*)msg[k],strlen(msg[k]),&sig[k],&sl[k]);}
  double period=1.0/rate; double start=now_s(), c0=cpu_s();
  long processed=0; double latsum=0,latmax=0; double busy=0;
  double *lat=malloc(sizeof(double)*rate*dur*2+16);
  for(long m=0;;m++){
    double arr=start+m*period; if(arr-start>=dur) break;
    double t=now_s(); if(t-start>=dur) break;
    if(t<arr){ while(now_s()<arr); }      /* idle until next arrival (spin keeps timing simple) */
    double b0=now_s(); int k=m%POOL;
    bool ok=verify_signature(tx.dsa_public_key,(uint8_t*)msg[k],strlen(msg[k]),sig[k],sl[k]);
    ASRA_Result r=asra_evaluate_signing_robust(rx,runs,trials);
    (void)asra_authentication_decision(ok,&r,0.7);
    double b1=now_s(); busy+=b1-b0;
    double l=b1-arr; lat[processed++]=l; latsum+=l; if(l>latmax)latmax=l;
  }
  double wall=now_s()-start, cpu=cpu_s()-c0;
  long offered=(long)(dur*rate);
  /* median latency */
  for(long i=1;i<processed;i++){double x=lat[i];long j=i-1;while(j>=0&&lat[j]>x){lat[j+1]=lat[j];j--;}lat[j+1]=x;}
  fprintf(f,"%d,%dx%d,%ld,%ld,%.2f,%.1f,%.1f,%.1f,%.1f\n",rate,runs,trials,offered,processed,
     processed/wall, 100.0*busy/wall, lat[processed/2]*1e3, lat[(long)(processed*0.95)]*1e3, latmax*1e3);
  fflush(f); free(lat); for(int k=0;k<POOL;k++)free(sig[k]); free_vehicle(&tx);
}

int main(int argc,char**argv){
  OQS_init(); Vehicle v; initialize_vehicle(&v,"RX"); generate_dsa_keys(&v);
  if(!strcmp(argv[1],"exp1")){ run_exp1(&v,atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),argv[5]); }
  else if(!strcmp(argv[1],"exp2")){
    FILE *f=fopen(argv[2],"a");
    int rates[]={10,30,50,100}; int cfg[][2]={{3,20},{3,10},{1,10}};
    for(int c=0;c<3;c++) for(int r=0;r<4;r++) run_exp2(&v,rates[r],cfg[c][0],cfg[c][1],atof(argv[3]),f);
    fclose(f);
  }
  free_vehicle(&v); return 0;
}
