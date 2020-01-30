import sys
import seaborn as sns
from pandas import DataFrame
sys.path.append('../')  # additing path to csaranshpp.py if not in current dir
from csaranshpp import processXyzFilesInDirGivenMetaFile, queryCdbTodownloadMetaFilesXmlAndXyz
from csaranshpp import processMetaFileNumbers, queryCdbForMetaFileNumbers
from csaranshpp_ml import analyseAndSaveJs
from csaranshpp import getDefaultConfig, writeResultsToJSON, queryCdbToProcess

dataDir = "../data/cascadesdb-dl/"  # directory to download data from cascadesdb
# all config options are described in the last cell documentation
config = getDefaultConfig()
config['logFilePath'] = "log-cdb-test.txt"
config['outputJSONFilePath'] = "cdb-test.json"
# path to csaransh library
config['csaranshLib'] = "../_build/libcsaransh-pp_shared.so"
# energy in keV and temperature in Kelvin, other options like author etc. can be selected check last cell for documentation
isSuccess, cascades = queryCdbToProcess(
    dataDir, config, material="W", energyRange=[2, 2], tempRange=["", 1500])
# writes the json file config['outputJSONFilePath']. Can be loaded again with json.load()
writeResultsToJSON(cascades, config)

# each cascade has many more properties, check end documentation for
# description of all. Names can be checked with cascade.keys();
# cascades[0].keys() or cascades[any other index].keys() 

sns.swarmplot(x="temperature", y="n_defects", data=DataFrame.from_dict(cascades))

# adds various other properties to each cascade and
# saves a js file that can be visualized in csaransh web-app (see README)
cascades, classes = analyseAndSaveJs(cascades, config, "cdb-Fe-27Jan.js")
# plotting cascade area (volume) that was just added
sns.swarmplot(x="temperature", y="hull_area", data=DataFrame.from_dict(cascades))

# demonstrating another way to process the same cascades, gives you slightly more control at the expense of loc
metaFileNumbers = queryCdbForMetaFileNumbers(
    material="W", energyRange=[2, 2], tempRange=["", 1500])
# you can select or remove metafiles, add directly in a list after checking cascadesdb
print(metaFileNumbers)
isSuccess, cascades = processMetaFileNumbers(metaFileNumbers, dataDir, config)
# writes the json file that can be loaded again with json.load()
writeResultsToJSON(cascades, config)

# demonstrating another way to process the same cascades, gives you even more control at the expense of loc

metaFilePaths, xyzDirPaths = queryCdbTodownloadMetaFilesXmlAndXyz(
    dataDir, material="W", energyRange=[2, 2], tempRange=["", 1500])
cascades = []
for metaFile, xyzDir in zip(metaFilePaths, xyzDirPaths):
    print("processing cascades for: ", metaFile, " present in dir: ", xyzDir)
    isSuccess, cascade = processXyzFilesInDirGivenMetaFile(
        metaFile, xyzDir, config)
    cascades.append(cascade)
writeResultsToJSON(cascades, config)

# optional arguments for querying cascadesdb:
# -----------------------------------
#
# applicable to following functions:
#
# - queryCdbToProcess
# - queryCdbForMetaFileNumbers
# - queryCdbTodownloadMetaFilesXmlAndXyz
# - queryCdbTodownloadMetaFilesXml
#
#     - *author* (optional): author name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
#     - *doi* (optional): doi to look for in the cascadesDB (default: "")
#     - *material* (optional): material to look for in the cascadesDB e.g. Fe, W (default: "")
#     - *tempRange* (optional): a list with two values of temperature in Kelvin for more than equal to and less than equal for filtering the data , e.g. [300, 500] will match all the cascades with 300K to 500K, [300, 300] will only match 300K, ["", 500] will match less than 500K (default: ["", ""])
#     - *energyRange* (optional): a list with two values of energy of PKA in keV for more than equal to and less than equal for filtering the data , e.g. [1, 5] will match all the cascades with 1keV to 5keV, [1, 1] will only match 1keV, ["", 5] will match less than 5keV (default: ["", ""])
#     - *archiveName* (optional): archive name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
#
#
# optional arguments  for file selection in a dir:
#  --------------------------------------------------
#
#  aplicable to following functions:
#
#  - queryCdbToProcess
#  - processMetaFileNumbers
#  - processXyzFilesInDirGivenMetaFile
#
#     - *prefix* (optional): a list of prefixes for xyz files in the archive, only files that start with one of the prefixes will be included in processing. (default: [])
#     - *suffix* (optional): a list of suffixes for xyz files in the archive, only files that end with one of the suffixes will be included in processing. (default: ["xyz"])
#     - *excludePrefix* (optional): a list of prefixes for non xyz files in the archive, files that start with one of these prefixes will NOT be included in processing. (default: ["init", "."])
#     - *excludeSuffix* (optional): a list of suffixes for non xyz files in the archive, files that end with one of the suffixes will NOT be included in processing. (default: [""])
#     - *idStartIndex* (optional): if appending to list that already has cascades then set as cascades in the list, this is to ensure id is unique for each cascade in a list, important only if you view cascades in csaransh web-app(default: 0)
#     - *onlyProcessTop* (optional): return if number of processed cascades are equal to or more than this value (default: 0 (i.e. process all))
#
#
#  all options for config:
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
