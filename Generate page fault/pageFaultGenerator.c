// Code is used to generate huge amound of page fault.
// Soruce : https://stackoverflow.com/questions/37825859/cache-miss-a-tlb-miss-and-page-fault

//////// Code 1

#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>

int main(){
     double* pBigArray = (double*)malloc(sizeof(double) * 536870912 *4); 
     double lfBigChecksum = 0.0;
     int BUFFER_SIZE = sizeof(pBigArray)/sizeof(double);
     for(long long int i=0;i<99999;i++)
     {
         int iIndex = rand() % BUFFER_SIZE;
         lfBigChecksum += pBigArray[iIndex];
 	printf("running......\n");
     }
     return 0;
}*/

    
//////// Code 2    
    
//#define NUM_SLOTS           1000
//#define SLOT_SIZE           0x1000
//#define ARRAY_LEN           (NUM_SLOTS*SLOT_SIZE)
//#define GET_SLOT(k)         (array[k*SLOT_SIZE])
//char array[ARRAY_LEN];
//char accessLocation;
//
//int main(){
//    for(int i=0;i<100000;i++){
//        int iIndex = rand() % 1000;
//	printf("%d -- %d\n",i,iIndex);
//        accessLocation = GET_SLOT(iIndex);
//    }
//}


