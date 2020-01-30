import os
from ctypes import c_bool, c_int, c_double, c_char_p, cdll, Structure, c_void_p, cast
import xmltodict
import json
import urllib.request
from pathlib import Path
import shutil
import sys

# utilities
# -------------------------------------------------


def _downloadFile(fileUrl, dirPath, fileName):
    if not os.path.exists(dirPath) or not os.path.isdir(dirPath):
        print("Data directory :", dirPath, "is not valid")
        return ""
    filePath = os.path.join(dirPath, fileName)
    if os.path.exists(filePath):
        return filePath
    tmpSuffix = "_tmp_pydownload"
    tmpFilePath = os.path.join(dirPath, fileName + tmpSuffix)
    print('Beginning file download: ' + fileUrl)
    try:
        urllib.request.urlretrieve(fileUrl, tmpFilePath)
    except:
        if os.path.exists(tmpFilePath):
            os.remove(tmpFilePath)
        print("Error while downloading: ", sys.exc_info()[0])
        raise
    else:
        os.rename(tmpFilePath, filePath)
        print('Downloaded')
    return filePath


def _unzipFile(dirPath, filePath, archiveName):
    archiveNameWoExt = archiveName.split('.')[0]
    destDirPath = os.path.join(dirPath, archiveNameWoExt)
    if not (os.path.isdir(destDirPath) and os.path.exists(destDirPath)):
        os.mkdir(destDirPath)
    else:
        print("Directory " + destDirPath +
              " already exists. Continuing without unzipping, assuming that it already contains files.")
        return destDirPath
    print("Unzipping to ", destDirPath)
    tmpSuffix = "_tmp_pydownload"
    tmpDirPath = destDirPath + tmpSuffix
    try:
        shutil.unpack_archive(filePath, tmpDirPath)
    except:
        if os.path.exists(tmpDirPath):
            shutil.rmtree(tmpDirPath)
        print("Error while Unzipping: ", sys.exc_info()[0])
        return ""
    else:
        os.rename(tmpDirPath, destDirPath)
        print('Unzipped.')
    return destDirPath


def _getAllFilesWithPrefixSuffix(rootDirPath, prefixes=[], suffixes=["xyz"], excludePrefixes=["./", "init"], excludeSuffixes=[]):
    res = []
    # dirpath, dirnames, filenames
    for dirpath, _, filenames in os.walk(rootDirPath):
        for f in filenames:
            toAdd = True
            for prefix in prefixes:
                if not f.startswith(prefix):
                    toAdd = False
            for suffix in suffixes:
                if not f.endswith(suffix):
                    toAdd = False
            for excludePrefix in excludePrefixes:
                if f.startswith(excludePrefix):
                    toAdd = False
            for excludeSuffix in excludeSuffixes:
                if f.endswith(excludeSuffix):
                    toAdd = False
            if toAdd:
                res.append(os.path.join(dirpath, f))
    return res


def xmlFileToDict(fname):
    f = open(fname, 'r')
    xmlStr = f.read()
    di = xmltodict.parse(xmlStr)
    return di

# cpp call specific internal
# -------------------------------------------------


class _InputInfoCpp(Structure):
    _fields_ = [("ncell", c_int),
                ("boxSize", c_double),
                ("latticeConst", c_double),
                ("originX", c_double),
                ("originY", c_double),
                ("originZ", c_double),
                ("originType", c_int),
                ("temperature", c_double),
                ("xyzFileType", c_char_p),
                ("xyzFilePath", c_char_p),
                ("structure", c_char_p)
                ]


class _ExtraInfoCpp(Structure):
    _fields_ = [("energy", c_double),
                ("simulationTime", c_double),
                ("id", c_int),
                ("isPkaGiven", c_bool),
                ("xrec", c_double),
                ("yrec", c_double),
                ("zrec", c_double),
                ("rectheta", c_double),
                ("recphi", c_double),
                ("substrate", c_char_p),
                ("infile", c_char_p),
                ("tags", c_char_p),
                ("potentialUsed", c_char_p),
                ("author", c_char_p)
                ]


class _ConfigCpp(Structure):
    _fields_ = [("onlyDefects", c_bool),
                ("isFindDistribAroundPKA", c_bool),
                ("isFindClusterFeatures", c_bool),
                ("filterZeroSizeClusters", c_bool),
                ("isIgnoreBoundaryDefects", c_bool),
                ("isAddThresholdInterstitials", c_bool),
                ("safeRunChecks", c_bool),
                ("thresholdFactor", c_double),
                ("extraDefectsSafetyFactor", c_double),
                ("logMode", c_int),
                ("logFilePath", c_char_p),
                ("outputJSONFilePath", c_char_p)
                ]


def _cookInfoForCpp(info, infoCpp):
    for field, ctype in infoCpp._fields_:
        curValue = str.encode(
            info[field]) if ctype == c_char_p else info[field]
        setattr(infoCpp, field, curValue)
    return infoCpp

