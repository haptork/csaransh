#!/usr/bin/env python
# coding: utf-8

# In[1]:


import sys
import math
from .geo import Point, Line, use_degrees
import json
use_degrees()
angular_unit = 180.0/math.pi
debug = False
import numpy as np
from scipy.sparse.csgraph import minimum_spanning_tree
from sklearn.neighbors import NearestNeighbors
from collections import Counter
from scipy.sparse import csr_matrix, lil_matrix

from .lineClusterFeatures import addLineFeat ,getLinesDataOnly, getAllAttrs, getAllHistAttrs, addLinesData, lineFeatsForCluster
# In[1098]:

def classesDataToSave(cluster_labels, show_dim, tag):
    class_points = {}
    class_tags = {}
    for i, label in enumerate(cluster_labels):
        label = str(label)
        if not label in class_points:
            class_points[label] = [[], [], []]
            class_tags[label] = []
        class_points[label][0].append(show_dim[i][0])
        class_points[label][1].append(show_dim[i][1])
        class_points[label][2].append(0)
        class_tags[label].append(tag[i])
    return class_points, class_tags

def getLineViz(histAttrs, tags, cascades):
    features = []
    for x, (tcas, tcid) in zip(histAttrs, tags):
        #curFeat = [x['nMain'] + x['nfreeIs'], x['nSubLine'] * 0.5]
        neighDists = x['neighDists']
        neighDists[1] += neighDists[5]
        neighDists[5] = 0
        sizeToLineRatio = 2.0
        if x['nMain'] > 0:
            sizeToLineRatio = abs(1.0 - (cascades[tcas]['clusterSizes'][tcid] / x['nMain'])) * 2
        curFeat = [sizeToLineRatio]
        #curFeat += (x['neighDistsPar'] / max(1.0, x['neighDistsPar'].sum())).tolist()
        curFeat += (x['neighDists'] / max(1.0, x['neighDists'].sum())).tolist()        
        curFeat += (x['neighAngles'] / max(1.0, x['neighAngles'].sum())).tolist()
        curFeat += (x['dir'].flatten() / max(1.0, x['dir'].sum())).tolist()
        features.append(curFeat)
        #curClass += [str(x['lineTypes']), str(x['neighDists']), str(x['neighAngles'])]
    seed = 3
    show_dim = TSNE(n_components=2, random_state=seed).fit_transform(features).tolist()
    #clusterer = hdbscan.HDBSCAN(min_cluster_size=9)
    return show_dim

def addComponentInfo(lineComponents, curClassName, cascade, tcid):
    unused = 0
    for i, line in enumerate(cascade['features'][tcid]['lines']['lines']):
        if i >= len(lineComponents): continue
        if not 'main' in line: 
            unused += 1
            continue
        color = [int(lineComponents[i - unused][1][0]), int(lineComponents[i - unused][0])]
        line['color'] = color
    cascade['features'][tcid]['lines']['cLinesT'] = []
    for j, line in enumerate(cascade['features'][tcid]['lines']['linesT']):
        i = j + len(cascade['features'][tcid]['lines']['lines'])
        if i >= len(lineComponents): continue
        color = [int(lineComponents[i][1][0]), int(lineComponents[i][0])]
        cascade['features'][tcid]['lines']['cLinesT'].append(color)
    if not 'clusterClasses' in cascade: cascade['clusterClasses'] = {}
    if not 'savi' in cascade['clusterClasses']: cascade['clusterClasses']['savi'] = {}
    cascade['clusterClasses']['savi'][tcid] = curClassName
    """
    di = {'lines':[], 'linesT':[], 'pointsI':lineC['pointsI'], 'pointsV': lineC['pointsV']}
    #print(lineC)
    for line in lineC['lines']:
        if 'parent' in line or 'del' in line: continue
        sub = []
        if line['subLine']: sub = [x[1] for x in line['subLine']['points']]
        di['lines'].append({'main': [x[1] for x in line['mainPoints']], 'sub': sub})
    for line in lineC['linesT']:
        if 'parent' in line or 'del' in line: continue
        di['linesT'].append([x[1] for x in line['mainPoints']])
    data[tcas]['features'][tcid]['lines'] = di
    """
    


from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import connected_components
from collections import Counter
import networkx as nx


# ! get parallel groups, tag big and small of these
# !find group formed of neighbouring random lines, not including lines from big || groups
#   but including small groups if neighbouring
# !remove rnd thresh lines from rnd
# label random lines groups as ring type (90 or 60 degrees 1NN), random
# tag big parallel groups with bulk %, both surface, single plane
# group groups as neighbouring

def filterRingGroupsByTriads(attrs, curIndices, latticeConst):
    matrix1 = attrs['adjacencyDist']
    matrix2 = attrs['adjacencyAng']
    matrix3 = attrs['adjacencyDistParOnly']
    mask = np.full(matrix1.shape[0], False)
    mask[curIndices] = True
    resultMat = np.full(matrix1.shape, False)
    nn1 = latticeConst * 0.8660254
    nn3 = latticeConst * 1.414213
    tol = 0.3
    tolPar = 0.5 * latticeConst
    for r_idx in curIndices:
        distRow = matrix1[r_idx, :].toarray()
        angleRow = matrix2[r_idx, :].toarray()
        distPar = matrix3[r_idx, :].toarray()
        distMask = mask & (distPar < tolPar) & (((distRow < (nn1 + tol)) & (distRow > (nn1 - tol)))
                           | ((distRow < (nn3 + tol)) & (distRow > (nn3 - tol))))
        angleMask = distMask & ((angleRow > 50))
        resultMat[r_idx, :] = angleMask
    resultMat = resultMat | resultMat.T
    G = nx.from_numpy_matrix(resultMat)
    cycles = nx.minimum_cycle_basis(G)
    return cycles
 
