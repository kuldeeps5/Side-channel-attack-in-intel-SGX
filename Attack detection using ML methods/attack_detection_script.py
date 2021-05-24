import os
import csv
import pandas as pd 
from sklearn.metrics import confusion_matrix
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.ensemble import AdaBoostClassifier
from sklearn.tree import DecisionTreeClassifier
from sklearn import svm
from sklearn.naive_bayes import GaussianNB
from sklearn.neural_network import MLPClassifier


#-------- creating dataset --------------#
finalList_Attack = []
finalList_NotAttack = []
attack_no_attack = False
headers =  []
for filename in os.listdir("attack_no_attack_dataset"):
    if filename[0] == 'A':
        attack_no_attack = True
    else:
        attack_no_attack = False
    with open('attack_no_attack_dataset/'+filename, 'r') as file:
        reader = csv.reader(file)
        c = 0
        if attack_no_attack == True:
            for row in reader:
                c = c+1
                if c == 1:
                    if len(headers) == 0:
                        headers.append(row)
                    continue
                row.pop()
                finalList_Attack.append(row)
        else:
            for row in reader:
                c = c+1
                if c == 1:
                    continue
                row.pop()
                finalList_NotAttack.append(row)       
for i in finalList_Attack:
    i.append(1)
for i in finalList_NotAttack:
    i.append(0)
headers[0].append('Prediction')
with open('finalFile.csv', 'w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(headers)
with open('finalFile.csv', 'a', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(finalList_Attack)
with open('finalFile.csv', 'a', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(finalList_NotAttack)
    
    
# Importing the dataset
dataset = pd.read_csv("finalFile.csv") 
X = dataset.iloc[:, 0:-1].values
y = dataset.iloc[:, -1].values
# Splitting the dataset into the Training set and Test set
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size = 0.20, random_state = 5)


def calculateAccuracy(actual, predicted):
    # Making the Confusion Matrix
    cm = confusion_matrix(y_test, y_pred)
    TP = cm[0][0]
    TN = cm[1][1]
    FN = cm[0][1]
    FP = cm[1][0]
    accuracy = (TP + TN)/(TP+TN+FN+FP)
    precision= TP / (TP + FP)
    recall= TP / (TP + FN)
    f1_Score= (2 * precision * recall) / (precision + recall)
    print("accuracy : ",accuracy*100)
    print("precision : ",precision*100)
    print("recall : ",recall*100)
    print("F1 Score : ",f1_Score*100)
    

#---------- RANDOM FOREST CLASSIFICATION --------------#
rfc = RandomForestClassifier(n_estimators=10,criterion='gini',random_state =0, min_impurity_split=1e-7)
rfc.fit(X,y)

# Predicting the random forest classifier Test set results
y_pred = rfc.predict(X_test)

calculateAccuracy(y_test,y_pred)



#---------- ADABOOST CLASSIFICATION --------------#
abc = AdaBoostClassifier(base_estimator=DecisionTreeClassifier(), 
                         algorithm='SAMME.R', n_estimators=50,learning_rate=1)
abmodel = abc.fit(X_train, y_train)

# Predicting the AdaBoost classifier Test set results
y_pred = abmodel.predict(X_test)

calculateAccuracy(y_test,y_pred)



#--------- C Support Vector Machine ------------#
csvm = svm.SVC(C=1.93,kernel='rbf',max_iter=-1, tol=1e-3,gamma=0.125)
csvm.fit(X_train, y_train)

# Predicting the C SVM classifier Test set results
y_pred = csvm.predict(X_test)

calculateAccuracy(y_test,y_pred)

            

#---------GAUSSIAN NAIVE BASED-----------------#
gnb = GaussianNB()
gnb.fit(X_train,y_train)

# Predicting the Gaussian naive based classifier Test set results
y_pred = gnb.predict(X_test)

calculateAccuracy(y_test,y_pred)



#--------- L Support Vector Machine ------------#
csvm = svm.SVC(C=1,max_iter=1000, tol=1e-4)
csvm.fit(X_train, y_train)

# Predicting the L SVM classifier Test set results
y_pred = csvm.predict(X_test)

calculateAccuracy(y_test,y_pred)



#-------- MULTILAYER PERCEPTRON----------------#
mlp = MLPClassifier(activation='relu', solver='adam', learning_rate='constant', beta_1=0.9, beta_2=0.009)
mlp.fit(X_train,y_train)

# Predicting the Multilayer perceptron classifier Test set results
y_pred = mlp.predict(X_test)

calculateAccuracy(y_test,y_pred)












