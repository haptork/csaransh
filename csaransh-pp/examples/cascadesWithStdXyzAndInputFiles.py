#!/usr/bin/env python
# coding: utf-8
"""
should be run from examples folder as current directory
processes Fe cascades with xyz files and corresponding inputs given as sample data.
prints some basic info from the processed data
runs a http server and opens the web-app with processed data loaded
"""

# In[ ]:


import sys
import os
import seaborn as sns
from pandas import DataFrame
pathToCsaranshPP = ".." # change if required
sys.path.append(pathToCsaranshPP)
from csaranshpp import getDefaultConfig, processXyzFileWithInputFile, processXyzFilesInDirWithInputFiles
buildDir = os.path.join(pathToCsaranshPP, "_build")
libPath = os.path.join(buildDir, "libcsaransh-pp_shared.so")
if (not os.path.exists(buildDir) or not os.path.exists(libPath)):
    print("Library not found at", libPath)
    print("This might be due to build errors in cmake.")
    print("If built successfully, edit this source and correct build directory & lib file (so / dlib / dll) path.")


# In[ ]:


config = getDefaultConfig()
config['logFilePath'] = "local-log-Fe.txt"
config['outputJSONFilePath'] = "local-Fe.json"
config['csaranshLib'] = libPath 
xyzDir = os.path.join(pathToCsaranshPP, "data", "parcas")
isSuccess, cascades = processXyzFilesInDirWithInputFiles(xyzDir, config)


# In[ ]:


for cascade in cascades:
    print(cascade['n_defects'], "defects in", cascade['infile'])


# In[ ]:

import matplotlib.pyplot as plt
# plotting number of defects for each temperature
sns.swarmplot(x="id", y="max_cluster_size", data=DataFrame.from_dict(cascades))
# each cascade has various properties, for description check documentation at bottom; to list all run: cascade[0].keys()
plt.show()

# In[ ]:


from csaranshpp_ml import analyseAndSaveJs
jsFile = "local-Fe.js"
cascades, classes = analyseAndSaveJs(cascades, config, jsFile)


# In[ ]:

indexFile = 'apps/index.html'
supportDir = 'apps/CSaransh_files'
if not(os.path.exists(indexFile) and os.path.exists(supportDir) and os.path.isdir(supportDir)):
    print("please run file from examples directory to run the server automatically.")
    sys.exit(0)

import shutil
import threading
import webbrowser
import time

shutil.copy(jsFile, os.path.join(supportDir ,"cascades-data.js"))

from http.server import HTTPServer, SimpleHTTPRequestHandler 

def start_server(server_class=HTTPServer, handler_class=SimpleHTTPRequestHandler):
    server_address = ('', 8080)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()

thread = threading.Thread(target=start_server)
thread.start()

print ("starting server - press ctrl + C to exit")
url = 'http://0.0.0.0:8080/'+ indexFile
webbrowser.open_new(url)

while True:
    try:
        time.sleep(1)
    except KeyboardInterrupt:
        print ("exiting....")
        sys.exit(0)
sys.exit(0)

#  optional arguments  for file selection in a dir and processing them:
#  ----------------------------------------------------------------------
#  
#  aplicable to following functions:
#  
#  - processXyzFilesInDirWithInputFiles
#  
#     - *prefix* (optional): a list of prefixes for xyz files in the archive, only files that start with one of the prefixes will be included in processing. (default: [])
#     - *suffix* (optional): a list of suffixes for xyz files in the archive, only files that end with one of the suffixes will be included in processing. (default: ["xyz"])
#     - *excludePrefix* (optional): a list of prefixes for non xyz files in the archive, files that start with one of these prefixes will NOT be included in processing. (default: ["init", "."])
#     - *excludeSuffix* (optional): a list of suffixes for non xyz files in the archive, files that end with one of the suffixes will NOT be included in processing. (default: [""])
#     - *idStartIndex* (optional): if appending to list that already has cascades then set as cascades in the list, this is to ensure id is unique for each cascade in a list, important only if you view cascades in csaransh web-app(default: 0)    
#     - *onlyProcessTop* (optional): return if number of processed cascades are equal to or more than this value (default: 0 (i.e. process all))
#  
# 
# all options for config:
#  --------------------------------
#  
#  returned with getDefaultConfig()
#   - "csaranshLib" : path to csaransh c++ library most probably compiled with cmake
#   - "onlyDefects" : Switch - Compute only the defect coordinates (default: False)
#   - "isFindDistribAroundPKA": Switch - compute distribution around pka if pka coordinates are given (default: True)
#   - "isFindClusterFeatures": Switch - find cluster features that can be used for pattern matching and classification of clusters later (default: True)
#   - "filterZeroSizeClusters": Switch - Ignore clusters that have zero surviving defects. The defects in these clusters are all added by threshold based algorithm (default: False)
#   - "isIgnoreBoundaryDefects": Switch - Ignore defects appearing in the unit cells at the boundary. Useful since defects appear at boundary due to PBC if origin / offset is not given properly in MD simulations, one condition where it can be set to False is if offset / origin is 0.25 in bcc (default: True)
#   - "isAddThresholdInterstitials": Switch - Add threshold based interstitials over the one found using W-S like algorithm (default: True)
#   - "safeRunChecks": Check and ignore files with anomalous number or proportion of defects (default: True)
#   - "thresholdFactor": threshold factor for threshold based interstitials (threshol value will be factor * latticeConstant), applicable only if threshold based interstitials are allowed. (default: 0.345)
#   - "extraDefectsSafetyFactor": safety factor for checks, lower value implies stricter checks to ignore files. Only matters if safety checks are not disabled altogether. (default: 50.0),
#   - "logFilePath": (default log-csaransh-pp-cpp.txt)
#   - "outputJSONFilePath": only needed if saving json file, (default cascades-data-py.json)
#   - "logMode": (default: warning and error (2 + 4 = 6)) can be set by input parameters to getDefaultConfig function. Its paramters can be any combination of the following strings: 
#     "none", "info", "warning", "error", "debug", "all"
#     enabling the logging for those messages.
#     example call: getDefaultConfig("info", "warning", "error")