# API or scraping CascadesDB
# -------------------------------------------------


def _getHtmlPage(url):
    fp = urllib.request.urlopen(url)
    mybytes = fp.read()
    mystr = mybytes.decode("utf8")
    fp.close()
    return (mystr)


def _cookQueryUrl(author="", doi="", material="", tempRange=["", ""], energyRange=["", ""], archiveName=""):
    tempRange = [str(x) for x in tempRange]
    energyRange = [str(x) for x in energyRange]
    baseUrl = "https://cascadesdb.org/cdbmeta/"
    queryStr = "?attribution__person__name=" + author
    queryStr += "&attribution__publication_doi=" + doi
    queryStr += "&material__chemical_formula=" + material
    queryStr += "&initial_temperature_gte=" + tempRange[0]
    queryStr += "&initial_temperature_lte=" + tempRange[1]
    queryStr += "&energy__gte=" + energyRange[0]
    queryStr += "&energy__lte=" + energyRange[1]
    queryStr += "&archive_name=" + archiveName
    return baseUrl+queryStr


def _extractXmlNumbers(queryUrl):
    pageNumber = 1
    res = []
    while(True):
        htmlPage = _getHtmlPage(queryUrl + "&page=" + str(pageNumber))
        curRes = []
        for line in htmlPage.split():
            needle = 'href="/cdbmeta/cdbrecord/xml/'
            index = line.find(needle)
            if (index == -1):
                continue
            startIndex = index + len(needle)
            endIndex = line[startIndex:].find('/')
            if (endIndex == -1):
                continue
            curRes.append(int(line[startIndex:startIndex+endIndex]))
        if curRes[0] in res:
            break
        res += curRes
        if len(curRes) < 10:
            break
        pageNumber += 1
    print("Total meta files satisfying the query: ", len(res))
    return res


def _downloadMetaFileXml(fileNumber, dirPath):
    baseUrl = "https://cascadesdb.org/cdbmeta/cdbrecord/xml/"
    fileUrl = baseUrl + str(fileNumber)
    filePath = _downloadFile(fileUrl, dirPath, str(fileNumber) + ".xml")
    return filePath


def _downloadAndUnzipXyzFiles(metaFilePath, dirPath):
    meta = xmlFileToDict(metaFilePath)
    archiveName = meta['cdbml']['cdbrecord']['data']['archive_name']
    baseUrl = "https://cascadesdb.org/data/cdb/"
    archiveUrl = baseUrl + archiveName
    expectedDir = os.path.join(dirPath, archiveName.split('.')[0])
    if (os.path.isdir(expectedDir) and os.path.exists(expectedDir)):
        print("Directory " + expectedDir +
              " already exists. Continuing without download & unzip.")
        return expectedDir
    filePath = _downloadFile(archiveUrl, dirPath, archiveName)
    return _unzipFile(dirPath, filePath, archiveName)

# helper functions
# -------------------------------------------------


def _sanitizeCppRes(res):
    res = res.decode().replace('\n', '')
    return json.loads(res)


def _validateMetaInfo(metaInfo):
    return metaInfo['has_surface'] == 'false' and metaInfo['initially_perfect'] == 'true' and metaInfo['material']['structure'] == 'bcc'


def _validateInfo(info, extraInfo):
    validXyzFileTypes = ["CASCADESDBLIKECOLS",
                         "LAMMPS-XYZ", "LAMMPS-DISP", "PARCAS"]
    if info['originType'] < 0 or info['originType'] > 2:
        print("Valid values of origin type are : 0 (use given origin only), 1 (estimated origin), 2 (try both use best of the two)")
        return False
    if not info['xyzFileType'].upper() in validXyzFileTypes:
        print("Valid values of xyzFileTypes are:",
              ", ".join(validXyzFileTypes))
        return False
    return True


def _validateConfig(config):
    if config['logMode'] > 15:
        print("Valid value of logmode is in range 1 to 15")
        return False
    return True

# user functions
# -------------------------------------------------


