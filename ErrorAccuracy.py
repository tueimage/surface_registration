# -*- coding: utf-8 -*-
"""
Created on Wed May 12 15:36:05 2021

@author: 20181933
"""
import matplotlib.pyplot as plt
import numpy as np

data = np.genfromtxt("D:/BEP_Results/Rings 1/Output/Plot/ErrorAccuracy.csv", delimiter=";", names=["Epoch", "Error", "Accuracy"])

fig, (ax1,ax2) = plt.subplots(2, 1, figsize=(5,3), dpi=300, sharey=False)

ax1.plot(data["Epoch"], data["Error"], 'r')
ax1.set_title("Error for each Epoch.")
ax1.set_xlabel("Epoch")
ax1.set_ylabel("Error")
ax1.set_ylim(ymin = 0)
ax1.locator_params(axis="y", nbins = 6)

ax2.plot(data["Epoch"], data["Accuracy"], 'g')
ax2.set_title("Accuracy for each Epoch.")
ax2.set_xlabel("Epoch")
ax2.set_ylabel("Accuracy")
ax2.set_ylim(ymax=1)
ax2.locator_params(axis="y", nbins = 6)

fig.tight_layout()
plt.show()
fig.savefig("D:/BEP_Results/Rings 1/Output/Plot/ErrorAccuracy.pdf")

