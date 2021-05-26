import nibabel as nib
import vtk
import numpy as np
import vtk.util.numpy_support
import os


def volume2surface(segmentation, out_ply, labels):
    for label in labels:
        print('Label', label)
        mask = np.zeros(segmentation.shape)
        mask[segmentation == label] = 1
        nib.save(nib.Nifti1Image(mask,np.eye(4)), 'temp.nii')

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
        print(outFile)
        plyWriter = vtk.vtkPLYWriter()
        plyWriter.SetInputData(smoother.GetOutput())
        plyWriter.SetFileName(outFile)
        plyWriter.SetFileTypeToASCII()
        plyWriter.Write()

        os.remove('temp.nii')
        

if __name__ == '__main__':
    import argparse
    description = 'Convert segmentation to a .ply file.'
    
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--labels', help='Labels to extract from segmentation.',
                         nargs='+', required=True)
    parser.add_argument('--out', help='A required ply filename base.', required=True,
                        type=str, default='TestWritePLY')
    parser.add_argument('--segm', help='A required segmentation Nifti.', required=True,
                        type=str)
    args = parser.parse_args()
    labels = list(map(int, args.labels))
    segmentation = nib.load(args.segm).get_fdata()
    volume2surface(segmentation, args.out, labels)