def findRandomLabels(attrs, randomIndices, latticeConst):
    matrix1 = attrs['adjacencyDist']
    matrix2 = attrs['adjacencyAngAligned']
    matrix3 = attrs['adjacencyDistPar']
    resultMat  = np.full(matrix1.shape, False)
    #resultMat2  = np.full(matrix1.shape, False)
    mask = np.full(matrix1.shape[0], False)
    mask[randomIndices] = True
    angleTol = 26#18
    nn1 = latticeConst * 0.8660254
    nn2 = latticeConst
    nn3 = latticeConst * 1.414213
    tol = 0.3
    epsilon = 0.000001
    for r_idx in randomIndices:
        distRow = matrix1[r_idx, :].toarray()
        angleRow = matrix2[r_idx, :].toarray()
        distMask = mask & (distRow > epsilon)
        distRow2 = matrix3[r_idx, :].toarray()
        distMask2 = mask & (distRow2 < math.ceil(nn2)) & (distRow2 > epsilon) & (angleRow <= angleTol)
        resultMat[r_idx, :] = distMask | distMask2
    graph = csr_matrix(resultMat)
    n_components, labels = connected_components(csgraph=graph, directed=False, return_labels=True)
    labels[~mask] = -1
    labelSet = set(labels)
    if -1 in labelSet:
        labelSet.remove(-1)
    labelCounts = dict(Counter(labels[:attrs['nMain']]))
    single = []
    par = []
    parRandom = []
    trueRandom = []
    ri = {'names':("single", "par", "parRandom", "random"), 'indices':[]}
    for x in labelSet:
        curIndices = np.arange(len(labels))[labels == x]
        if x not in labelCounts: continue
        if labelCounts[x] < 2:
            single.append(x)
            ri['indices'].append({x: (0, curIndices)})
            continue
        parCount = 0
        nonParCount = 0
        allMask = resultMat | resultMat.T
        for i in curIndices:
            angleRow = matrix2[i, :].toarray()
            countPar = np.count_nonzero((angleRow < 26) & allMask[i, :])
            countNonPar = np.count_nonzero((angleRow >= 26) & allMask[i, :])
            if countNonPar == 0 and countPar > countNonPar: # or >=
                parCount += 1
            else:
                nonParCount += 1
        typ = 1
        if nonParCount == 0 and parCount > nonParCount:
            par.append(x)
            typ = 1
        elif float(parCount) / float(nonParCount) > 0.8:
            parRandom.append(x)
            typ = 2
        else:
            trueRandom.append(x)
            typ = 3
        ri['indices'].append({x: (typ, curIndices)})
    return labels, par, parRandom, trueRandom, single, ri

def tagBigSmallStray(attrs, labels, multiplier = 15, maxLimit = 3):
    minLinesInGroup = 1
    big = set()
    small = set()
    single = set()
    stray = set()
    if attrs['nMain'] < 1 or len(labels) == 0: return big, small, single, stray
    if attrs['nMain'] == 1 and len(labels) > 0: return big, small, labels[0], stray
    if attrs['nMain'] > multiplier:
        minLinesInGroup = math.ceil(attrs['nMain'] / multiplier)
        if minLinesInGroup > maxLimit: minLinesInGroup = maxLimit
    labelCounts = dict(Counter(labels[:attrs['nMain']]))
    for label in set(labels):
        if label not in labelCounts:
            stray.add(label)
        elif labelCounts[label] == 1:
            single.add(label)
        elif labelCounts[label] > minLinesInGroup:
            big.add(label)
        else:
            small.add(label)
    return big, small, single, stray

def definitelyParNonParGroup(countPar, countNonPar, countStrict, countStrictNon, componentLabels):
    allLabels = set(componentLabels)
    parNonParCounts = {}
    parLabels = []
    nonParLabels = []
    npIndices = []
    isAnyBig = False
    #print(componentLabels)
    for label in allLabels:
        indices = np.arange(len(componentLabels))[componentLabels == label]
        #print(label, indices)
        if len(indices) > 2:
            isAnyBig = True
        parCount = 0
        nonParCount = 0
        #debug = label == 4
        for i in indices:
            #print("cP, cNp, cs, csN", countPar[i], countNonPar[i], countStrict[i], countStrictNon[i])
            if (countPar[i] > countNonPar[i] and countPar[i] >= 1):
                parCount += 1
            else:
                nonParCount += 1
        #print("par, non", parCount, nonParCount)
        parNonParCounts[label] = (parCount > 0 and parCount > nonParCount, parCount, nonParCount)
        if parNonParCounts[label][0] or len(indices) >= 3 or len(indices) > 0.5 * len(componentLabels):
            parLabels.append(label)
            for i in indices: npIndices.append(i)
        else:
            nonParLabels.append(label)
    return parNonParCounts, parLabels, nonParLabels, npIndices, isAnyBig

def checkDiInterstitialRing(curAttrs, ri, latticeConst, countPar):
    nn1 = latticeConst * 0.8660254
    nn3 = latticeConst * 1.414213
    if countPar[ri[0]] > 0 or countPar[ri[1]] > 0: return False
    if curAttrs['nPoints'][ri[0]] > 3 and curAttrs['nPoints'][ri[1]] > 3: return False
    if curAttrs['nPoints'][ri[0]] > 5 or curAttrs['nPoints'][ri[1]] > 5: return False
    isIt = abs(curAttrs['adjacencyDist'][ri[0],ri[1]] - nn1) < 0.01 or abs(curAttrs['adjacencyDist'][ri[0],ri[1]] - nn3) < 0.01 
    angleTol = 60
    tolPar = nn1
    return isIt and curAttrs['adjacencyAng'][ri[0],ri[1]] > angleTol and curAttrs['adjacencyDistParOnly'][ri[0],ri[1]] < tolPar