def getDefaultConfig(*logModes):
    """Gives a default configuration dictionary.
    Returns
    -------
    a configuration dictionary that can be modified later. It has following fields:

    "csaranshLib" : path to csaransh c++ library most probably compiled with cmake
    "onlyDefects" : Switch - Compute only the defect coordinates (default: False)
    "isFindDistribAroundPKA": Switch - compute distribution around pka if pka coordinates are given (default: True)
    "isFindClusterFeatures": Switch - find cluster features that can be used for pattern matching and classification of clusters later (default: True)
    "filterZeroSizeClusters": Switch - Ignore clusters that have zero surviving defects. The defects in these clusters are all added by threshold based algorithm (default: False)
    "isIgnoreBoundaryDefects": Switch - Ignore defects appearing in the unit cells at the boundary. Useful since defects appear at boundary due to PBC if origin / offset is not given properly in MD simulations, one condition where it can be set to False is if offset / origin is 0.25 in bcc (default: True)
    "isAddThresholdInterstitials": Switch - Add threshold based interstitials over the one found using W-S like algorithm (default: True)
    "safeRunChecks": Check and ignore files with anomalous number or proportion of defects (default: True)
    "thresholdFactor": threshold factor for threshold based interstitials (threshol value will be factor * latticeConstant), applicable only if threshold based interstitials are allowed. (default: 0.345)
    "extraDefectsSafetyFactor": safety factor for checks, lower value implies stricter checks to ignore files. Only matters if safety checks are not disabled altogether. (default: 50.0),
    "logMode": as set by input parameters (default: warning and error (2 + 4 = 6))
    "logFilePath": (default log-csaransh-pp-cpp.txt)
    "outputJSONFilePath": only needed if saving json file, (default cascades-data-py.json)

    Parameters
    ---------
    *logModes: can be any combination of the following strings: 
    "none", "info", "warning", "error", "debug", "all"
    enabling the logging for those messages.
    example call: getDefaultConfig("info", "warning", "error")


    """
    config = {
        "csaranshLib": "./_build/libcsaransh-pp_shared.so",
        "onlyDefects": False,
        "isFindDistribAroundPKA": True,
        "isFindClusterFeatures": True,
        "filterZeroSizeClusters": False,
        "isIgnoreBoundaryDefects": True,
        "isAddThresholdInterstitials": True,
        "safeRunChecks": True,
        "thresholdFactor": 0.345,
        "extraDefectsSafetyFactor": 50.0,
        "logMode": 6,
        "logFilePath": "log-csaransh-pp-py.txt",
        "outputJSONFilePath": "cascades-data-py.json"
    }
    if logModes:
        config['logMode'] = 0
        for logMode in logModes:
            if logMode.lower() == "none":
                config['logMode'] = 0
            elif logMode.lower() == "info":
                config['logMode'] |= 1
            elif logMode.lower() == "warning":
                config['logMode'] |= 2
            elif logMode.lower() == "error":
                config['logMode'] |= 4
            elif logMode.lower() == "debug":
                config['logMode'] |= 8
            elif logMode.lower() == "all":
                config['logMode'] = 15
    return config


def writeResultsToJSON(res, config):
    """writes json file with config in "meta" and cascades in "data" keys, 
       the output filename is same as config['outputJSONFilePath']
    """
    di = {"meta": config, "data": res}
    f = open(config['outputJSONFilePath'], "w")
    json.dump(di, f)
    f.close()


# user functions for processing with custom info
# -------------------------------------------------


def processXyzFileGivenInfo(info, extraInfo, config):
    """processes cascade xyz file given info

    Returns
    -------
    a dictionary that corresponds to the cascade with various
    attibutes such as n_defects (number of defects), maxClusterSize, clusters, 
    coords (coordinares of defects (x, y, z, isInterstitial, clusterId, isSurviving)). More on the structure of
    the cascade dictionary can be found in the documentation or can be explored with csaransh web app.

    Parameters
    -----------
    info: info obtained from getDefaultInfos and modified. xyzFilePath must be set. infile may also be set.
    extraInfo: info obtained from getDefaultInfos (and possibly modified)
    config : configuration obtained from getDefault config (and possibly modified)
    """


    if not _validateConfig(config) or not _validateInfo(info, extraInfo):
        return False, ""
    lib = cdll.LoadLibrary(config["csaranshLib"])
    lib.pyProcessFile.restype = c_void_p
    res = lib.pyProcessFile(_cookInfoForCpp(info, _InputInfoCpp()), _cookInfoForCpp(
        extraInfo, _ExtraInfoCpp()), _cookInfoForCpp(config, _ConfigCpp()))
    resStr = _sanitizeCppRes(cast(res, c_char_p).value)
    lib.dalloc.argtypes = [c_void_p]
    lib.dalloc(res)
    return True, resStr


