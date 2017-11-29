from sklearn import svm
from sklearn.model_selection import cross_val_score
import numpy as np


d00 = np.genfromtxt('sleeping_features.csv',delimiter=',')
d01 = np.genfromtxt('sitting_features.csv',delimiter=',')
d02 = np.genfromtxt('standing_features.csv',delimiter=',')
d03 = np.genfromtxt('walking_features.csv',delimiter=',')


n00 = d00.shape[0]
n01 = d01.shape[0]
n02 = d02.shape[0]
n03 = d03.shape[0]

print(n00)
print(n01)
print(n02)
print(n03)

t00 = np.zeros(n00) + 1
t01 = np.zeros(n01) + 2
t02 = np.zeros(n02) + 3
t03 = np.zeros(n03) + 4


d10 = np.vstack((d00,d01))
d10 = np.vstack((d10,d02))
d10 = np.vstack((d10,d03))

print(d10)
print(d10.shape)

t10 = np.concatenate((t00,t01))
t10 = np.concatenate((t10,t02))
t10 = np.concatenate((t10,t03))

print(t10)
print(t10.shape)

from sklearn.utils import shuffle
d11, t11 = shuffle(d10, t10, random_state=0)

print(d11,t11)

clf = svm.SVC()
print(clf)
scores = cross_val_score(clf, d11, t11, cv=10)
print(scores)
print("Accuracy: %0.2f (+/- %0.2f)" % (scores.mean(), scores.std() * 2))


dt01 = np.genfromtxt('sitting_short_features.csv', delimiter=',')
dt02 = np.genfromtxt('standing_short_features.csv', delimiter=',')

clf2 = svm.SVC()
print(clf2)
clf2.fit(d11,t11)
zz01 = clf2.predict(dt01)
nzz01 = zz01.shape[0]
#print(zz01)
ac01 = (zz01 == 1.).sum()
print(ac01, nzz01)

zz02 = clf2.predict(dt02)
nzz02 = zz02.shape[0]
print(zz02)
ac02 = (zz02 == 3.).sum()
print(ac02, nzz02)