def getParallelGroups(attrs, latticeConst):
    matrix1 = attrs['adjacencyDistPar']
    matrix2 = attrs['adjacencyAngAligned']
    matrix3 = attrs['adjacencyLineType']
    matrix4 = attrs['adjacencyDist']
    resultMat  = np.full(matrix1.shape, False, dtype=bool)
    resultMat2  = np.full(matrix1.shape, False, dtype=bool)
    resultMat3  = np.full(matrix1.shape, False, dtype=bool)
    angleTol = 26#18
    angleBlocking = 45#18
    nn1 = math.ceil(latticeConst * 0.8660254)
    nn2 = math.ceil(latticeConst)
    nn3 = math.ceil(latticeConst * 1.414213)
    epsilon = 1e-6
    if attrs['nMain'] < 2: return resultMat, 0, np.array([]), np.array([]), np.array([]), np.array([]), np.array([])
    for r_idx in range(resultMat.shape[0]):
        angRow = matrix2[r_idx, :].toarray()
        distRow = matrix1[r_idx, :].toarray()
        angleMask = (angRow <= angleTol) & (distRow > epsilon) & (matrix3[r_idx, :]).toarray()
        #angleMask = (angRow <= angleTol).minimum(distRow > epsilon).minimum(matrix3[r_idx, :])
        angleMaskedDist = distRow[angleMask]
        if angleMaskedDist.shape[0] >= 1:
            minimum = math.ceil(max(min(angleMaskedDist), nn2) + 0.001)
            resultMat[r_idx, :] = (angleMask) & (distRow <= minimum)
            #resultMat[r_idx, :] = (angleMask).minimum(distRow <= minimum)
        distRow2 = matrix4[r_idx, :].toarray()
        #resultMat4[r_idx, :] =(distRow2 <= nn1) & (distRow2 > epsilon) & (angRow < angleTolStrict)
        resultMat2[r_idx, :] =(distRow2 <= nn3) & (distRow2 > epsilon) & (angRow > angleTol)
        resultMat3[r_idx, :] = resultMat2[r_idx, :] & (angRow > angleBlocking)
        #resultMat2[r_idx, :] =(distRow2 <= nn3).minimum(distRow2 > epsilon).minimum(angRow > angleTol)
        #resultMat3[r_idx, :] = resultMat2[r_idx, :].minimum(angRow > angleBlocking)
    graph = csr_matrix(resultMat)
    graph = resultMat
    n_components, labels = connected_components(csgraph=graph, directed=False, return_labels=True)
    resultMatAll = resultMat | resultMat.T
    resultMat2 = resultMat2 | resultMat2.T
    resultMat3 = resultMat3 | resultMat3.T
    #resultMatAll = resultMat.maximum(resultMat.T)
    #resultMat2 = resultMat2.maximum(resultMat2.T)
    #resultMat3 = resultMat3.maximum(resultMat3.T)
    #resultMat4 = resultMat4 | resultMat4.T
    countPar = np.zeros(resultMat.shape[0])
    countNonPar = np.zeros(resultMat.shape[0])
    countBlocking = np.zeros(resultMat.shape[0]) 
    countStrictPar = np.zeros(resultMat.shape[0]) 
    countStrictNonPar = np.zeros(resultMat.shape[0]) 
    for r_idx in range(resultMat.shape[0]):
        countPar[r_idx] = np.count_nonzero(resultMatAll[r_idx, :])
        countNonPar[r_idx] = np.count_nonzero(resultMat2[r_idx, :])
        countBlocking[r_idx] = np.count_nonzero(resultMat3[r_idx, :])
    return resultMat, countStrictPar, countStrictNonPar, labels, countPar, countNonPar, countBlocking

def filterRingGroups1(attrs, probLabelNames, labels, latticeConst):
    matrix1 = attrs['adjacencyDist']
    matrix2 = attrs['adjacencyAng']
    matrix3 = attrs['adjacencyDistPar']
    mask = np.isin(labels, list(probLabelNames))
    strayIndices = np.arange(len(labels))[mask]
    resultMat = np.full(matrix1.shape, False, dtype=bool)
    resultMat2 = np.full(matrix1.shape, False, dtype=bool)
    nn1 = latticeConst * 0.8660254
    nn3 = latticeConst * 1.414213
    tol = 0.3
    ang1 = 60
    ang2 = 90
    tol2 = 12 
    roots = []
    neighs = []
    grouping = np.zeros(len(labels), dtype=int)
    groupingPar = np.zeros(len(labels), dtype=int)
    epsilon = 1e-6
    for r_idx in range(len(labels)):#strayIndices:
        distRow = matrix1[r_idx, :].toarray()
        angleRow = matrix2[r_idx, :].toarray()
        distRow2 = matrix3[r_idx, :].toarray()
        #distParOnly = matrix4[r_idx, :]
        # TODO: instead of masking might include neighs for better results
        distMask = (((distRow < (nn1 + tol)) & (distRow > (nn1 - tol)))
                           | ((distRow < (nn3 + tol)) & (distRow > (nn3 - tol))))
        #angleMask2 = distMask & (distParOnly < tolDistParOnly) & ((angleRow > 46))
        angleMask2 = distMask & ((angleRow > 46))
        angleMask1 = distMask & (((angleRow < (ang1 + tol2)) & (angleRow > (ang1 - tol2)))
                           | (angleRow > (ang2 - tol2)))
        distMask2 = (distRow2 > epsilon) & (distRow2 < math.ceil(nn1)) & (angleRow < 27)
        if r_idx in strayIndices:
            resultMat[r_idx, :] = angleMask2
            resultMat2[r_idx, :] = distMask2
        else:
            resultMat[r_idx, :] = mask & angleMask2
            resultMat2[r_idx, :] = mask & distMask2
        #resultMat3[r_idx, :] = angleMask1
    for r_idx in strayIndices:
        grouping[r_idx] = np.count_nonzero(resultMat[r_idx, :] | resultMat[:, r_idx])
    for r_idx in strayIndices:
        groupingPar[r_idx] = np.count_nonzero(resultMat2[r_idx, :] | resultMat2[:, r_idx])
    minGroupingTol = min(len(strayIndices) - 1, 2)
    maxGroupingTol = 6
    graph = csr_matrix(resultMat)
    n_components, ringLabels = connected_components(csgraph=graph, directed=False, return_labels=True)
    toDel = set()
    labelCounts = dict(Counter(ringLabels[:attrs['nMain']]))
    ringSet = set(ringLabels)
    for x in ringSet:
        if x not in labelCounts or labelCounts[x] == 1:
            toDel.add(x)
    ringSet = ringSet.difference(toDel)
    ringIndices = []
    ringLabelNames = []
    toDel = set()
    for x in ringSet:
        curRingIndices = np.arange(len(ringLabels))[ringLabels == x]
        nGood = len(curRingIndices)
        for y in curRingIndices:
            if grouping[y] > len(curRingIndices): nGood -= 1
        if nGood < len(curRingIndices) * 0.75: toDel.add(x)
        if x not in toDel:
            ringIndices.append(curRingIndices)
            ringLabelNames.append(x)
    ringSet = ringSet.difference(toDel)
    return resultMat, ringLabels, ringLabelNames, grouping, groupingPar, ringIndices
    
