#define _GNU_SOURCE
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <stdint.h>
#include<sched.h>
#include "./cacheutils.h"

#define ITERATIONS (200000)

long long count = 0;
int cached_var = 0;
int non_cached_var = 0;
int cached_delta_arr[ITERATIONS];
int non_cached_delta_arr[ITERATIONS];
int core_id = 0;

int set_affinity(long thread_id, int cpu_id){
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id,&mask);
    pthread_setaffinity_np(thread_id,sizeof(cpu_set_t),&mask);
}

void *cached_in_core(){
    set_affinity(pthread_self(),core_id);
    cached_var = cached_var%10;
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

int measure_access_latency(void* addr){
    int delta;
    asm volatile(
        " mfence           \n\t"
        " lfence           \n\t"
        " rdtsc            \n\t"
        " lfence           \n\t"
        " mov %%rax, %%rdi \n\t"
        " mov (%1), %%r8   \n\t"
        " lfence           \n\t"
        " rdtsc            \n\t"
        " sub %%rdi, %%rax \n\t"
        : "=a"(delta) /*output*/
        : "c"(addr)
        : "r8", "rdi");
    return delta;
}

int attacker(void* addr, int cached)
{
    flush(addr);

    if(core_id==-1 && cached){
        measure_access_latency(addr);
        compute_intensive_loop(400);
    }
    else if(cached){
        pthread_t t[1];
        pthread_create(&t[0],NULL,cached_in_core,NULL);
        compute_intensive_loop(400);
        pthread_join(t[0],NULL);
    }
    else
        compute_intensive_loop(400);

    size_t delta = measure_access_latency(addr);

    if (delta >= 0 && delta <= 1000 && count < ITERATIONS)
    {
	    if(cached)
            cached_delta_arr[count] = delta;
        else
            non_cached_delta_arr[count] = delta;
        count++;
    }
    else if(count >= ITERATIONS)
        return 1;
   return 0;  
}

int main()
{
    void* addr1;
    addr1 = &cached_var;
    void* addr2;
    addr2 = &non_cached_var;
    int fd = open("fr_latency_freq.csv",O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    char str[100];

    int core_id_arr[4] = {-1, 0, sysconf(_SC_NPROCESSORS_CONF)/2, sysconf(_SC_NPROCESSORS_CONF)/4};
    printf("%d -- %d -- %d --%d\n",core_id_arr[0],core_id_arr[1],core_id_arr[2],core_id_arr[3]);
    char test_type[4][100] = {"Same program","Same logical core","Same physical core","Different physical cores"};

    for(int i = 0; i < 4; i++){
        sleep(5);
        
        set_affinity(pthread_self(),0);
        core_id = core_id_arr[i];
        count = 0;
        size_t time_remaining = rdtsc();
        while(1)
            if(attacker(addr1, 1))  break;
        count = 0;
        time_remaining = rdtsc() - time_remaining;
        sleep(5);
        while(1)
            if(attacker(addr2, 0))  break;
        while(count>0){
            sprintf(str,"%lld,%d,%d\n",ITERATIONS-count+1,cached_delta_arr[ITERATIONS-count],non_cached_delta_arr[ITERATIONS-count]);
            if(write(fd,str,strlen(str))<0)
                printf("Write error!\n");
            count--;
        }
        printf("Test type: %s, Status: done, please wait for approximately %d seconds.\n",test_type[i],(int)(((float)(time_remaining/3400000000)+5)*(6-2*i)+1));
    }
    close(fd);
    return 0;
}
