// ACCESS 1000 ARRAY SLOTS 10 TIMES
// CONSIDER 4 CASES, 1. SAME PROGRAM , 2. SAME LOGICAL CORE , 
                  // 3. SAME PHYSICAL CORE , 4. DIFFERENT PHYSICAL CORE


/* utility headers */
#include "debug.h"
#include "cacheutils.h"
#include<stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <stdint.h>
#include<sched.h>

/*PAPI headers*/
#include <papi.h>

/* SGX untrusted runtime */
#include <sgx_urts.h>
#include "Enclave/encl_u.h"

#define NUM_SAMPLES         5
#define NUM_SLOTS           1000
#define SLOT_SIZE           0x1000
#define ARRAY_LEN           (NUM_SLOTS*SLOT_SIZE)
#define GET_SLOT(k)         (array[k*SLOT_SIZE])
char array[ARRAY_LEN];

int solution[8][50*NUM_SLOTS];

sgx_enclave_id_t eid;

sgx_enclave_id_t create_enclave(void)
{
    sgx_launch_token_t token = {0};
    int updated = 0;
    sgx_enclave_id_t eid = -1;

    info_event("Creating enclave...");
    SGX_ASSERT( sgx_create_enclave( "./Enclave/encl.so", /*debug=*/1,
                                    &token, &updated, &eid, NULL ) );

    return eid;
}

//------------------------------------------------------
int event_set = PAPI_NULL;
long long values[2] = {0,0};
int ret;
FILE *pfile;

void foo() {
  int c = 0;
  for(int i= 0; i < 1e7; ++i) {
    c = (c+i)%10000;
  }
  printf("Foo's output is %d\n",c);
}

void handle_error(int line) {
  printf("Something wrong happened on line %d\n", line);
  exit(1);
}
//------------------------------------------------------

int compare(const void * a, const void * b) {
   return ( *(uint64_t*)a - *(uint64_t*)b );
}

int tsc[NUM_SLOTS][NUM_SAMPLES];

int cached_var = 0;
int currCore = 0;

int set_affinity(long thread_id, int cpu_id){
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id,&mask);
    pthread_setaffinity_np(thread_id,sizeof(cpu_set_t),&mask);
}

void *cached_in_core(){
    set_affinity(pthread_self(),currCore);
    SGX_ASSERT( ecall_secret_lookup(eid, array, ARRAY_LEN) );
    for(int i=0;i<100;i++){
        flush(&i);
        cached_var += i;
        i=i+(i%2);
    }
}

void compute_intensive_loop(int max_i){
    long x, y;
    x = 5;
    y = 6;
    for (int i = 0; i < max_i; ++i){
        // Flush so the code doesn't get optimized out
        flush(&max_i);
        x *= y;
        y *=x;
        if(i%7==0){
            x = y/5.2;
            y = x*0.81;
        }
        else if(i%20==0){
            x = y%13;
            y = x%6;
        }
        else{
            x = y/(x+1);
            y = 4*x/(7+3*x);
        }
    }
}

void attacker(int c_OR_nc,int k, int counter){
    pfile = fopen("Output/papi_reading.txt","a");
    if(currCore == -1){     // Same program
        for (int i=0; i < NUM_SAMPLES; i++)
        {
            /* 1. Flush */
            // flush(&array[0]);
            for (int j=0; j < NUM_SLOTS; j++)
                flush(&GET_SLOT(j));

            /* 2. Victim exec */
            if(c_OR_nc == 1){
                SGX_ASSERT( ecall_secret_lookup(eid, array, ARRAY_LEN) );
            }
            else {
                compute_intensive_loop(400);
            }

            // ----------------------------------------
            ret = PAPI_start(event_set);
            if(ret != PAPI_OK)  handle_error(__LINE__);

            /* 3. Reload */
            for (int j=0; j < NUM_SLOTS; j++)
                tsc[j][i] = reload(&GET_SLOT(j));

            ret = PAPI_stop(event_set, values);
            if(ret != PAPI_OK) handle_error(__LINE__);
            if(c_OR_nc == 1)
                fprintf(pfile,"HIT => %d -- %d -- Level1 DCM %lld -- Level3 %lld\n", k, counter, values[0], values[1]);
            else
                fprintf(pfile,"MISS => %d -- %d -- Level1 DCM %lld -- Level3 %lld\n", k, counter, values[0], values[1]);
            // ----------------------------------------
   
        }
    }
    else{
        for (int i=0; i < NUM_SAMPLES; i++)
        {
            /* 1. Flush */
            for (int j=0; j < NUM_SLOTS; j++)
                flush(&GET_SLOT(j));

            /* 2. Victim exec */
            if(c_OR_nc == 1){
                pthread_t t[1];
                pthread_create(&t[0],NULL,cached_in_core,NULL);
                compute_intensive_loop(400);
                pthread_join(t[0],NULL);
            } else {
                compute_intensive_loop(400);
            }

            // ----------------------------------------
            ret = PAPI_start(event_set);
            if(ret != PAPI_OK)  handle_error(__LINE__);

            /* 3. Reload */
            for (int j=0; j < NUM_SLOTS; j++)
                tsc[j][i] = reload(&GET_SLOT(j));
                
            ret = PAPI_stop(event_set, values);
            if(ret != PAPI_OK) handle_error(__LINE__);
            if(c_OR_nc == 1)
                fprintf(pfile,"HIT => %d -- %d -- Level1 DCM %lld -- Level3 %lld\n", k, counter, values[0], values[1]);
            else
                fprintf(pfile,"MISS => %d -- %d -- Level1 DCM %lld -- Level3 %lld\n", k, counter, values[0], values[1]);
            // ----------------------------------------
        }
    }
}