def findRings(curAttrs, ri, latticeConst, probRingNames, countPar):
    ringNames = []
    ringIndices = []
    for (curIndices, curName) in zip(ri, probRingNames):
        mnSize = curIndices.shape[0]
        if mnSize == 2:
            isRing = checkDiInterstitialRing(curAttrs, curIndices, latticeConst, countPar)
            if isRing:
                ringNames.append(curName)
                ringIndices += curIndices.tolist()
                continue
        cycle = filterRingGroupsByTriads(curAttrs, curIndices, latticeConst)
        len3 = 0
        for x in cycle:
            #print(x)
            if qualifyCycle(x, curAttrs, latticeConst): 
                #print(x)
                len3 += 1
            #print(len3, len(cycle))
        if len3 > 0 and len3 >= len(cycle) * 0.4: 
            #print('ring indices', curIndices)
            ringNames.append(curName)
            ringIndices += curIndices.tolist()
        elif curIndices.shape[0] == 3:
            isThresh = -1
            for i, index in enumerate(curIndices):
                if index >= curAttrs['nMain']: isThresh = i
            if isThresh > -1:
                if isThresh != 2:
                    temp = curIndices[isThresh]
                    curIndices[isThresh] = curIndices[2]
                    curIndices[2] = temp
                isRing = checkDiInterstitialRing(curAttrs, curIndices, latticeConst, countPar)
                if isRing:
                    #print('di-ring indices', curIndices)
                    ringNames.append(curName)
                    ringIndices += curIndices.tolist()
    return ringNames, ringIndices
    
def findComponents(curAttrs, cascade):
    latticeConst = cascade['latticeConst']
    adjPar, countStrictPar, countStrictNon, parLabels, countPar, countNonPar, countBlocking = getParallelGroups(curAttrs, latticeConst)
    pNpCounts, pLabels, npLabels, ppIndices, isAnyBig = definitelyParNonParGroup(countPar, countNonPar, countStrictPar, countStrictNon, parLabels)
    if len(npLabels) > 0:
        adjRing, ringLabels, probRingNames, g1, g2, ri = filterRingGroups1(curAttrs, npLabels, parLabels, latticeConst)
        ringLabelNames, ringIndices = findRings(curAttrs, ri, latticeConst, probRingNames, countPar)
        mask = np.isin(parLabels, list(npLabels))
        mask[ringIndices] = False
        randomIndices = np.arange(len(parLabels))[mask]
        if len(pLabels) > 0 and len(ppIndices) > 4:# isAnyBig:
            countIfTol = 2 #else 3
            parLabelInRandom = parLabels[randomIndices]
            newNpLabels = list()
            for curLabel in npLabels:
                #print("curLabel, parLIR", curLabel, parLabelInRandom)
                whatever = parLabelInRandom == curLabel
                isGreaterThanTol = np.count_nonzero(whatever) >= countIfTol
                if isGreaterThanTol:
                    pLabels.append(curLabel)
                else:
                    newNpLabels.append(curLabel)
            npLabels = newNpLabels
            mask = np.isin(parLabels, list(npLabels))
            mask[ringIndices] = False
            randomIndices = np.arange(len(parLabels))[mask]
        randomLabels, par, parRandom, trueRandom, single, randomIndices = findRandomLabels(curAttrs, randomIndices, latticeConst)
        offset = 0 
        if len(randomLabels) > 0: offset = max(randomLabels) + 1
        ringLabelNames += offset
        for x, y in zip(ri, probRingNames):
            if y + offset not in ringLabelNames: continue
            randomLabels[x] = y + offset
            parLabels[x] = y + offset
        offset = 0
        if len(randomLabels) > 0: offset = max(randomLabels) + 1
        parLabels += offset
        pLabels += offset
        mask = np.isin(parLabels, list(pLabels))
        mask[ringIndices] = False
        randomLabels[mask] = parLabels[mask]
        big, small, single, stray = tagBigSmallStray(curAttrs, randomLabels)
        result1 = {}
        result2 = [[], [], [], [], [], []]
        for i, y in enumerate([pLabels, ringLabelNames, trueRandom, parRandom]):
            for x in y:
                if x in single or x in stray: continue
                isMain = x in big
                result1[x] = (i, isMain)
                result2[i].append([isMain, x, np.arange(len(randomLabels))[randomLabels==x]])
        for x in par:
            if x in single or x in stray: continue
            isMain = x in big
            result1[x] = (0, isMain)
            result2[0].append([isMain, x, np.arange(len(randomLabels))[randomLabels==x]])
        for x in single: 
            result1[x] = (4, False)
            result2[4].append([False, x, np.arange(len(randomLabels))[randomLabels==x]])
        for x in stray: 
            result1[x] = (5, False)
            result2[5].append([False, x, np.arange(len(randomLabels))[randomLabels==x]])
        result3 = [(x, result1[x], (p, q, r)) for x, p, q, r in zip(randomLabels, countPar, countNonPar, countBlocking)]
        extraRings = 0
        if curAttrs['nfreeIs'] > 0 and len(pLabels) <= 1 and extraRings < 1 and len(trueRandom) <= 1:
            extraRings = 1
        return (result1, result2, result3, extraRings)
    else:
        big, small, single, stray = tagBigSmallStray(curAttrs, parLabels)
        result1 = {}
        result2 = [[], [], [], [], [], []]
        for x in big: 
            result1[x] = (0, True)
            result2[0].append([True, x, np.arange(len(parLabels))[parLabels==x]])
        for x in small: 
            result1[x] = (0, False)
            result2[0].append([False, x, np.arange(len(parLabels))[parLabels==x]])
        for x in single: 
            result1[x] = (4, False)
            result2[4].append([False, x, np.arange(len(parLabels))[parLabels==x]])
        for x in stray: 
            result1[x] = (5, False)
            result2[5].append([False, x, np.arange(len(parLabels))[parLabels==x]])
        result3 = [(x, result1[x], (p, q, r)) for x, p, q, r in zip(parLabels, countPar, countNonPar, countBlocking)]
        nRings = 1 if curAttrs['nfreeIs'] > 0 and len(big) < 2 else 0
        return (result1, result2, result3, nRings)

