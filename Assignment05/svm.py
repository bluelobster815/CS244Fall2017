# -*- coding: utf-8 -*-
"""
Created on Wed Nov  8 11:41:28 2017

@author: zero
"""

from sklearn import svm, datasets
from sklearn.model_selection import GridSearchCV

import numpy as np

#--------------------------------

def processData(indata):
    sdata = np.hsplit(indata, np.array([1,4,5,8,9,12,13,16,17,20]))
    
    timestamps = sdata[0]
#    print(timestamps)

    odata1 = sdata[1]
    otdata = sdata[2]
    odata2 = (odata1*odata1).sum(axis=1)**0.5
    for i in range(1,5):
#        print(i)
        odata1 = np.vstack((odata1,sdata[2*i+1]))
        otdata = np.vstack((otdata,sdata[2*i+2]))
        dd = (sdata[2*i+1]*sdata[2*i+1]).sum(axis=1)**0.5
        odata2 = np.concatenate((odata2,dd))

    otdata = np.ravel(otdata)
    odata2 = odata2.reshape(-1,1)

    return odata1, odata2, otdata, timestamps

#-------------------------------

indata = np.genfromtxt('indata2.csv',skip_header=1,delimiter=',')
#print(indata)
#print(indata.shape)

n_data = indata.shape[0]
#print("num total data samples:",n_data*5)
n_trdata = (n_data*2)//3
#print("num training data samples:",n_trdata*5)

trdata = indata[:n_trdata]
#print(trdata.shape)

ttdata = indata[n_trdata:]
#print(ttdata.shape)

trdata1,trdata2,trtargetdata, ts = processData(trdata)
ttdata1,ttdata2,tttargetdata, ts2 = processData(ttdata)

#print(trdata1,trdata2,trtargetdata)
'''
classifier1 = svm.SVC()
print(classifier1)
classifier1.fit(trdata1,trtargetdata)

ttdata1,ttdata2,tttargetdata, ts2 = processData(ttdata)

predictdata = classifier1.predict(ttdata1)
#print("predictdata",predictdata)

#print("otd shape", predictdata.shape)
#print("ttd shape", np.ravel(tttargetdata).shape)
n_ttdata = tttargetdata.shape[0]
error1 = (np.abs(predictdata-tttargetdata) > 0.001).sum()
#print(error1)
error1 = error1/n_ttdata
print("error:",error1)
error1 = (error1)*100
print("error:",error1,"%")
print('accuracy:', 100-error1, '%')
'''

svc = svm.SVC()

param_grid = [
  {'C': [1, 10, 100, 1000], 'kernel': ['linear']},
  {'C': [1, 10, 100, 1000], 'gamma': [0.001, 0.0001], 'kernel': ['rbf']},
 ]

'''
param_grid = [
  {'C': [1, 10, 100], 'gamma': [0.001, 0.0001], 'kernel': ['rbf']},
 ]
'''
clf = GridSearchCV(svc, param_grid)
print(clf)
clf.fit(trdata1, trtargetdata)
print(sorted(clf.cv_results_.keys()))

scoreval = clf.score(ttdata1,tttargetdata)
print("score:", scoreval)
print('accuracy:', scoreval*100, '%')

