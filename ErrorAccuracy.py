# -*- coding: utf-8 -*-
"""
Created on Wed May 12 15:36:05 2021

@author: 20181933
"""
import matplotlib.pyplot as plt
import numpy as np

data = np.genfromtxt("C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/MainProgram/build/Output/Plot/ErrorAccuracy.csv", delimiter=";", names=["Epoch", "Error", "Accuracy"])

fig, (ax1,ax2) = plt.subplots(2, 1, sharey=False)

ax1.plot(data["Epoch"], data["Error"], 'red')
ax1.set_title("Error for each Epoch.")
ax1.set_xlabel("Epoch")
ax1.set_ylabel("Error")

ax2.plot(data["Epoch"], data["Accuracy"], 'green')
ax2.set_title("Accuracy for each Epoch.")
ax2.set_xlabel("Epoch")
ax2.set_ylabel("Accuracy")

fig.tight_layout()
plt.show()
fig.savefig("C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/MainProgram/build/Output/Plot/ErrorAccuracy.pdf")