def acceptableBlocking(components, lines):
    mainComponentSize = 0.0
    for x in components[0]:
        if x[0]:
            mainComponentSize = len(x[2])
            break
    blockingSmallComps = 0.0
    for compTypeList in components:
        for comp in compTypeList:
            if not comp[0]: # small
                for lineId in comp[2]: #all line ids
                    if lines[lineId][2][2] > 0: blockingSmallComps += 1.0
    bias = 10
    ratio = 8 # 1 every ratio after bias
    if blockingSmallComps == 0: return 0, True
    curAllowed = (mainComponentSize - bias) / ratio
    return blockingSmallComps, blockingSmallComps <= curAllowed

def calcBiggest(components):
    res = 0
    for comp in components:
        if len(comp[2]) > res: 
            res = len(comp[2])
    return res

def calcBiggestDislocation(components):
    res = 0
    orient = 0
    for comp in components:
        if len(comp[2]) > res: 
            res = len(comp[2])
            orient = comp[3]
    return res, orient

def calcCentComps(components):
    res = np.zeros(len(components))
    total = 0
    for i, compTypeList in enumerate(components):
        for comp in compTypeList:
            res[i] += len(comp[2])
            total += len(comp[2])
    if total != 0: res /= total
    return res

def findComponentClass(attrs, labels, components, lines, ringFreeIs):
    sizesMain = np.array([len(list(filter(lambda y: y[0], x))) for x in components])
    centComps = calcCentComps(components)
    allParSizes = [(len(x[2]), x[3]) for x in components[0]]
    allParSizes.sort(key=lambda x: x[0], reverse=True)        
    allRingSizes  = [len(x[2]) for x in components[1]]
    allRingSizes.sort(reverse=True)
    allRandomSizes  = [len(x[2]) for x in components[2]]
    allRandomSizes.sort(reverse=True)
    onlyParallel = sizesMain[0] > 0 and all(sizesMain[1:] == 0) and centComps[0] > 0.7
    nLineTol = 15
    #print(centComps)
    #print(sizesMain)
    if onlyParallel:
        alone = (sizesMain[0] == 1 or allParSizes[1][0] / allParSizes[0][0] < 0.2 and allParSizes[1][0] < 6)
        if alone:
            if ringFreeIs > 0:
                if allParSizes[0][0] < 20: return (2, "c", "||@") # TODO check type of ring
            orientV = allParSizes[0][1]['verdict']
            if orientV == 3: return (1, "a", "||")
            if orientV == 1: return (1, "b", "||-!")
            return (4, "a", "#")
        return (1, "c", "||//")
    sizesAny = np.array([len(x) for x in components[0:3]])
    maxRingSize = 0 if len(allRingSizes) == 0 else allRingSizes[0]
    maxRandomSize = 0 if len(allRandomSizes) == 0 else allRandomSizes[0]
    #print(centComps)
    if ringFreeIs > 0 and attrs['nMain'] < 3: return (3, "a", "@")
    if centComps[1] > 0.4:
        #print("cent ring:", centComps[0], centComps[1], sum(centComps[2:]))
        if centComps[1] < 0.75 and centComps[0] > sum(centComps[2:]): return (3, "b", "@||")
        if centComps[1] < 0.75 and sum(sizesAny[2:]) >= 3: return (3, "c", "@#")
        return (3, "a", "@")
    #print('mxP', maxParSize)
    if centComps[0] > 0.4 and allParSizes[0][0] > 2:
        #print("cent par:", centComps[0], centComps[1], centComps[2], sum(centComps[3:]))
        if (ringFreeIs > 0 and sum(centComps[2:]) < 0.2) or centComps[1] > 0.2 or maxRingSize > 2:
            return (2, "c", "||@")
        stillParallel = len(allParSizes) > 0 and maxRingSize / allParSizes[0][0] < 0.4
        stillParallel = stillParallel and maxRandomSize / allParSizes[0][0] < 0.4 and allParSizes[0][0] > 3
        stillParallel = maxRandomSize < 5 and maxRingSize < 3
        if stillParallel:
            if centComps[2] > 0.5 and sizesMain[2] > 0: return (4, "c", "||#")
            if sizesMain[0] > 1: return (1, "c", "||//")
            orient = allParSizes[0][1]['verdict']
            if orient == 1: return (1, "b", "||-!")
            if orient == 3: return (1, "a", "||")        
            return (4, "c", "||#")            
        if (sizesAny[1] > 0) and centComps[1] <= 0.2:  return (2, "c", "||#")
    if (centComps[0] < 0.2 or allParSizes[0][0] <= 2) and sizesMain[1] == 0 and ringFreeIs == 0: return (4, "a", "#")
    if sizesAny[1] > 0 or ringFreeIs > 0: return (4, "b", "#@")
    if sizesMain[0] == 1:  return (4, "c", "||#")
    if sizesMain[0] > 1: return (1, "c", "||//")
    return (5, "a", "?")