def processXyzFilesInDirGivenInfo(xyzDir, info, extraInfo, config, idStartIndex=0, onlyProcessTop = 0, prefix=[], suffix=["xyz"], excludePrefix=["init", "."], excludeSuffix=[]):
    """processes cascades from a directory given info objects

    Returns
    -------
    a list of processed cascades, each item is a dictionary that corresponds to a cascade with varioush 
    attibutes such as n_defects (number of defects), maxClusterSize, clusters, 
    coords (coordinares of defects (x, y, z, isInterstitial, clusterId, isSurviving)). More on the structure of
    the cascade dictionary can be found in the documentation or can be explored with csaransh web app.

    Parameters
    -----------
    xyzDir: directory path where all the input files and xyz files are
    info: info obtained from getDefaultInfos and modified (xyzFilePath, infile can be left as default)
    extraInfo: info obtained from getDefaultInfos (and possibly modified)
    config : configuration obtained from getDefaultConfig (and possibly modified)
    idStartIndex (optional): if appending to list that already has cascades then set as cascades 
                             in the list, this is to ensure id is unique for each cascade in a list,
                             important only if you view cascades in csaransh web-app(default: 0)
    onlyProcessTop (optional): return if number of processed cascades are equal to or more than this value
    prefix (optional): a list of prefixes for xyz files in the archive, only files that start with one of the prefixes will be included in processing. (default: [])
    suffix (optional): a list of suffixes for xyz files in the archive, only files that end with one of the suffixes will be included in processing. (default: ["xyz"])
    excludePrefix (optional): a list of prefixes for non xyz files in the archive, files that start with one of these prefixes will NOT be included in processing. (default: ["init", "."])
    excludeSuffix (optional): a list of suffixes for non xyz files in the archive, files that end with one of the suffixes will NOT be included in processing. (default: [""])
    """


    if not(os.path.isdir(xyzDir) and os.path.exists(xyzDir)):
        return False, "Error in xyz directory path"
    xyzFiles = _getAllFilesWithPrefixSuffix(
        xyzDir, prefix, suffix, excludePrefix, excludeSuffix)
    print(str(len(xyzFiles)) + " xyz files")
    res = []
    for i, xyzFile in enumerate(xyzFiles):
        print("processing " + str(i + 1) + ": " + xyzFile)
        extraInfo['id'] = idStartIndex + i + 1
        info["xyzFilePath"] = xyzFile
        extraInfo["infile"] = xyzFile
        isSuccess, curRes = processXyzFileGivenInfo(info, extraInfo, config)
        if (isSuccess):
            res.append(curRes)
        else:
            print(curRes)
        if onlyProcessTop > 0 and len(res) >= onlyProcessTop:
            break
    return True, res


def getDefaultInfos():
    """ Get default inputs to process an xyz file.

    Returns
    -------
    inputInfo and extraInfo dictionary. Below is the description of each key.

    InputInfo
    ---------
    xyzFilePath (required): path to xyz file.
    boxSize : boxsize, leave to default if not known. Required if latConst is not known.
    latticeConst : latticeConstant. Required if boxSize is not known.
    ncell (optional): number of cells, leave to default if not known.
    origin(X/Y/Z) (optional): x/y/z coordinate of the offset or origin used in MD simulation.
    originType (optional): 1 for autoinfer (default), 0 for take only what is given, 2 for try given and inferred both.
    temperature (optional)
    xyzFileType (optional): can be parcas, lammps-xyz, lammps-disp or cascadesdblikecols (if file is downloaded or compliant with cascades db columns "element x y z" format)
    structure (optional): only bcc is valid as of current implementation.

    ExtraInfo (all optional)
    ----------
    isPkaGiven (is pka coorinate is known for the simulation)
    x/y/zrec: coordinates of PKA
    rec(theta/phi): angles of PKA
    substrate: element or material e.g. Fe or W.
    """
    inputInfo = {
        "ncell": -1,
        "boxSize": -1.0,
        "latticeConst": -1.0,
        "originX": 0.0,
        "originY": 0.0,
        "originZ": 0.0,
        "originType": 1,
        "temperature": 0.0,
        "xyzFileType": "CASCADESDBLIKECOLS",
        "xyzFilePath": "",
        "structure": "bcc"
    }
    extraInfo = {
        "energy": 0.0,
        "simulationTime": 0.0,
        "id": 1,
        "isPkaGiven": False,
        "xrec": 0.0,
        "yrec": 0.0,
        "zrec": 0.0,
        "rectheta": 0.0,
        "recphi": 0.0,
        "substrate": "",
        "infile": "",
        "tags": "",
        "potentialUsed": "",
        "author": ""
    }
    return inputInfo, extraInfo

# user functions for processing with only xyzFileName assuming input files are present
# -------------------------------------------------


def processXyzFileWithInputFile(xyzFilePath, config, id = 1):
    """processes cascade xyz file assuming input (.in) files exist.

    Returns
    -------
    a dictionary that corresponds to the cascade with various
    attibutes such as n_defects (number of defects), maxClusterSize, clusters, 
    coords (coordinares of defects (x, y, z, isInterstitial, clusterId, isSurviving)). More on the structure of
    the cascade dictionary can be found in the documentation or can be explored with csaransh web app.

    Parameters
    -----------
    xyzFilePath: path to the xyz file (input file should also be there with either same name replacing 
                 xyz extension with .in or with name "common_input.in" must be there. 
                 "common_input.in" can be in current directory as well. Input files can be PARCAS or Lammps xyz / displaced atom, for more check the format of input file check examples)
    config : configuration obtained from getDefault config (and possibly modified)
    id : id to be assigned to the file, if appending to and existing cascades results then this must be len(cascades) + 1
    """


    if not _validateConfig(config):
        return False, ""
    lib = cdll.LoadLibrary(config["csaranshLib"])
    lib.pyProcessFileWoInfo.restype = c_void_p
    res = lib.pyProcessFileWoInfo(
        c_char_p(xyzFilePath.encode()), _cookInfoForCpp(config, _ConfigCpp()))
    resDi = _sanitizeCppRes(cast(res, c_char_p).value)
    lib.dalloc.argtypes = [c_void_p]
    lib.dalloc(res)
    resDi['id'] = id
    return True, resDi


