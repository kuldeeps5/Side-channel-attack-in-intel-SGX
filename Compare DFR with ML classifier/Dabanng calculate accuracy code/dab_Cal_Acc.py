import sys

inpFile = sys.argv[1]

file = open (inpFile)
hitCycle = []
missCycle = []
startRange = [40,108]
endRange = [219,228]
for i in file:
    curr = i.split(',')
    hitCycle.append((int)(curr[1]))
    missCycle.append((int)(curr[2]))

tp = 0
tn = 0
fp = 0
fn = 0
for i in range(len(hitCycle)):
    check  = False
    for j in range(len(startRange)):
        if(hitCycle[i] >= startRange[j] and hitCycle[i] <= endRange[j]):
            tp = tp+1
            check = True
            break
    if check == False:
        fn = fn+1

for i in range(len(missCycle)):
    check  = True
    for j in range(len(startRange)):
        if(missCycle[i] >= startRange[j] and missCycle[i] <= endRange[j]):
            fp = fp+1
            check = False
            break
    if check == True:
        tn = tn+1
        
accuracy  = (tp + tn) / (tp+tn+fp+fn)
print(accuracy * 100)