# All keys of the resulting cascades
# ------------------------------------------------
# 
# - Input
#     - xyzFilePath
#     - id : possibly unique number for each cascade in the json list.
#     - substrate : Material Formula e.g. W, Fe etc.
#     - energy : energy of PKA for the cascade
#     - simulationTime
#     - ncell : number of cells in the simulation box
#     - boxSize : simulation box size
#     - origin : offset or origin given in MD simulation
#     - rectheta : angle for PKA
#     - recphi : angle for PKA
#     - xrec : position for PKA
#     - yrec : position for PKA
#     - zrec : position for PKA
#     - latticeConst
#     - temperature
#     - infile : input or meta file
#     - tags : any text that can be given in input for use with filtering or grouping and comparison in statistics
#     - potentialUsed
#     - isPkaGiven : is position of pka was given as input while processing
#     - originType : 0 (given), 1 (estimated) ,  2 (both were tried)
#     - simulationCode: Lammps, Parcas, Lammps-disp, cascadesdblikecols
# 
# - output scalar values added by csaranshpp:
#     - error : error message while processing if any
#     - n_defects : number of defects found
#     - n_clusters : number of clusters found
#     - max_cluster_size_I : maximum size of the interstitial cluster 
#     - max_cluster_size_V : maximum size of the vacancy cluster 
#     - max_cluster_size : maximum size of a cluster 
#     - in_cluster_I : proportion of interstitials in the clusters
#     - in_cluster_V : proportion of vacancies in the clusters
#     - in_cluster : proportion of defects in the clusters
#     
# - output lists added by csaranshpp:
#     - coords : list of coordinates, each item is [x, y, z, isIntersitial, clusterId, isSurviving], clusterId is zero if defect is single e.g. [3.5, 2.45, -1.4, 0, 2, 1] is a surviving vacancy in cluster 2. 
#     - clusters : dictionary of cluster ids each having list of indices that correspond to defect coordinates that belong to that cluster id. e.g. {'2': [11, 2, 34], '206': [9, 1, 7, 124]}, there are two clusters with ids '2' and '206' having 3 and 4 defects respectively. The coordinates for defects in cluster-id '2' can be found in coords[11], coords[2], coords[34].
#     - clusterSizes : surviving number of defects for each cluster-id. negative values imply the vacancy cluster. e.g. {'2': -3, '206': 2} implies that there are two clusters(ids: '2','206'). '2' has three surviving vacancies while '206' has two surviving interstitials.
#     - features : The distance and angle histogram features for each cluster-id representing its shape. Check the research article on classification of defect clusters for algorithm (https://doi.org/10.1016/j.commatsci.2019.109364).
#     - dumbbellPairs : The interstitial-vacancy pair for interstitials e.g. [[4, 12], [6, 19], [9, 11]], there are three interstitial-vacancy pairs. The position of first pair is coords[4] as interstitial and coords[12] as corresponding vacancy. The other interstitial for the first dumbbell would be coords[5].
#     - distancesI : distance distribution of interstitials from PKA if pka position was given as input. 
#     - distancesV : distance distribution of vacancies from PKA if pka position was given as input. 
#     - anglesI : angle distribution of interstitials from PKA if pka position was given as input. 
#     - anglesV : angle distribution of vacancies from PKA if pka position was given as input.
#     
# - added by csaranshpp-ml:
#     - eigen_coords : coordinates of defects in principle components (PCA). Ideal for plotting the cascade in 2D.
#     - eigen_pka : 3D coordinates of pka in principle components (PCA) found for coords.
#     - eigen_var : variance explained by first, second and third principle component e.g. [0.5, 0.3, 0.2]. A high value in first implies the cascade is spread in one direction way more than others, while a high value in first + second would imply it is a planar cascade. A value of 0.95 or above in first index can mean highly linear cascade while higher than 0.95 in first two would mean planar.
#     - eigen_features : 
#     - dclust_coords : density clusters found using dbscan, the density clusters are bigger islands of densities surrounded by very sparse regions. The number of dense regions of vacancies has a good correlation with number of subcascades.
#     - dclustI_count : number of interstitial density cluster islands.
#     - dclust_sec_impact : The proportion of defects in the second biggest density cluster of vacancies. This correspond to the impact of secondary sub-cascade.
#     - hull_vol : cascade hull volume.
#     - hull_area : cascade hull area.
#     - hull_density : cascade hull density.
#     - hull_vertices : cascade hull vertices.
#     - hull_simplices : cascade hull simplices.
#     - hull_nvertices : cascade hull number of vertices.
#     - hull_nsimplices : cascade hull number of simplices.
#     - clust_cmp : clusters with similar shape for each cluster id, based on histogram features of 'angle', 'dist' and 'both'.
#     - clust_cmp_size : clusters with similar shape and size for each cluster id, based on histogram features of 'angle', 'dist' and 'both'.
#     - clusterClasses : cluster shape class for each cluster id. To know the typical shape for each cluster check the research article on classification of defect clusters (https://doi.org/10.1016/j.commatsci.2019.109364) .