def processXyzFilesInDirWithInputFiles(xyzDir, config, idStartIndex=0, onlyProcessTop = 0, prefix=[], suffix=["xyz"], excludePrefix=["init", "."], excludeSuffix=[]):
    """processes cascades from a directory assuming input (.in) files exist.

    Returns
    -------
    a list of processed cascades, each item is a dictionary that corresponds to a cascade with varioush 
    attibutes such as n_defects (number of defects), maxClusterSize, clusters, 
    coords (coordinares of defects (x, y, z, isInterstitial, clusterId, isSurviving)). More on the structure of
    the cascade dictionary can be found in the documentation or can be explored with csaransh web app.

    Parameters
    -----------
    xyzDir: directory path where all the input files and xyz files are
    config : configuration obtained from getDefault config (and possibly modified)
    idStartIndex (optional): if appending to list that already has cascades then set as cascades 
                             in the list, this is to ensure id is unique for each cascade in a list,
                             important only if you view cascades in csaransh web-app(default: 0)
    onlyProcessTop (optional): return if number of processed cascades are equal to or more than this value
    prefix (optional): a list of prefixes for xyz files in the archive, only files that start with one of the prefixes will be included in processing. (default: [])
    suffix (optional): a list of suffixes for xyz files in the archive, only files that end with one of the suffixes will be included in processing. (default: ["xyz"])
    excludePrefix (optional): a list of prefixes for non xyz files in the archive, files that start with one of these prefixes will NOT be included in processing. (default: ["init", "."])
    excludeSuffix (optional): a list of suffixes for non xyz files in the archive, files that end with one of the suffixes will NOT be included in processing. (default: [""])
    """


    if not(os.path.isdir(xyzDir) and os.path.exists(xyzDir)):
        return False, "Error in xyz directory path"
    xyzFiles = _getAllFilesWithPrefixSuffix(
        xyzDir, prefix, suffix, excludePrefix, excludeSuffix)
    print(str(len(xyzFiles)) + " xyz files")
    res = []
    for i, xyzFile in enumerate(xyzFiles):
        print("processing " + str(i + 1) + ": " + xyzFile)
        isSuccess, curRes = processXyzFileWithInputFile(xyzFile, config)
        if (isSuccess):
            curRes['id'] = idStartIndex + i + 1
            res.append(curRes)
        else:
            print(curRes)
        if onlyProcessTop > 0 and len(res) >= onlyProcessTop: break
    return True, res

# user functions for processing with cascades-db meta files
# -------------------------------------------------


def processXyzFilesInDirGivenMetaFile(metaFilePath, xyzDir, config, idStartIndex=0, onlyProcessTop = 0, prefix=[], suffix=["xyz"], excludePrefix=["init", "."], excludeSuffix=[]):
    """queries, downloads and processes cascades from cascadesDB.

    Returns
    -------
    a dictionary that corresponds to the cascade with various
    attibutes such as n_defects (number of defects), maxClusterSize, clusters, 
    coords (coordinares of defects (x, y, z, isInterstitial, clusterId, isSurviving)). More on the structure of
    the cascade dictionary can be found in the documentation or can be explored with csaransh web app.

    Parameters
    -----------
    metaFilePath: path to the xml meta file in the cascadesdb format
    xyzDir: directory path where all the meta files and xyz files will be
              downloaded (if not already there), keeping the same directory 
              for download will make sure that the data is downloaded only once
    config : configuration obtained from getDefault config (and possibly modified)
    idStartIndex (optional): if appending to list that already has cascades then set as cascades 
                             in the list, this is to ensure id is unique for each cascade in a list,
                             important only if you view cascades in csaransh web-app(default: 0)
    onlyProcessTop (optional): return if number of processed cascades are equal to or more than this value
    prefix (optional): a list of prefixes for xyz files in the archive, only files that start with one of the prefixes will be included in processing. (default: [])
    suffix (optional): a list of suffixes for xyz files in the archive, only files that end with one of the suffixes will be included in processing. (default: ["xyz"])
    excludePrefix (optional): a list of prefixes for non xyz files in the archive, files that start with one of these prefixes will NOT be included in processing. (default: ["init", "."])
    excludeSuffix (optional): a list of suffixes for non xyz files in the archive, files that end with one of the suffixes will NOT be included in processing. (default: [""])
    """

    if not(os.path.isdir(xyzDir) and os.path.exists(xyzDir)):
        return False, "Error in xyz directory path"
    meta = xmlFileToDict(metaFilePath)
    metaInfo = meta['cdbml']['cdbrecord']
    if not _validateMetaInfo(metaInfo):
        return False, metaFilePath + ": csaransh does not yet support processing corresponding cascades (only supports perfect, not surface bccs)"
    xyzFiles = _getAllFilesWithPrefixSuffix(
        xyzDir, prefix, suffix, excludePrefix, excludeSuffix)
    print(str(len(xyzFiles)) + " xyz files corresponding to meta file " + metaFilePath)
    res = []
    for i, xyzFile in enumerate(xyzFiles):
        print("processing " + str(i + 1) + ": " + xyzFile)
        info, extraInfo = getInfoFromMeta(metaInfo, metaFilePath, xyzFile)
        extraInfo['id'] = idStartIndex + i + 1
        isSuccess, curRes = processXyzFileGivenInfo(info, extraInfo, config)
        if (isSuccess):
            res.append(curRes)
        else:
            print(curRes)
        if onlyProcessTop > 0 and len(res) >= onlyProcessTop: break
    return True, res


