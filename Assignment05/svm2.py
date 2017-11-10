# -*- coding: utf-8 -*-
"""
Created on Wed Nov  8 15:38:13 2017

@author: zero
"""

from sklearn import svm
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
#n_trdata = (n_data*2)//3
#print("num training data samples:",n_trdata*5)
#trdata = indata[:n_trdata]
#print(trdata.shape)

#ttdata = indata[n_trdata:]
#print(ttdata.shape)
n_trdata = (n_data*1)//3
ttdata = indata[:n_trdata]
trdata = indata[n_trdata:]

trdata1,trdata2,trtargetdata, ts = processData(trdata)

#print(trdata1,trdata2,trtargetdata)

#classifier1 = svm.SVC()
classifier1 = svm.SVC(C=100, gamma=0.1)
print(classifier1)
classifier1.fit(trdata1,trtargetdata)

ttdata1,ttdata2,tttargetdata, ts2 = processData(ttdata)

'''
predictdata = classifier1.predict(ttdata1)
#print("predictdata",predictdata)

n_ttdata = tttargetdata.shape[0]
error1 = (np.abs(predictdata-tttargetdata) > 0.001).sum()

error1 = error1/n_ttdata
print("error:",error1)
error1 = (error1)*100
print("error:",error1,"%")
print('accuracy:', 100-error1, '%')
'''

scoreval = classifier1.score(ttdata1,tttargetdata)
print("score:", scoreval)
print('accuracy:', scoreval*100, '%')
