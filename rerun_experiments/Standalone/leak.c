/* Exp 5: LRS vs induced leak size, compared with Welch t (TVLA). Leak = benchmark's secret-bit busy-wait, loop count varied. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <oqs/oqs.h>
static double now_us(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1e6+t.tv_nsec/1e3;}
static OQS_SIG *sig; static uint8_t *pk,*sk,*sg; static const char *m="controlled_leakage_benchmark_test_message_v2v";
static double one(long L,int fresh){ size_t sl;
  if(fresh) OQS_SIG_keypair(sig,pk,sk);
  int bit=sk[0]&1; double t0=now_us(); OQS_SIG_sign(sig,sg,&sl,(uint8_t*)m,strlen(m),sk);
  if(bit){volatile long b=0;for(long j=0;j<L;j++)b+=j;} return now_us()-t0;}
static double cvof(double*x,int n){double s=0;for(int i=0;i<n;i++)s+=x[i];double mu=s/n,v=0;for(int i=0;i<n;i++)v+=(x[i]-mu)*(x[i]-mu);return sqrt(v/n)/mu;}
static int cmpd(const void*a,const void*b){double x=*(double*)a,y=*(double*)b;return (x>y)-(x<y);}
static double asra(long L,int fresh){double c[3],w[20];for(int r=0;r<3;r++){for(int i=0;i<20;i++)w[i]=one(L,fresh);c[r]=cvof(w,20);}qsort(c,3,sizeof(double),cmpd);return c[1];}
int main(int argc,char**argv){
  int N=atoi(argv[1]), E=atoi(argv[2]); long Ls[]={0,500,1000,2500,5000,10000,25000,50000,100000}; int nL=9;
  OQS_init(); sig=OQS_SIG_new(OQS_SIG_alg_ml_dsa_65); pk=malloc(sig->length_public_key); sk=malloc(sig->length_secret_key); sg=malloc(sig->length_signature);
  double *base=malloc(N*sizeof(double)),*x=malloc(N*sizeof(double));
  for(int i=0;i<N;i++) base[i]=one(0,1);
  double mb=0,vb=0; for(int i=0;i<N;i++)mb+=base[i]; mb/=N; for(int i=0;i<N;i++)vb+=(base[i]-mb)*(base[i]-mb); vb/=(N-1);
  /* fixed key with secret bit = 1 (worst case for a fixed-key leak) */
  uint8_t *skf=malloc(sig->length_secret_key),*pkf=malloc(sig->length_public_key); do{OQS_SIG_keypair(sig,pkf,skf);}while(!(skf[0]&1));
  printf("L,delay_us,mean_us,welch_t,cv_all,lrs_fresh_mean,lrs_fresh_sd,rej_fresh_pct,lrs_fixed_mean,lrs_fixed_sd,rej_fixed_pct\n");
  for(int k=0;k<nL;k++){ long L=Ls[k];
    double d0=now_us(); for(int r=0;r<200;r++){volatile long b=0;for(long j=0;j<L;j++)b+=j;} double dly=(now_us()-d0)/200;
    for(int i=0;i<N;i++){ base[i]=one(0,1); x[i]=one(L,1);} /* interleaved */
    mb=0;vb=0; for(int i=0;i<N;i++)mb+=base[i]; mb/=N; for(int i=0;i<N;i++)vb+=(base[i]-mb)*(base[i]-mb); vb/=(N-1);
    double mx=0,vx=0; for(int i=0;i<N;i++)mx+=x[i]; mx/=N; for(int i=0;i<N;i++)vx+=(x[i]-mx)*(x[i]-mx); vx/=(N-1);
    double t=(mx-mb)/sqrt(vb/N+vx/N);
    double sf=0,sf2=0,sx=0,sx2=0; int rf=0,rx=0;
    for(int e=0;e<E;e++){double l=fmin(asra(L,1)/0.8345,1); sf+=l; sf2+=l*l; rf+=l>0.7;}
    memcpy(sk,skf,sig->length_secret_key);
    for(int e=0;e<E;e++){double l=fmin(asra(L,0)/0.8345,1); sx+=l; sx2+=l*l; rx+=l>0.7;}
    double mf=sf/E, mxk=sx/E;
    printf("%ld,%.1f,%.1f,%.2f,%.4f,%.4f,%.4f,%.1f,%.4f,%.4f,%.1f\n",L,dly,mx,t,sqrt(vx)/mx,mf,sqrt(fmax(sf2/E-mf*mf,0)),100.0*rf/E,mxk,sqrt(fmax(sx2/E-mxk*mxk,0)),100.0*rx/E);
    fflush(stdout);
  }
  return 0;}