def processMetaFileNumbers(metaFileNumbers, dirPath, config, idStartIndex = 0, onlyProcessTop = 0, prefix=[], suffix=["xyz"], excludePrefix=["init", "."], excludeSuffix=[]):
    """queries, downloads and processes cascades from cascadesDB.

    Returns
    -------
    a list of processed cascades, each item is a dictionary that corresponds to a cascade with varioush 
    attibutes such as n_defects (number of defects), maxClusterSize, clusters, 
    coords (coordinares of defects (x, y, z, isInterstitial, clusterId, isSurviving)). More on the structure of
    the cascade dictionary can be found in the documentation or can be explored with csaransh web app.

    Parameters
    -----------
    metaFileNumbers: a list of meta file numbers that can be retrieved based on filters with queryCdbTodownloadMetaFilesXmlAndXyz or queryCdbForMetaFileNumbers (if downloaded).
    dirPath : directory path where all the meta files and xyz files will be
              downloaded (if not already there), keeping the same directory 
              for download will make sure that the data is downloaded only once
    config : configuration obtained from getDefault config (and possibly modified)
    idStartIndex (optional): if appending to list that already has cascades then set as cascades 
                             in the list, this is to ensure id is unique for each cascade in a list,
                             important only if you view cascades in csaransh web-app(default: 0)
    onlyProcessTop (optional): return if number of processed cascades are equal to or more than this value
    prefix (optional): a list of prefixes for xyz files in the archive, only files that start with one of the prefixes will be included in processing. (default: [])
    suffix (optional): a list of suffixes for xyz files in the archive, only files that end with one of the suffixes will be included in processing. (default: ["xyz"])
    excludePrefix (optional): a list of prefixes for non xyz files in the archive, files that start with one of these prefixes will NOT be included in processing. (default: ["init", "."])
    excludeSuffix (optional): a list of suffixes for non xyz files in the archive, files that end with one of the suffixes will NOT be included in processing. (default: [""])
    """

    if not(os.path.isdir(dirPath) and os.path.exists(dirPath)):
        return False, "Error in directory path"
    res = []
    metaFilePaths = []
    xyzDirs = []
    for metaFileNumber in metaFileNumbers:
        metaFilePath = _downloadMetaFileXml(metaFileNumber, dirPath)
        xyzDir = _downloadAndUnzipXyzFiles(metaFilePath, dirPath)
        metaFilePaths.append(metaFilePath)
        xyzDirs.append(xyzDir)
    for metaFilePath, xyzDir in zip(metaFilePaths, xyzDirs):
        isSuccess, curRes = processXyzFilesInDirGivenMetaFile(
            metaFilePath, xyzDir, config, len(res) + idStartIndex, onlyProcessTop, prefix, suffix, excludePrefix, excludeSuffix)
        if (isSuccess):
            res += curRes
        else:
            print(curRes)
        if onlyProcessTop > 0 and len(res) >= onlyProcessTop: break
    return True, res