def findComponentClassOld(attrs, labels, components, lines, ringFreeIs):
    sizesMain = np.array([len(list(filter(lambda y: y[0], x))) for x in components])
    centComps = calcCentComps(components)
    onlyParallel = sizesMain[0] > 0 and all(sizesMain[1:] == 0) and centComps[0] > 0.7
    nLineTol = 15
    #print(centComps)
    #print(sizesMain)
    if onlyParallel:
        alone = (sizesMain[0] == 1)
        if not alone:
            biggestSize = 0.0
            secondBiggest = 0.0
            for comp in components[0]:
                if len(comp[2]) > biggestSize: 
                    secondBiggest = biggestSize
                    biggestSize = len(comp[2])
                elif len(comp[2]) > secondBiggest:
                    secondBiggest = len(comp[2])
            #print("bigs", biggestSize, secondBiggest)
            alone = (secondBiggest / biggestSize) < 0.2 and secondBiggest < 6
        #print("alone", alone)
        if alone:
            if ringFreeIs > 0: 
                biggestSize = calcBiggest(components[0])
                #print('bigSize', biggestSize)
                if biggestSize < 30: return (2, "c", "||@")
            blockingSmallComps, isAcceptable = acceptableBlocking(components, lines)
            if blockingSmallComps == 0: return (1, "a", '||')
            if isAcceptable: return (1, "a", '||')#(1, "b", "|!")
            return (2, "a", "||=!")
        else: return (2, "b", "||//")
    sizesAny = np.array([len(x) for x in components[0:3]])
    maxRingSize = calcBiggest(components[1])
    #print(centComps)
    if ringFreeIs > 0 and attrs['nMain'] < 3: return (3, "a", "@")
    if centComps[1] > 0.4:
        #print("cent ring:", centComps[0], centComps[1], sum(centComps[2:]))
        if centComps[1] < 0.75 and centComps[0] > sum(centComps[2:]): return (3, "b", "@||")
        if centComps[1] < 0.75 and sum(sizesAny[2:]) >= 3: return (3, "c", "@#")
        return (3, "a", "@")
    maxParSize = calcBiggest(components[0])
    #print('mxP', maxParSize)
    if centComps[0] > 0.4 and maxParSize > 2:
        #print("cent par:", centComps[0], centComps[1], centComps[2], sum(centComps[3:]))
        if centComps[1] > 0.2 or maxRingSize > 2: return (2, "c", "||@")
        if (sizesAny[1] > 0) and centComps[1] <= 0.2:  return (2, "c", "||#")
        if (ringFreeIs > 0) and sum(centComps[2:]) < 0.2: return (2, "c", "||@")
        if sizesAny[1] == 0:
            if centComps[0] < 0.7 and sizesMain[0] == 1 and sum(sizesMain) > 1: return (4, "c", "||#")
            if centComps[2] < 0.25 and sizesMain[0] == 1: return (2, "a", "||=!")
            if centComps[2] > 0.5 and sizesMain[2] > 0: return (4, "c", "||#")
            if sizesMain[0] > 1: return (2, "b", "||//")
            return (2, "a", "||=!")
    if (centComps[0] < 0.2 or maxParSize <= 2) and sizesMain[1] == 0 and ringFreeIs == 0: return (4, "a", "#")
    if sizesAny[1] > 0 or ringFreeIs > 0: return (4, "b", "#@")
    if sizesMain[0] == 1:  return (4, "c", "||#")
    if sizesMain[0] > 1: return (2, "b", "||//")
    return (5, "a", "?")            
#compType = {0:'par', 1:'rings', 2:'random', 3:'parRandom', 4:'single', 5:'stray'}
#compSizeType = {False:'small', True:'main'}

def qualifyCycle(indices, attrs, latticeConst):
    if len(indices) > 3: return False
    for i in indices:
        if attrs['nPoints'][i] > 3: 
            return False
    return True

    """
    names = [(1, "a", "||"), (2, "a", "||-!"), (2, "b", "||//"), (2, "c", "||@"),
            (3, "a", "@"), (3, "b", "@||"), (3, "c", "@#"), (4, "a", "#"),
            (4, "b", "#@"), (4, "c", "||#"), (5, "a", "?")]
    namesDi = {(x[0], x[1]): i for i, x in enumerate(names)}
    """

def findDislocationDirection(comp, lines):
    res = {'total': [0, 0, 0, 0], 'ratio': [0, 0, 0, 0], 'verdict':()}
    for curLineIndex in comp[2]:
        if curLineIndex >= len(lines): continue
        val = sum(lines[curLineIndex]['forceAlign']['type1'])
        res['total'][val] += 1
    lineCount = sum(res['total'])
    if lineCount == 0: return res
    for i in range(len(res['ratio'])):
        res['ratio'][i] = res['total'][i] / lineCount
    maxIndex = max(range(len(res['total'])), key=res['total'].__getitem__)
    res['verdict'] = maxIndex
    return res
    

def addFullComponentInfo(cascade, cid):
    if cascade['clusterSizes'][cid] < 2: return
    linesData = lineFeatsForCluster(cascade, cid)
    addLineFeat(cascade, cid, linesData)
    attrs = cookLineAttrs(cascade, cid, linesData['lines'], linesData['pointsI'])
    components = findComponents(attrs, cascade)
    for comp in components[1][0]:
        comp.append(findDislocationDirection(comp, linesData['lines']))
    curClass = findComponentClass(attrs, *components)
    curClassName = '-'.join([str(x) for x in curClass])
    addComponentInfo(components[2], curClassName, cascade, cid)

def getSaviDetails(cascade, cid):
    if cascade['clusterSizes'][cid] < 2: return
    linesData = lineFeatsForCluster(cascade, cid)
    attrs = cookLineAttrs(cascade, cid, linesData['lines'], linesData['pointsI'])
    components = findComponents(attrs, cascade)
    for comp in components[1][0]:
        comp.append(findDislocationDirection(comp, linesData['lines']))
    return linesData, attrs, components


