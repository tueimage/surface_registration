# -*- coding: utf-8 -*-
"""
Created on Wed Apr 28 09:44:41 2021

@author: 20181933
"""

import numpy as np
import nibabel as nb
from skimage import morphology
import vtk.util.numpy_support
import os
import vtk

def segmentation(patient, label):
    if patient == 1:
        img_handle = nb.load("C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/BEPdata/133019/133019_wmseg.nii")
    else:
        img_handle = nb.load("C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/BEPdata/133625/133625_wmseg.nii") 
    img = np.asarray(img_handle.dataobj)
    brain_part = np.zeros(img.shape)
    brain_part[img==label] = 1
    brain_part = morphology.area_opening(brain_part, area_threshold = 32)
    new_img_handle = nb.Nifti1Image(brain_part, header = img_handle.header, affine = img_handle.affine)
    new_path = "C:/Users/20181933/Documents/Jaar_3/Kwartiel_4/BEP_Medical_Imaging/Segmentations/Label" + str(label) + "Patient" + str(patient)
    nb.save(new_img_handle, new_path)
    return new_path
    
def volume2surface(segmentation, out_ply, labels):
    for label in labels:
        mask = np.zeros(segmentation.shape)
        mask[segmentation == label] = 1
        nb.save(nb.Nifti1Image(mask,np.eye(4)), 'temp.nii')

        reader = vtk.vtkNIFTIImageReader()
        reader.SetFileName('temp.nii')
        reader.Update()

        marchCubes = vtk.vtkDiscreteMarchingCubes()
        marchCubes.ComputeNormalsOn()
        marchCubes.SetInputData(reader.GetOutput())
        marchCubes.SetValue(0,1)
        marchCubes.Update()

        smoother = vtk.vtkSmoothPolyDataFilter()
        smoother.SetInputData(marchCubes.GetOutput())
        smoother.SetNumberOfIterations(15)
        smoother.SetRelaxationFactor(0.1)
        smoother.FeatureEdgeSmoothingOff()
        smoother.BoundarySmoothingOn()
        smoother.Update()
   
        outFile = "{}.ply".format(out_ply)
        plyWriter = vtk.vtkPLYWriter()
        plyWriter.SetInputData(smoother.GetOutput())
        plyWriter.SetFileName(outFile)
        plyWriter.SetFileTypeToASCII()
        plyWriter.Write()

        os.remove('temp.nii')

#patients = [1,2]
#labels = [5002, 50, 4035, 4028, 4025]

def MakePly(labels, patients=[1,2]):
    for i in range(len(patients)):
        for n in range(len(labels)):
            path_seg = segmentation(patients[i], labels[n])
            label = list(map(int, [1]))
            seg = nb.load(path_seg + ".nii").get_fdata()
            volume2surface(seg, path_seg, label)