def queryCdbToProcess(baseDirPathToDownload, config, idStartIndex = 0, onlyProcessTop = 0, author="", doi="", material="", tempRange=["", ""], energyRange=["", ""], archiveName="", prefix=[], suffix=["xyz"], excludePrefix=["init", "."], excludeSuffix=[]):
    """queries, downloads and processes cascades from cascadesDB.

    Returns
    -------
    a list of processed cascades, each item is a dictionary that corresponds to a cascade with varioush 
    attibutes such as n_defects (number of defects), maxClusterSize, clusters, 
    coords (coordinares of defects (x, y, z, isInterstitial, clusterId, isSurviving)). More on the structure of
    the cascade dictionary can be found in the documentation or can be explored with csaransh web app.

    Parameters
    -----------
    baseDirPathToDownload : directory path where all the meta files and xyz files will be
                            downloaded, keeping the same directory for download will make sure that the
                            data is downloaded only once
    config : configuration obtained from getDefault config (and possibly modified)
    idStartIndex (optional): if appending to list that already has cascades then set as cascades 
                             in the list, this is to ensure id is unique for each cascade in a list,
                             important only if you view cascades in csaransh web-app(default: 0)
    onlyProcessTop (optional): return if number of processed cascades are equal to or more than this value
    author (optional): author name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    doi (optional): doi to look for in the cascadesDB (default: "")
    material (optional): material to look for in the cascadesDB e.g. Fe, W (default: "")
    tempRange (optional): a list with two values of temperature in Kelvin for more than 
                          equal to and less than equal for filtering the data , e.g. [300, 500]
                          will match all the cascades with 300K to 500K, [300, 300] will only match 300K,
                          ["", 500] will match less than 500K (default: ["", ""])
    energyRange (optional): a list with two values of energy of PKA in keV for more than 
                            equal to and less than equal for filtering the data , e.g. [1, 5]
                            will match all the cascades with 1keV to 5keV, [1, 1] will only match 1keV,
                            ["", 5] will match less than 5keV (default: ["", ""])

    archiveName (optional): archive name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    prefix (optional): a list of prefixes for xyz files in the archive, only files that start with one of the prefixes will be included in processing. (default: [])
    suffix (optional): a list of suffixes for xyz files in the archive, only files that end with one of the suffixes will be included in processing. (default: ["xyz"])
    excludePrefix (optional): a list of prefixes for non xyz files in the archive, files that start with one of these prefixes will NOT be included in processing. (default: ["init", "."])
    excludeSuffix (optional): a list of suffixes for non xyz files in the archive, files that end with one of the suffixes will NOT be included in processing. (default: [""])
    """
    if not(os.path.isdir(baseDirPathToDownload) and os.path.exists(baseDirPathToDownload)):
        return False, "Error in directory path"
    fileNumbers = _extractXmlNumbers(_cookQueryUrl(
        author, doi, material, tempRange, energyRange, archiveName))
    print(len(fileNumbers), "satisfy the query.")
    metaFilePaths = []
    xyzDirs = []
    for fileNumber in fileNumbers:
        metaFilePath = _downloadMetaFileXml(fileNumber, baseDirPathToDownload)
        xyzDir = _downloadAndUnzipXyzFiles(metaFilePath, baseDirPathToDownload)
        metaFilePaths.append(metaFilePath)
        xyzDirs.append(xyzDir)
    res = []
    for metaFilePath, xyzDir in zip(metaFilePaths, xyzDirs):
        isSuccess, curRes = processXyzFilesInDirGivenMetaFile(
            metaFilePath, xyzDir, config, len(res) + idStartIndex, onlyProcessTop, prefix, suffix, excludePrefix, excludeSuffix)
        if (isSuccess):
            res += curRes
        else:
            print(curRes)
        if onlyProcessTop > 0 and len(res) >= onlyProcessTop: break
    return True, res


# other utility functions for helping with processing cascades db
# -------------------------------------------------


def queryCdbForMetaFileNumbers(author="", doi="", material="", tempRange=["", ""], energyRange=["", ""], archiveName=""):
    """queries cascades from cascadesDB.

    Returns
    -------
    a list of meta file numbers that satisfy the input filters

    Parameters
    -----------
    author (optional): author name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    doi (optional): doi to look for in the cascadesDB (default: "")
    material (optional): material to look for in the cascadesDB e.g. Fe, W (default: "")
    tempRange (optional): a list with two values of temperature in Kelvin for more than 
                          equal to and less than equal for filtering the data , e.g. [300, 500]
                          will match all the cascades with 300K to 500K, [300, 300] will only match 300K,
                          ["", 500] will match less than 500K (default: ["", ""])
    energyRange (optional): a list with two values of energy of PKA in keV for more than 
                            equal to and less than equal for filtering the data , e.g. [1, 5]
                            will match all the cascades with 1keV to 5keV, [1, 1] will only match 1keV,
                            ["", 5] will match less than 5keV (default: ["", ""])

    archiveName (optional): archive name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    """

    return _extractXmlNumbers(_cookQueryUrl(author, doi, material, tempRange, energyRange, archiveName))


