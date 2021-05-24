from sklearn.preprocessing import PolynomialFeatures
from sklearn.pipeline import make_pipeline
from sklearn.linear_model import LinearRegression, LogisticRegression
import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeClassifier
from sklearn.svm import SVC

from collections import deque
from sklearn.tree import _tree as ctree
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import sys
from collections import defaultdict
import math

class AABB:
    """Axis-aligned bounding box"""
    def __init__(self, n_features):
        self.limits = np.array([[-np.inf, np.inf]] * n_features)

    def split(self, f, v):
        left = AABB(self.limits.shape[0])
        right = AABB(self.limits.shape[0])
        left.limits = self.limits.copy()
        right.limits = self.limits.copy()

        left.limits[f, 1] = v
        right.limits[f, 0] = v

        return left, right


def tree_bounds(tree, n_features=None):
    """Compute final decision rule for each node in tree"""
    if n_features is None:
        n_features = np.max(tree.feature) + 1
    aabbs = [AABB(n_features) for _ in range(tree.node_count)]
    queue = deque([0])
    while queue:
        i = queue.pop()
        l = tree.children_left[i]
        r = tree.children_right[i]
        if l != ctree.TREE_LEAF:
            aabbs[l], aabbs[r] = aabbs[i].split(tree.feature[i], tree.threshold[i])
            queue.extend([l, r])
    return aabbs


def decision_areas(tree_classifier, maxrange, x=0, y=1, n_features=None):
    """ Extract decision areas.

    tree_classifier: Instance of a sklearn.tree.DecisionTreeClassifier
    maxrange: values to insert for [left, right, top, bottom] if the interval is open (+/-inf)
    x: index of the feature that goes on the x axis
    y: index of the feature that goes on the y axis
    n_features: override autodetection of number of features
    """
    tree = tree_classifier.tree_
    aabbs = tree_bounds(tree, n_features)

    rectangles = []
    for i in range(len(aabbs)):
        if tree.children_left[i] != ctree.TREE_LEAF:
            continue
        l = aabbs[i].limits
        r = [l[x, 0], l[x, 1], np.argmax(tree.value[i])]
        rectangles.append(r)
    rectangles = np.array(rectangles)
    return rectangles

def plot_areas(rectangles):
    for rect in rectangles:
        if rect[2] == 0:
            plt.axvline(x=rect[0], color='r')
            plt.axvline(x=rect[1], color='b')
    plt.yticks([0,1],['positive','negative'])
    #plt.show()

fname = sys.argv[1]
data = pd.read_csv(fname, header=None)
dtc = DecisionTreeClassifier()
X1 = list(data[1])
X2 = list(data[2])
X = np.array(X1+X2).reshape(-1,1)
y1 = list(np.zeros(len(X1)))
y2 = list(np.ones(len(X2)))

y = np.array(y1+y2)
new_X = X
dtc.fit(new_X, y)
rectangles = decision_areas(dtc, [-np.inf, np.inf, np.inf, -np.inf], x=0, y=1)

rectangles = sorted(rectangles, key=lambda x: x[0])
if rectangles[0][0] == -np.inf:
    rectangles[0][0] = 0
if rectangles[-1][1] == np.inf:
    rectangles[-1][1] = np.max(X) + 1
new_rectangles = []
new_rectangles.append(rectangles[0])
i = 1
while i < len(rectangles):
    curr = rectangles[i]
    if curr[2] == new_rectangles[-1][2] and curr[0] == new_rectangles[-1][1]:
        new_rectangles[-1][1] = curr[1]
    else:
        new_rectangles.append(curr)
    i += 1

#print(new_rectangles)
#print(len(rectangles), len(new_rectangles))
plt.figure()
plt.xlabel("Cycles")
plt.ylabel("Class")
# plt.scatter(X,y)
# plot_areas(new_rectangles)

new_rectangles = np.array(new_rectangles)
new_rectangles_tmp = new_rectangles
tot = [0] * new_rectangles_tmp.shape[0]
col2_range = [0] * new_rectangles_tmp.shape[0]
col3_range = [0] * new_rectangles_tmp.shape[0]

for j in range(X.shape[0]):
    x = X[j][0]
    for i in range(new_rectangles_tmp.shape[0]):
        if x >= new_rectangles_tmp[i][0] and x <= new_rectangles_tmp[i][1]:
            tot[i] += 1
            if y[j] == 1:
                col3_range[i] += 1
            else:
                col2_range[i] += 1




