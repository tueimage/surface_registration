# -*- coding: utf-8 -*-
"""
Created on Wed Jun 16 11:49:00 2021

@author: 20181933
"""
from plyfile import PlyData, PlyElement
import matplotlib.pyplot as plt
import os

for k in range(3000):
    if (os.path.exists("D:/BEP_Results/Rings 2/Output/1/Connectors_Deform_Vertex" + str(k) + ".ply")):
        plydata = PlyData.read("D:/BEP_Results/Rings 2/Output/1/Connectors_Deform_Vertex" + str(k) + ".ply")
        
        points = plydata.elements[0].data
        edges = plydata.elements[1].data
        
        fig = plt.figure()
        ax = fig.add_subplot(projection='3d')
        
        i = 0
        xmin = 10000;
        ymin = 10000;
        zmin = 10000;
        xmax = 0;
        ymax = 0;
        zmax = 0;
        for point in points:
            if (i % 2 == 0):
                ax.scatter(point[0], point[1], point[2], color = [1,0,0])
            else:
                ax.scatter(point[0], point[1], point[2], color = [0,0.8,0])
            i=i+1
            if (point[0] < xmin):
                xmin = point[0]
            if (point[0] > xmax):
                xmax = point[0]
            if (point[1] < ymin):
                ymin = point[1]
            if (point[1] > ymax):
                ymax = point[1]
            if (point[2] < zmin):
                zmin = point[2]
            if (point[2] > zmax):
                zmax = point[2]
            
        for edge in edges:
            x = [points[edge[0]][0], points[edge[1]][0]]
            y = [points[edge[0]][1], points[edge[1]][1]]
            z = [points[edge[0]][2], points[edge[1]][2]]
            ax.plot(x, y, z, color=[0.3,0.5,0.8], linestyle = 'dashed')
        
        ax.grid(False)
        ax.set_xlabel('x')
        ax.set_ylabel('y')
        ax.set_zlabel('z')
        ax.set_xlim(xmin, xmax)
        ax.set_ylim(ymin, ymax)
        ax.set_zlim(zmin, zmax)
        ax.set_xticklabels([])
        ax.set_yticklabels([])
        ax.set_zticklabels([])

        fig.tight_layout()
        plt.savefig("D:/BEP_Results/Rings 2/Vis/Connectors_Deform_Vertex" + str(k) + ".png")
        plt.show()
        plt.close()