def queryCdbTodownloadMetaFilesXml(baseDirPathToDownload, author="", doi="", material="", tempRange=["", ""], energyRange=["", ""], archiveName=""):
    """queries cascades from cascadesDB.

    Returns
    -------
    a list with meta file paths that satisfy the input filters

    Parameters
    -----------
    baseDirPathToDownload : directory path where all the meta files and xyz files will be
                            downloaded, keeping the same directory for download will make sure that the
                            data is downloaded only once
    author (optional): author name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    doi (optional): doi to look for in the cascadesDB (default: "")
    material (optional): material to look for in the cascadesDB e.g. Fe, W (default: "")
    tempRange (optional): a list with two values of temperature in Kelvin for more than 
                          equal to and less than equal for filtering the data , e.g. [300, 500]
                          will match all the cascades with 300K to 500K, [300, 300] will only match 300K,
                          ["", 500] will match less than 500K (default: ["", ""])
    energyRange (optional): a list with two values of energy of PKA in keV for more than 
                            equal to and less than equal for filtering the data , e.g. [1, 5]
                            will match all the cascades with 1keV to 5keV, [1, 1] will only match 1keV,
                            ["", 5] will match less than 5keV (default: ["", ""])

    archiveName (optional): archive name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    """

    fileNumbers = _extractXmlNumbers(_cookQueryUrl(
        author, doi, material, tempRange, energyRange, archiveName))
    print(len(fileNumbers), "satisfy the query.")
    metaFilePaths = []
    for fileNumber in fileNumbers:
        filePath = _downloadMetaFileXml(fileNumber, baseDirPathToDownload)
        metaFilePaths.append(filePath)
    return metaFilePaths


def queryCdbTodownloadMetaFilesXmlAndXyz(baseDirPathToDownload, author="", doi="", material="", tempRange=["", ""], energyRange=["", ""], archiveName=""):
    """queries cascades from cascadesDB.

    Returns
    -------
    two lists with meta file paths and corresponding xyz directory that satisfy the input filters

    Parameters
    -----------
    baseDirPathToDownload : directory path where all the meta files and xyz files will be
                            downloaded, keeping the same directory for download will make sure that the
                            data is downloaded only once
    author (optional): author name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    doi (optional): doi to look for in the cascadesDB (default: "")
    material (optional): material to look for in the cascadesDB e.g. Fe, W (default: "")
    tempRange (optional): a list with two values of temperature in Kelvin for more than 
                          equal to and less than equal for filtering the data , e.g. [300, 500]
                          will match all the cascades with 300K to 500K, [300, 300] will only match 300K,
                          ["", 500] will match less than 500K (default: ["", ""])
    energyRange (optional): a list with two values of energy of PKA in keV for more than 
                            equal to and less than equal for filtering the data , e.g. [1, 5]
                            will match all the cascades with 1keV to 5keV, [1, 1] will only match 1keV,
                            ["", 5] will match less than 5keV (default: ["", ""])

    archiveName (optional): archive name to look for in the cascadesDB, comparison is not exact, similar strings are matched (default: "")
    """

    fileNumbers = _extractXmlNumbers(_cookQueryUrl(
        author, doi, material, tempRange, energyRange, archiveName))
    print(len(fileNumbers), "satisfy the query.")
    metaFilePaths = []
    xyzFilePaths = []
    for fileNumber in fileNumbers:
        metaFilePath = _downloadMetaFileXml(fileNumber, baseDirPathToDownload)
        xyzFilePath = _downloadAndUnzipXyzFiles(
            metaFilePath, baseDirPathToDownload)
        metaFilePaths.append(metaFilePath)
        xyzFilePaths.append(xyzFilePath)
    return metaFilePaths, xyzFilePaths


def getInfoFromMeta(metaInfo, metaFilePath, xyzFilePath):
    """ returns info and extrainfo cooked from the input metaFile. These info can be used to process a cascade"""
    info, extraInfo = getDefaultInfos()
    extraInfo["energy"] = float(metaInfo['PKA']['energy']['#text'])  # assuming kev (TODO check)
    extraInfo['simulationTime'] = float(metaInfo['simulation_time']['#text'])
    extraInfo["boxSize"] = float(
        metaInfo['simulation_box']['box_X_length']['#text'])
    extraInfo["substrate"] = metaInfo['material']['formula']
    extraInfo["potentialUsed"] = metaInfo['interatomic_potential']['uri']
    extraInfo["isPKAGiven"] = False
    extraInfo["infile"] = metaFilePath
    extraInfo["tags"] = "[pyscript]"
    info["latticeConst"] = float(
        metaInfo['material']['lattice_parameters']['a']['#text'])
    info["boxSize"] = float(metaInfo['simulation_box']
                            ['box_X_length']['#text'])
    info["ncell"] = -1  # int(extraInfo["boxSize"] / info["latticeConst"])
    info["originType"] = 1
    info["temperature"] = float(metaInfo['initial_temperature']['#text'])
    info["structure"] = metaInfo['material']['structure']
    info["xyzFilePath"] = xyzFilePath
    info["xyzFileType"] = "CASCADESDBLIKECOLS"
    return info, extraInfo	
