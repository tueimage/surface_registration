# -*- coding: utf-8 -*-
"""
Created on Wed Apr 28 09:44:41 2021

@author: 20181933
"""

import numpy as np
import nibabel as nb

img_handle = nb.load("C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/BEPdata/133019/133019_wmseg.nii")
img = img_handle.get_fdata()
brain_part = np.zeros(img.shape)
brain_part[img==4025] = 1
new_img_handle = nb.Nifti1Image(brain_part, img_handle.affine)
new_path = "C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/Segmentations/Patient1Seg5"
nb.save(new_img_handle, new_path)


img_handle2 = nb.load("C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/BEPdata/133625/133625_wmseg.nii")
img2 = img_handle2.get_fdata()
brain_part2 = np.zeros(img2.shape)
brain_part2[img2==4025] = 1
new_img_handle2 = nb.Nifti1Image(brain_part2, img_handle2.affine)
new_path2 = "C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/Segmentations/Patient2Seg5"
nb.save(new_img_handle2, new_path2)