def mergeOverlapingandSmallIntervals(new_rectangles, tot, col2_range, col3_range):

    i = 1
    curr = new_rectangles[0]
    curr_tot_count = tot[0]
    curr_neg_count = col3_range[0]
    curr_pos_count = col2_range[0]
    new_rectangles_tmp = [curr]
    new_tot = [curr_tot_count]
    new_col2_range = [curr_pos_count]
    new_col3_range = [curr_neg_count]
    while i < new_rectangles.shape[0]:
        curr = new_rectangles[i]
        if curr[2] == new_rectangles_tmp[-1][2] and curr[0] == new_rectangles_tmp[-1][1]:
            new_rectangles_tmp[-1][1] = curr[1]
            curr_tot_count += tot[i]
            curr_pos_count += col2_range[i]
            curr_neg_count += col3_range[i]
            new_tot[-1] = curr_tot_count
            new_col2_range[-1] = curr_pos_count
            new_col3_range[-1] = curr_neg_count
            if curr_pos_count > curr_neg_count:
                new_rectangles_tmp[-1][2] = 0
            else:
                new_rectangles_tmp[-1][2] = 1
        elif curr[0] == new_rectangles_tmp[-1][1] and curr_tot_count < 10:
            new_rectangles_tmp[-1][1] = curr[1]
            curr_tot_count += tot[i]
            curr_pos_count += col2_range[i]
            curr_neg_count += col3_range[i]
            new_tot[-1] = curr_tot_count
            new_col2_range[-1] = curr_pos_count
            new_col3_range[-1] = curr_neg_count
            if curr_pos_count > curr_neg_count:
                new_rectangles_tmp[-1][2] = 0
            else:
                new_rectangles_tmp[-1][2] = 1
        else:
            new_rectangles_tmp.append(curr)

            curr_tot_count = tot[i]
            curr_pos_count = col2_range[i]
            curr_neg_count = col3_range[i]
            new_tot.append(curr_tot_count)
            new_col2_range.append(curr_pos_count)
            new_col3_range.append(curr_neg_count)

        i += 1
    new_rectangles_tmp = np.array(new_rectangles_tmp)
    return new_rectangles_tmp, new_tot, new_col2_range, new_col3_range


new_rectangles_tmp, new_tot, new_col2_range, new_col3_range = \
    mergeOverlapingandSmallIntervals(new_rectangles, tot, col2_range, col3_range)

new_rectangles_tmp, new_tot, new_col2_range, new_col3_range = \
    mergeOverlapingandSmallIntervals(new_rectangles_tmp, new_tot, new_col2_range, new_col3_range)

tot = new_tot
col2_range = new_col2_range
col3_range = new_col3_range
TP = 0
TN = 0
FP = 0
FN = 0

data_from_file = pd.read_csv(fname, header=None)
originalRangeList = defaultdict(list)
for i in data_from_file[1]:
    blockNo = math.floor(int(i)/5)
    originalRangeList[blockNo].append(i)

modifiedOriginalRangeList = dict()
for k in originalRangeList:
    modifiedOriginalRangeList[k] = len(originalRangeList[k])

#print(modifiedOriginalRangeList)

rangeList = []
for i in new_rectangles_tmp:
    if (int)(i[2]) == 0:
        for i in range(int(i[0]),int(i[1]),5):
            block = math.floor(i/5)
            if block in modifiedOriginalRangeList:
                if(modifiedOriginalRangeList[block] >500):
                    rangeList.append(block)

print(rangeList)

                        

print("range: start - end - type: Positive, Negative, total")

for i in range(new_rectangles_tmp.shape[0]):
    print("range: {} - {} - {} : {} , {}, {}".format(new_rectangles_tmp[i][0], new_rectangles_tmp[i][1],
                                                    'Positive' if new_rectangles_tmp[i][2] == 0 else 'Negative',
                                                    col2_range[i], col3_range[i], tot[i]))
    if new_rectangles_tmp[i][2] == 0:
        # Positve
        TP += col2_range[i]
        FP += col3_range[i]
    else:
        # Negative
        TN += col3_range[i]
        FN += col2_range[i]

precision = TP/(TP+FP)
recall = TP/(TP+FN)
f1Score = 2 * (precision * recall) / (precision + recall)

print("TP = {}, TN = {}, FP = {}, FN = {}, Acc = {}, Pre = {}, Rec = {}, F1 Score = {}".format(TP, TN, FP, FN, (TP + TN)/ (TP + FP + TN + FN), TP/(TP+FP), TP/(TP+FN), f1Score))
# y_pred = dtc.predict(new_X)
# y_pred_thresh = np.zeros(y_pred.shape)
# y_pred_thresh[np.where(y_pred > 0.5)] = 1

pass