int main( int argc, char **argv )
{
    eid = create_enclave();
    int rv = 1, secret = 0;
    int i, j, med;

    /* Ensure array pages are mapped in */
    for (i=0; i < ARRAY_LEN; i++)
        array[i] = 0x00;
    
    /* ---------------------------------------------------------------------- */
    info_event("calling enclave...");
    /* =========================== START SOLUTION =========================== */

    int currRow = 0;
    pfile = fopen("Output/papi_reading.txt","w");
    fclose(pfile);

    /* -------------------- PAPI INITIALISATION -----------------------*/

    ret = PAPI_library_init(PAPI_VER_CURRENT);
    if(ret != PAPI_VER_CURRENT) handle_error(__LINE__);

    ret = PAPI_create_eventset(&event_set);
    if(ret != PAPI_OK) handle_error(__LINE__);

    ret = PAPI_add_event(event_set, PAPI_L1_DCM);
    if(ret != PAPI_OK)  handle_error(__LINE__);

    ret = PAPI_add_event(event_set, PAPI_L3_TCM);
    if(ret != PAPI_OK)  handle_error(__LINE__);

    /* -------------------- PAPI INITIALISATION END-----------------------*/
  
    for(int k=0;k<4;k++){

        if(k==0){
            printf("Same program\n");
            currCore = -1;
        }
        else if(k==1){
            printf("Same Logical Core\n");
            currCore = 0;
        }
        else if(k==2){
            printf("Same Physical Core\n");
            currCore = sysconf(_SC_NPROCESSORS_CONF)/2;  //  0,8 map to CPU 0.   1,9 map to CPU 1 and so on....
        }
        else if(k==3){
            printf("Different Physical Core\n");
            //currCore = sysconf(_SC_NPROCESSORS_CONF)/4;
	    currCore = 3;  
        }

        sleep(5);
        set_affinity(pthread_self(),0);
        int begin = 0;
        for(int counter = 0;counter<50;counter++){
            // Setting readings to 0 initially
            for (int j=0; j < NUM_SLOTS; j++)
                for (int i=0; i < NUM_SAMPLES; i++)
                    tsc[j][i] = 0;

            // Allow victim to access array
            attacker(1,k,counter);

            // Taking median of Samples for each slot
            for (int j=0; j < NUM_SLOTS; j++)
            {
                qsort(tsc[j], NUM_SAMPLES, sizeof(int), compare);
                med = tsc[j][NUM_SAMPLES/2];
                solution[currRow][begin] = med;
                begin++;
            } 
        }

        currRow++;
        sleep(5);
        begin = 0;
        for(int counter = 0;counter<50;counter++){
            // Setting readings to 0 initially
            for (int j=0; j < NUM_SLOTS; j++)
                for (int i=0; i < NUM_SAMPLES; i++)
                    tsc[j][i] = 0;

            // Flush all the slots of array
            for (int j=0; j < NUM_SLOTS; j++)
                flush(&GET_SLOT(j));

             // Allow victim to access array   
            attacker(0,k,counter);

            // Taking median of Samples for each slot
            for (int j=0; j < NUM_SLOTS; j++)
            {
                qsort(tsc[j], NUM_SAMPLES, sizeof(int), compare);
                med = tsc[j][NUM_SAMPLES/2];
                solution[currRow][begin] = med;
                begin++;
            } 
        }
        currRow++;
	    printf("Done Case %d\n",k);
    }

    FILE *fptr1 = fopen("Output/reading_sameProgram.txt","w");
    for(int i=0;i<NUM_SLOTS*50;i++){
	    //printf("%d -> %d hit cycle %d miss cycle\n",i,solution[0][i],solution[1][i]);
	    fprintf(fptr1,"%d,%d\n",solution[0][i],solution[1][i]);
    }
    fclose(fptr1);
    FILE *fptr2 = fopen("Output/reading_sameLogical.txt","w");
    for(int i=0;i<NUM_SLOTS*50;i++){
	    //printf("%d -> %d hit cycle %d miss cycle\n",i,solution[0][i],solution[1][i]);
	    fprintf(fptr2,"%d,%d\n",solution[2][i],solution[3][i]);
    }
    fclose(fptr2);
    FILE *fptr3 = fopen("Output/reading_samePhysical.txt","w");
    for(int i=0;i<NUM_SLOTS*50;i++){
	    //printf("%d -> %d hit cycle %d miss cycle\n",i,solution[0][i],solution[1][i]);
	    fprintf(fptr3,"%d,%d\n",solution[4][i],solution[5][i]);
    }
    fclose(fptr3);
    FILE *fptr4 = fopen("Output/reading_diffPhysical.txt","w");
    for(int i=0;i<NUM_SLOTS*50;i++){
	    //printf("%d -> %d hit cycle %d miss cycle\n",i,solution[0][i],solution[1][i]);
	    fprintf(fptr4,"%d,%d\n",solution[6][i],solution[7][i]);
    }
    fclose(fptr4);
    /* ---------------------------------------------------------------------- */
    info_event("destroying SGX enclave");
    SGX_ASSERT( sgx_destroy_enclave( eid ) );

    info("all is well; exiting..");
	return 0;
}
