# Flush reload attack in the presence of intel SGX with various stress levels.

1. In this repository we performed flush reload attack in the presence of intel SGX environment.

Direction to run.
1. Run makefile using 'make' command.
2. It will generate the txt file. 

3. To run code using different stress level use the command below in another terminal :
a) stress-ng –cpu N, where N tells the system to start N workers spinning on sqrt(rand()).  // For CPU stress.
b) stress-ng –io N, where N tells the system to start N workers spinning on sync(). // For IO stress
c) stress-ng –vm N –vm-bytes M, where N tells the system to start N workers spinning on anonymous mmap and allocate M bytes per vm worker (default 256MB).  // For VM stress.

4. Sample outputs are present in PlotValues folder.