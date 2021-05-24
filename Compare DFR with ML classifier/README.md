# This directory contain comparision of accuracy of calibration code between dabanng classifier and decision tree classifier.

1. calibration Code folder contains the 'calibration code' for DFR and DFF code. After running the script csv files are generated. 
2. These csv files are taken as input in 'Dabanng calculate accuracy code' fr_display_stable_pair.py from which threshold ranges are found.
3. For calculating the accuracy dab_Cal_Acc.py is used, which gives the accuracy of dabanng classifier.
4. For calculating the accuracy using decision tree classifier, the csv file is given as input to ML_method.py present in 'Decision Tree calculate accuracy code' folder.
5. Sample generated files are present in the folder 'Generated file'.
6. If you want to calculate accuracy under different stress then in another terminal run the CPU,IO, or VM stress. 