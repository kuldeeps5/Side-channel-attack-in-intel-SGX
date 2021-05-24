#include "encl_t.h"
#include <sgx_trts.h>
#include "secret.h"

volatile char c;

void ecall_secret_lookup(char *array, int len, int nn)
{
    /* First ensure the _untrusted_ pointer points outside the enclave */
    if (!sgx_is_outside_enclave(array, len))
        return;

    /* Now do the secret lookup */
    /*for(int i=0;i<20;i++){
	int currNo = rand()%1000;
	printf("%d<->",currNo);
	c = array[(4096*currNo) % len];
    }
    printf("\n");*/
    c = array[(4096*nn) % len];  // ---th location
    //c = array[(4096*(secret_idx*4))%len]; // 60th location
    //c = array[(4096*(secret_idx*6))%len];    // 90th location */
}
