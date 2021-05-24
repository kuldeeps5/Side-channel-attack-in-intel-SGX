# This repository contains the attack detection methods using various ML techniques.

**** Direction to run ***
A. For collection attack scenario data

1. Run flush based attack in the presence of attacker and victim.
2. Run script ./detection_script.sh in another terminal, which internall call formatter.py and store data in a csv file. 
3. Rename the CSV file accourding to the stress level you used, i.e. if you run CPU stress in the background then rename it accordingly and place it in attack_no_attack_dataset folder.
4. After completetion of the attack csv file is generated. Run attack_detection_script.py and calculate the accuracy of attack using various ML methods.