######-------
"""
def getLineTypeVal(val):
    sumVal = sum(val)
    if 2 in val: return 3
    elif sumVal == 3: return 0
    elif sumVal == 2: return 2
    elif sumVal == 1: return 1
    return 3
    
classUpdateData = {}
for i, (classLabel, (tcas, tcid)) in enumerate(zip(forms['pot-cmp']['classNumber'], classData['pot-cmp']['tags'])):
    cascade = classData['pot-cmp']['cascades'][tcas]
    if not int(cascade['energy']) in curEnergy: continue
    curSize = cascade['clusterSizes'][tcid]
    if curSize <= 1: continue
    if classLabel > 2: continue
#    if curSize < 15: continue
    #lineComps = forms['pot-cmp']['components'][i][2]
    pot = cascade['potentialUsed']
    if classLabel == 5: classLabel = 3
    if classLabel == 8: classLabel = 6
    #for plotData in (plotDataPrimary, plotDataSecondary, plotDataTertiary):
    #    if classLabel not in plotData: plotData[classLabel] = {'JW':[], 'M-S':[], 'DND-BN':[]}
    biggest = -1
    bigSize = 0
    curPickSize = 0
    for comp in forms['pot-cmp']['components'][i][1][0]:
        if len(comp[2]) > curPickSize:
            curPickSize = len(comp[2])
            biggest = comp[1]
    secondBig = -1
    bigSize = curPickSize
    secSize = 0
    if classLabel == 2:
        curPickSize = 0
        for comp in forms['pot-cmp']['components'][i][1][0]:
            if comp[1] == biggest: continue
            if len(comp[2]) > curPickSize:
                curPickSize = len(comp[2])
                secondBig = comp[1]           
        secSize = curPickSize
    bigCompType = {0: 0, 1: 0, 2: 0, 3: 0}
    secCompType = {0: 0, 1: 0, 2: 0, 3: 0}
    for compIndex, comp in enumerate(forms['pot-cmp']['components'][i][1][0]):
        if comp[1] == biggest or comp[1] == secondBig:
            for curLineIndex in comp[2]:
                if curLineIndex >= len(classData['pot-cmp']['lines'][i]['lines']): continue
                val = classData['pot-cmp']['lines'][i]['lines'][curLineIndex]['forceAlign']['type1']
                val = getLineTypeVal(val)
                if comp[1] == biggest: bigCompType[val] += 1
                else: secCompType[val] += 1
    finalBig = 3
    secBig = 3
    curPickSize = 0
    for x in bigCompType:
        if bigCompType[x] > curPickSize:
            curPickSize = bigCompType[x]
            finalBig = x
    curPickSize = 0
    for x in secCompType:
        if secCompType[x] > curPickSize:
            curPickSize = secCompType[x]
            secBig = x
    if classLabel == 0:
        if finalBig == 1: 
            print('from || to ||-!', cascade['id'], tcid)
            forms['pot-cmp']['classNumber'][i] = 1
        elif finalBig == 2:
            forms['pot-cmp']['classNumber'][i] = 7
            print('from || to #', cascade['id'], tcid)
    elif classLabel == 1:
        if finalBig == 0:
            if curSize - bigSize > 5:
                forms['pot-cmp']['classNumber'][i] = 2
                print('from ||-! to ||//', cascade['id'], tcid)
            else:
                forms['pot-cmp']['classNumber'][i] = 7
                print('from ||-! to #', cascade['id'], tcid)
        elif finalBig == 2:
            forms['pot-cmp']['classNumber'][i] = 7
            print('from ||-! to #', cascade['id'], tcid)
       
names = ['pot-cmp']
for x in names:
    classData[x]['classes'] = []#classData[x]['classes'][:2]
    classData[x]['classes'].append({'name': 'line-hist', 'data': classesDataToSave(forms[x]['class'], classData[x]['show_dim'], classData[x]['tags'])})
"""

def cookLineAttrs(cascade, cid, clusterLines, freeIs):
    totalLines = 0
    for i, line in enumerate(clusterLines):
        if 'parent' in line or 'del' in line: continue
        totalLines += 1
    """
    res = {}
    if totalLines < 1:
        res = {'nPoints': np.zeros(totalLines),
               'nMain': 0,
               'nfreeIs': len(freeIs),
               'adjacencyDist': np.zeros((totalLines, totalLines), dtype=np.float32),
               'adjacencyDistParOnly': np.zeros((totalLines, totalLines), dtype=np.float32), 
               'adjacencyDistPar': np.zeros((totalLines, totalLines), dtype=np.float32),
               'adjacencyAng': np.zeros((totalLines, totalLines), dtype=np.int8),
               'adjacencyAngAligned': np.zeros((totalLines, totalLines), dtype=np.int8),
               'adjacencyLineType': np.zeros((totalLines, totalLines), dtype=bool)
        }
    else:
    """
    res = {'nPoints': np.zeros(totalLines, dtype=np.int8),
           'nMain': 0,
           'nfreeIs': len(freeIs),
           'adjacencyDist': lil_matrix((totalLines, totalLines), dtype=np.float32),
           'adjacencyDistParOnly': lil_matrix((totalLines, totalLines), dtype=np.float32), 
           'adjacencyDistPar': lil_matrix((totalLines, totalLines), dtype=np.float32),
           'adjacencyAng': lil_matrix((totalLines, totalLines), dtype=np.int8),
           'adjacencyAngAligned': lil_matrix((totalLines, totalLines), dtype=np.int8),
           'adjacencyLineType': lil_matrix((totalLines, totalLines), dtype=bool)
    }
    curI = 0
    for i, line in enumerate(clusterLines):
        if 'parent' in line or 'del' in line: continue
        #for angIndex, angleI in enumerate(line['angles']): res['lineAngles'][angIndex].append(angleI)
        res['nPoints'][curI] = len(line['mainPoints'])
        if line['subLine']: res['nPoints'][-1] += len(line['subLine']['points'])
        if 'isVac' in line:  
            res['nMain'] += 1
        distTol = cascade['latticeConst'] * 1.8 # ~ > sqrt(3)
        curJ = 0
        for j, otherLine in enumerate(clusterLines):
            if 'parent' in otherLine or 'del' in otherLine: continue
            if curJ <= curI: 
                curJ += 1
                continue
            areTriplets = ('isVac' in line and 'isVac' in otherLine)
            areTriplets = areTriplets and ('children' not in line and 'children' not in otherLine)
            lineDist = 0.0
            lineDistPar = 0.0   
            lineDistParOnly = 0.0   
            cmpLine1 = line
            cmpLine2 = otherLine
            switch = False
            if line['forceAlign']['err'][0] and otherLine['forceAlign']['err'][0]:
                if len(line['mainPoints']) < len(otherLine['mainPoints']):
                    switch = True
            elif (line['forceAlign']['err'][0] or otherLine['forceAlign']['err'][0]):
                if line['forceAlign']['err'][0] == False:
                    switch = True
            else: #(line['forceAlign']['err'][0] or line['forceAlign']['err'][1]) == False:
                if line['forceAlign']['err'][1] > otherLine['forceAlign']['err'][1]:
                    switch = True
            if switch:
                cmpLine1 = otherLine
                cmpLine2 = line
            cmpLine1Aligned = cmpLine1
            cmpLine2Aligned = cmpLine2
            if cmpLine1['forceAlign']['err'][0]: cmpLine1Aligned = cmpLine1['forceAlign']
            if cmpLine2['forceAlign']['err'][0]: cmpLine2Aligned = cmpLine2['forceAlign']
            lineAng = line['eq'].angle_to(otherLine['eq'])    
            lineAngAligned = cmpLine1Aligned['eq'].angle_to(cmpLine2Aligned['eq'])    
            res['adjacencyAng'][curI,curJ] = lineAng
            res['adjacencyAngAligned'][curI,curJ] = lineAngAligned
            res['adjacencyAng'][curJ,curI] = lineAng
            res['adjacencyAngAligned'][curJ,curI] = lineAngAligned
            if (cmpLine1['forceAlign']['type1'] == cmpLine2['forceAlign']['type1']) or (not(cmpLine1['forceAlign']['err'][0] and cmpLine2['forceAlign']['err'][0]) and cmpLine1['forceAlign']['type1'] == cmpLine2['forceAlign']['type1'] and cmpLine1['forceAlign']['type1'][-1] != 0):
                res['adjacencyLineType'][curI,curJ] = True
                res['adjacencyLineType'][curJ,curI] = True
            lineDistParOnly = cmpLine1Aligned['eq'].distance_to(cmpLine2Aligned['eq'])   
            if areTriplets:
                p1 = Point(cascade['coords'][cascade['clusters'][cid][line['points'][1]]][:3])
                p2 = Point(cascade['coords'][cascade['clusters'][cid][otherLine['points'][1]]][:3])
                lineDist = p1.distance_to(p2)
                if line == cmpLine1:
                    lineDistPar = cmpLine1['forceAlign']['eq'].distance_to(p2)
                else:
                    lineDistPar = cmpLine1['forceAlign']['eq'].distance_to(p1)
            else:
                lineDist = lineDistParOnly
                lineDistPar = lineDist
            if lineDistParOnly > distTol: 
                curJ += 1
                continue
            if not areTriplets:# and not sameForceAligned:
                cmpPoints1 = cmpLine1['subLine']['points'] if cmpLine1['subLine'] else cmpLine1['mainPoints']
                cmpPoints2 = cmpLine2['subLine']['points'] if cmpLine2['subLine'] else cmpLine2['mainPoints']
                extraPoint1 = []
                extraPoint2 = []
                if not cmpLine1['subLine'] and 'isVac' in cmpLine1 and cmpLine1['isVac'] == 0:
                    extraPoint1 = [cmpLine1['points'][1]]
                if not cmpLine2['subLine'] and 'isVac' in cmpLine2 and cmpLine2['isVac'] == 0:
                    extraPoint2 = [cmpLine2['points'][1]]
                cmpLine1 = cmpLine1Aligned
                cmpLine2 = cmpLine2Aligned
                lineDist = distTol * 2
                lineDistPar = distTol * 2
                for x in cmpPoints1 + extraPoint1:
                    if isinstance(x, tuple): x = x[1]
                    curPoint1 = cascade['coords'][cascade['clusters'][cid][x]]
                    if curPoint1[3] == 1: continue
                    for y in cmpPoints2 + extraPoint2:
                        if isinstance(y, tuple): y = y[1] 
                        curPoint2 = cascade['coords'][cascade['clusters'][cid][y]]
                        if curPoint2[3] == 1: continue
                        refPoint = Point(curPoint2[:3])
                        curDist = Point(curPoint1[:3]).distance_to(refPoint)
                        curDistPar = cmpLine1['eq'].distance_to(refPoint)
                        #print(x, y, curPoint1[:3], curPoint2[:3], curDist, lineDist)
                        if curDist < lineDist: lineDist = curDist
                        if curDistPar < lineDistPar: lineDistPar = curDistPar
                for x in cmpPoints2 + extraPoint2:
                    if isinstance(x, tuple): x = x[1]
                    curPoint2 = cascade['coords'][cascade['clusters'][cid][x]]
                    if curPoint2[3] == 1: continue
                    for y in cmpPoints1 + extraPoint1:
                        if isinstance(y, tuple): y = y[1] 
                        curPoint1 = cascade['coords'][cascade['clusters'][cid][y]]
                        if curPoint1[3] == 1: continue
                        refPoint = Point(curPoint1[:3])
                        curDistPar = cmpLine2['eq'].distance_to(refPoint)
                        if curDistPar < lineDistPar: lineDistPar = curDistPar
            if lineDistPar > distTol: 
                curJ += 1
                continue
            if lineDist < 0.000001: lineDist = 0.02
            if lineDistPar < 0.000001: lineDistPar = 0.02
            if lineDistParOnly < 0.000001: lineDistParOnly = 0.02
            if lineDist < distTol: 
                res['adjacencyDist'][curI,curJ] = lineDist
            res['adjacencyDistPar'][curI,curJ] = lineDistPar
            res['adjacencyDistParOnly'][curI,curJ] = lineDistParOnly             
            curJ += 1
        curI += 1
    return res
