#!/usr/bin/env python
# coding: utf-8

# In[1]:


import sys
import math
import seaborn as sns
import matplotlib.pyplot as plt
from .geo import Point, Line, use_degrees, Plane
import json
use_degrees()
angular_unit = 180.0/math.pi
debug = False
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import minimum_spanning_tree
from sklearn.neighbors import NearestNeighbors
from collections import Counter

# In[1098]:

def getLinesData(data):
    finalCluster = []
    attrs = []
    tags = []
    for i, cascade in enumerate(data):
        sorted_clusters = sorted(cascade['clusters'].keys())
        #print(i, cascade['id'])
        triads, pairs = makeLatticeGroups(cascade)
        for cid in sorted_clusters:
            if cascade['clusterSizes'][cid] < 0: continue
            lines, pointLineMap, freeIs = getTriadLines(cascade, cid, triads)
            linesT, pointLineMapT = getThreshLines(cascade, cid, len(lines), pairs)
            extraVs = getExtraFreeVs(cascade, cid, pointLineMap, pointLineMapT)
            freeVs, extraVsMap = addCollinearToTriads(cascade, cid, lines, pointLineMap, linesT, pointLineMapT, extraVs)
            attrs.append(cookAttrs(cascade, cid, lines + linesT, freeIs, extraVs))
            allVs = list(freeVs) + list(extraVs)
            finalCluster.append({'lines':lines, 'linesT':linesT, 'pointsI':freeIs, 'pointsV':allVs})
            tags.append((i, cid))
    return finalCluster, tags, attrs

def getLinesDataOnly(data):
    finalCluster = []
    tags = []
    for i, cascade in enumerate(data):
        sorted_clusters = sorted(cascade['clusters'].keys())
        #print(i, cascade['id'])
        triads, pairs = makeLatticeGroups(cascade)
        for cid in sorted_clusters:
            if cascade['clusterSizes'][cid] < 0: continue
            lines, pointLineMap, freeIs = getTriadLines(cascade, cid, triads)
            linesT, pointLineMapT = getThreshLines(cascade, cid, len(lines), pairs)
            extraVs = getExtraFreeVs(cascade, cid, pointLineMap, pointLineMapT)
            freeVs, extraVsMap = addCollinearToTriads(cascade, cid, lines, pointLineMap, linesT, pointLineMapT, extraVs)
            allVs = list(freeVs) + list(extraVs)
            finalCluster.append({'lines':lines, 'linesT':linesT, 'pointsI':freeIs, 'pointsV':allVs})   
            tags.append((i, cid))
    return finalCluster, tags

def linesForCascade(cascade, cid):
    finalCluster = []
    attrs = []
    tags = []
    if cascade['clusterSizes'][cid] < 0: return None
    triads, pairs = makeLatticeGroups(cascade)
    lines, pointLineMap, freeIs = getTriadLines(cascade, cid, triads)
    for line in lines:
      line['forceAlign'] = forceAlign(line['eq'])
    #linesT, pointLineMapT = getThreshLines(cascade, cid, len(lines), pairs)
    #extraVs = getExtraFreeVs(cascade, cid, pointLineMap, pointLineMapT)
    #freeVs, extraVsMap = addCollinearToTriads(cascade, cid, lines, pointLineMap, linesT, pointLineMapT, extraVs)
    #attrs.append(cookAttrs(cascade, cid, lines + linesT, freeIs, extraVs))
    #allVs = list(freeVs) + list(extraVs)
    return lines#, triads, pairs

def lineFeatsForCluster(cascade, cid):
    if cascade['clusterSizes'][cid] < 0: return None
    triads, pairs = makeLatticeGroups(cascade)
    lines, pointLineMap, freeIs = getTriadLines(cascade, cid, triads)
    for line in lines:
      line['forceAlign'] = forceAlign(line['eq'])
    linesT, pointLineMapT = getThreshLines(cascade, cid, len(lines), pairs)
    extraVs = getExtraFreeVs(cascade, cid, pointLineMap, pointLineMapT)
    freeVs, extraVsMap = addCollinearToTriads(cascade, cid, lines, pointLineMap, linesT, pointLineMapT, extraVs)
    allVs = list(freeVs) + list(extraVs)
    return {'allLines':lines+linesT, 'lines':lines, 'linesT':linesT, 'pointsI':freeIs, 'pointsV':allVs}

def getAllAttrs(finalCluster, tags, cascades):
    attrs = []    
    for x, (tcas, tcid) in zip(finalCluster, tags):
        #print(tcas, tcid)
        attrs.append(cookAttrs(cascades[tcas], tcid, x['lines'] + x['linesT'], x['pointsI'], x['pointsV']))
    return attrs

def getAllHistAttrs(attrs, tags, cascades):
    maxAttrs = cookMaxAttrs(attrs)
    histAttrs = []
    for x, (tcas, _) in zip(attrs, tags):
        nnDistBins = cookNNBins(cascades[tcas]['latticeConst']) ## Make it take max for last bin edge
        histAttrs.append(cookHistAttrs(x, maxAttrs, nnDistBins))
    return histAttrs

def addLinesData(data, finalClusters, tags):
    for lineC, (tcas, tcid) in zip(finalClusters, tags):
        di = {'lines':[], 'linesT':[], 'pointsI':lineC['pointsI'], 'pointsV': lineC['pointsV']}
        #print(lineC)
        for line in lineC['lines']:
            if 'parent' in line or 'del' in line: continue
            sub = []
            if line['subLine']: sub = [x[1] for x in line['subLine']['points']]
            di['lines'].append({'main': [x[1] for x in line['mainPoints']], 'sub': sub, 'orient': line['forceAlign']['dir']})
        for line in lineC['linesT']:
            if 'parent' in line or 'del' in line: continue
            di['linesT'].append({'main': [x[1] for x in line['mainPoints']], 'orient': line['forceAlign']['dir']})
        data[tcas]['features'][tcid]['lines'] = di

def addLineFeat(cascade, cid, lineC):
    di = {'lines':[], 'linesT':[], 'pointsI':lineC['pointsI'], 'pointsV': lineC['pointsV']}
    #print(lineC)
    for line in lineC['lines']:
        if 'parent' in line or 'del' in line: continue
        sub = []
        if line['subLine']: sub = [x[1] for x in line['subLine']['points']]
        di['lines'].append({'main': [x[1] for x in line['mainPoints']], 'sub': sub, 'orient': line['forceAlign']['dir']})
    for line in lineC['linesT']:
        if 'parent' in line or 'del' in line: continue
        di['linesT'].append({'main': [x[1] for x in line['mainPoints']], 'orient': line['forceAlign']['dir']})
    cascade['features'][cid]['lines'] = di


# In[1235]:



stdPlanes = [Plane(Point(0.0,0.0,0.0), Point(0.0, 2.0, 5.0), Point(0.0, 4.0, 9.0)),
             Plane(Point(0.0,0.0,0.0), Point(1.0, 0.0, 4.0), Point(4.0, 0.0, 5.0)),
             Plane(Point(0.0,0.0,0.0), Point(1.0, 2.0, 0.0), Point(3.0, 4.0, 0.0))]
def getSymmetricPlaneAngle(line):
    res = []
    for x in stdPlanes:
        res.append(x.angle_to(line))
    return res

def getSymmetricPlaneAngleOld(line):
    res = [0.0, 0.0, 0.0]
    for i, x in enumerate(line.t):
        j = i + 1 if i < 2 else 0
        angle = 0.0
        if abs(line.t[j]) > 0.0: angle = abs(angular_unit*math.atan(line.t[i] / line.t[j]))
        if angle > 45: angle = 90 - angle
        res[i] = angle
    return res

def forceAlign(line):
    angleTol = 18
    milErrTol = 0.20
    milErrTotTol = 0.35
    subP = (line.r - line.r2)
    divP = max(abs(subP))
    if divP == 0.0:
      return {"type": (0,0,0), 'type1': (0,0,0), 'dir': (0.0, 0.0), 'eq':line, 'angles': (0.0,0.0), 'err': (False, 0.0, 0.0, 0.0)}
    milBasic = subP/divP
    milBasicAbs = abs(milBasic)
    # method 1
    milErrSum = 0.0
    milErrMax = 0.0    
    milErrList = []
    milMult = 1
    perfect = False
    for milMult in range(1, 3):
        curMilAbs = milBasicAbs * milMult      
        milErrList = abs(curMilAbs - np.rint(curMilAbs))
        milErrSum = sum(milErrList)
        milErrMax = max(milErrList)
        #print(milMult, milErrList)        
        if milErrMax < (milErrTol / milMult) and milErrSum < (milErrTotTol / milMult):
            perfect = True
            break
    #print(milErrList)
    if not perfect:
        milErrOne = sum(abs(milBasicAbs - np.rint(milBasicAbs)))
        if milErrOne < milErrSum * 2:
            milErr = milErrOne
            milMult = 1
    #print(milErr, milErrList)
    milFamOne = tuple(sorted([abs(int(x)) for x in np.rint(milBasic)]))
    milDir = np.rint(milBasic * milMult)
    milFam = tuple(sorted([abs(int(x)) for x in milDir]))
    p1 = Point(line.r)
    lineN = Line(p1, Point(p1.r - milDir))
    angles = getSymmetricPlaneAngle(lineN)
    angErr = line.angle_to(lineN)
    rawDir = tuple(sorted(abs(milBasic))[:-1])
    return {"type": milFam, 'type1': milFamOne, 'dir': rawDir, 'eq':lineN, 'angles': angles, 'err': (perfect, milErrSum, milErrMax, angErr)}

# In[1104]:



# In[1106]:


"""
- in all neighbouring points, check if in a line and add line only if angle is less
- 
"""

def lineDist(a, b):
    angleBetween = a['eq'].angle_to(b['eq'])
    return (angleBetween, a['eq'].distance_to(Point(b['eq'].r)), a['eq'].distance_to(Point(b['eq'].r2)))

def addLine(line, otherLine, dist):
    if 'children' not in line: 
        line['children'] = {'lines':[]}
    line['children']['lines'].append((dist, otherLine['id']))
    if 'parent' not in otherLine:
        otherLine['parent'] = []
    otherLine['parent'].append((dist, line['id']))

def addForceAlignAndSort(cascade, cid, lines, linesT):
    sortPointsHelper = lambda a: sum(cascade['eigen_features'][cid]['coords'][a[1]])
    for line in lines:
        if 'parent' in line: continue
        line['mainPoints'].sort(key=sortPointsHelper)        
        if line['subLine']:
            line['forceAlign'] = forceAlign(line['subLine']['eq'])
            line['subLine']['forceAlign'] = line['forceAlign']
            line['subLine']['points'].sort(key=sortPointsHelper)
        else:
            line['forceAlign'] = forceAlign(line['eq'])
    for line in linesT:
        if 'parent' in line: continue
        line['mainPoints'].sort(key=sortPointsHelper)        
        line['forceAlign'] = forceAlign(line['eq'])
    # TODO what if a line's neighbours are added while that line is not added?
    # possibly add Vs that don't add to subline to freeVs and points to the line?


"""
TODO
Finds if there is a free vacancy,
if other prob-vacancies around it and that vacancy forms a line that is either at an angle > eps or
at a distance if yes creates a sub line.
"""

def findSubLine(cascade, cid, line, vacsOrig):
    # TODO, continue with probP vacs with dist > threshold are more than 2
    #distTol = 0.20
    vacs = list(reversed(sorted(vacsOrig)))
    #if vacs[1][0] < distTol: return None
    mainPIndex = vacs[0][1]
    #mainP = cascade['eigen_features'][cid]['coords'][mainPIndex]
    mainPReal = cascade['coords'][cascade['clusters'][cid][mainPIndex]][:3]
    otherP = vacs[1][1]
    otherPReal = cascade['coords'][cascade['clusters'][cid][otherP]][:3]
    err = vacs[0][0]#line['vacErr'][1]
    err += vacs[1][0]
    subLine = Line(Point(mainPReal), Point(otherPReal))
    subLinePs = list()    
    subLinePs.append((0.0, mainPIndex))
    subLinePs.append((0.0, otherP))
    #if line['id'] == 11: print (vacs, subLinePs)
    for mainDist, index in vacs[2:]:
        curPoint = cascade['coords'][cascade['clusters'][cid][index]][:3]
        curDist = subLine.distance_to(Point(curPoint))
        #if line['id'] == 13: print (index, mainDist, curDist)
        if (curDist < mainDist):
            subLinePs.append((curDist, index))
            err += (mainDist - curDist)
    angles = getSymmetricPlaneAngle(subLine)
    angleErr = line['eq'].angle_to(subLine)
    if angleErr > 15 and len(subLinePs) <= 2:
        return None
    #if line['id'] == 13: print(subLinePs)
    return {'eq': subLine, 'points': subLinePs, 'angles':angles, 'err': [angleErr, err]}
# %%

def addCollinearToThresh(cascade, cid, lines, coordIndexMap, linesT, coordIndexMapT, nn, extraVs, extraVsMap):
    angleTol = 18
    distTol = [0.8, cascade['latticeConst']]
    # different criterion for there are only vac and interstitial. intersitial
    # can be slightly shifted in whatever angle so we can not consider that two
    # t-line as one line based on distance from the line subtended
    for line in linesT:
        if 'parent' in line: continue
        pointsToLookNeighs = [cascade['eigen_features'][cid]['coords'][x] for x in line['points']]
        IgnoreNeighs = set(line['points'])
        while pointsToLookNeighs:
            dist, neigh = nn.radius_neighbors(pointsToLookNeighs)
            curNeigh = set()
            for x in neigh:
                for y in x:
                    if y in IgnoreNeighs: continue
                    if y in coordIndexMap: continue
                    curNeigh.add(y)
                    if y in coordIndexMapT: 
                        IgnoreNeighs.update(linesT[coordIndexMapT[y] - len(lines)]['points'])
                    else:
                        IgnoreNeighs.add(y)
            nextPointsToLook = []
            #if (line['id'] == 10): print(curNeigh)
            for neighIndex in curNeigh:
                neighIndex = int(neighIndex)
                if neighIndex in coordIndexMapT:
                    otherLine = linesT[coordIndexMapT[neighIndex]- len(lines)]
                    #print(neighIndex, line['id'])
                    #dist = lineDist(line, otherLine)
                    angleBetween = line['eq'].angle_to(otherLine['eq'])
                    dist = line['eq'].distance_to(Point(otherLine['eq'].r))
                    vacDist = Point(line['eq'].r).distance_to(Point(otherLine['eq'].r))
                    if angleBetween < angleTol and dist < distTol[0] and vacDist < distTol[1]:
                        addLine(line, otherLine, vacDist)
                        nextPointsToLook.append(cascade['eigen_features'][cid]['coords'][neighIndex]) # TODO check index to add or something else
                        for y in otherLine['points']:
                            coordIndexMapT[y] = line['id']
                            coordIndexMapT.pop(y)# = line['id']                           
                        # add to error                                              
                elif neighIndex in extraVs:
                    coordNeighIndex = cascade['coords'][cascade['clusters'][cid][neighIndex]][:3]
                    dist = line['eq'].distance_to(Point(coordNeighIndex))
                    if neighIndex not in extraVsMap or extraVsMap[neighIndex][0] > dist:
                        if neighIndex in extraVsMap: 
                            preLineIndex = extraVsMap[neighIndex][1]
                            if preLineIndex < len(lines):
                                removeExtraV(lines[preLineIndex], neighIndex)
                            else:
                                removeExtraV(linesT[preLineIndex - len(lines)], neighIndex)
                        addExtraV(line, neighIndex, dist)
                        extraVsMap[neighIndex] = (dist, line['id'])
            pointsToLookNeighs = nextPointsToLook
        line['subLine'] = None
        curLinePs = list()
        curLinePs.append((0.0, line['points'][0]))
        curLinePs.append((0.0, line['points'][1]))                          
        if 'children' in line:
            for lineChIndex in line['children']['lines']:
                lineCh = linesT[lineChIndex[1] - len(lines)]
                for x in lineCh['points']:
                    curLinePs.append((lineChIndex[0], x))
        line['freeVs'] = list()
        line['mainPoints'] = curLinePs
        #line['mainPoints'].sort(key=sortPointsHelper)        
        #line['forceAlign'] = forceAlign(line['eq'])
    # TODO what if a line's neighbours are added while that line is not added?
    # possibly add Vs that don't add to subline to freeVs and points to the line?

def addCollinearToTriadsAgain(cascade, cid, lines, coordIndexMap, linesT, coordIndexMapT):
    latConst = cascade['latticeConst']
    distTolErr = 0.10
    distTol = [0.30 + distTolErr, 0.50 + distTolErr, 0.70 + (distTolErr * 2)]#latConst * 0.3
    nn = NearestNeighbors(5, latConst * 2.0)
    tIs = []
    tIsMap = {}
    for line in linesT:
        if 'parent' in line: continue
        tIsMap[len(tIs)] = line['points'][1]
        tIs.append(cascade['eigen_features'][cid]['coords'][line['points'][1]])
    if len(tIs) == 0: return
    nn.fit(tIs)
    allLines = lines + linesT
    for line in allLines:
        if 'parent' in line or len(line['mainPoints']) < 3: continue
        if len(line['mainPoints']) <= 3 and not line['subLine']: continue
        pointsToLookNeighs = [cascade['eigen_features'][cid]['coords'][x[1]] for x in line['mainPoints']]
        IgnoreNeighs = set([x[1] for x in line['mainPoints']])
        #print(line['id'], IgnoreNeighs)
        line['oldEq'] = line['eq']
        line['eq'] = Line([Point(cascade['coords'][cascade['clusters'][cid][x[1]]][:3]) for x in line['mainPoints']])
        while pointsToLookNeighs:
            dist, neigh = nn.radius_neighbors(pointsToLookNeighs)
            curNeigh = set()
            for x in neigh:
                for y in x:
                    #print(y, tIsMap[int(y)])
                    whatever = tIsMap[int(y)]
                    if whatever in IgnoreNeighs: continue
                    curNeigh.add(whatever)
                    IgnoreNeighs.add(whatever)
            nextPointsToLook = []
            #if (line['id'] == 10): (curNeigh)
            for neighIndex in curNeigh:
                if neighIndex in coordIndexMapT:
                    otherLine = linesT[coordIndexMapT[neighIndex]- len(lines)]
                    #(neighIndex, line['id'])
                    interstitialDist = line['eq'].distance_to(Point(otherLine['eq'].r2))
                    vacDist = line['subLine']['eq'].distance_to(Point(otherLine['eq'].r)) if line['subLine'] else line['eq'].distance_to(Point(otherLine['eq'].r))
                    if interstitialDist < distTol[1] and vacDist < distTol[1]: # interstitial inline
                        nextPointsToLook.append(cascade['eigen_features'][cid]['coords'][neighIndex])
                        #print(line['id'], otherLine['id'])
                        addLine(line, otherLine, interstitialDist)
                        cMap = coordIndexMap if line['id'] < len(lines) else coordIndexMapT
                        for y in otherLine['points']:
                            cMap[y] = line['id']
                            if cMap == coordIndexMap: coordIndexMapT.pop(y)
                        line['mainPoints'].append((interstitialDist, otherLine['points'][1]))
                        if line['subLine']:
                            line['subLine']['points'].append((vacDist, otherLine['points'][0]))
                        else:
                            line['mainPoints'].append((vacDist, otherLine['points'][0]))
                            # add to error                                              
            pointsToLookNeighs = nextPointsToLook
    return

def addExtraV(line, neighIndex, dist):
    distTol = 0.70
    if 'extraVs' not in line: line['extraVs'] = set()
    line['extraVs'].add(neighIndex)
    #line['extraVs'][neighIndex] = (dist < distTol, dist)

def removeExtraV(line, neighIndex):
    line['extraVs'].remove(neighIndex)
    #line['extraVs'].pop(neighIndex)
    
def calcSize(lines, linesT):
    for line in lines:
        if 'parent' in line: continue
        size = 1
        if 'children' in line:
            for x in line['children']['lines']:
                if x[1] < len(lines):
                    size += 1
        if 'extraVs' in line:
            line['extraVs'] = list(line['extraVs'])
            size -= len(line['extraVs'])
        line['defectCount'] = size
    for line in linesT:
        if 'parent' in line: continue
        size = 0
        if 'extraVs' in line:
            line['extraVs'] = list(line['extraVs'])
            size -= len(line['extraVs'])
        line['defectCount'] = size

def addCollinearToTriads(cascade, cid, lines, coordIndexMap, linesT, coordIndexMapT, extraVs):
    latConst = cascade['latticeConst']
    angleTol = [15, 27]
    distTolErr = 0.10
    distTol = [0.30 + distTolErr, 0.50 + distTolErr, 0.70 + (distTolErr * 2)]#latConst * 0.3
    nn = NearestNeighbors(5, latConst * 2.0)
    nn.fit(cascade['eigen_features'][cid]['coords'])
    allFreeVs = []
    allLines = lines + linesT
    extraVsMap = {}
    for line in lines:
        if 'parent' in line: continue
        pointsToLookNeighs = [cascade['eigen_features'][cid]['coords'][x] for x in line['points']]
        IgnoreNeighs = set(line['points'])
        subLineVacs = [(line['vacErr'][1] + 1.0, line['points'][1])]
        isCheckSubLine = line['isVac'] != 2
        probableLines = {}
        while pointsToLookNeighs:
            dist, neigh = nn.radius_neighbors(pointsToLookNeighs)
            curNeigh = set()
            for x in neigh:
                for y in x:
                    if y in IgnoreNeighs: continue
                    curNeigh.add(y)
                    if y in coordIndexMap: 
                        IgnoreNeighs.update(lines[coordIndexMap[y]]['points'])
                        # not ignoring probable points which might be farther
                    elif y in coordIndexMapT: 
                        IgnoreNeighs.update(linesT[coordIndexMapT[y] - len(lines)]['points'])
                        # not ignoring probable points which might be farther
                    else:
                        IgnoreNeighs.add(y)
            nextPointsToLook = []
            #if (line['id'] == 10): (curNeigh)
            for neighIndex in curNeigh:
                neighIndex = int(neighIndex)
                if neighIndex in coordIndexMap:
                    otherLine = lines[coordIndexMap[neighIndex]]
                    if 'children' in otherLine: continue
                    #(neighIndex, line['id'])
                    dist = lineDist(line, otherLine)
                    if (dist[0] < angleTol[0] and (dist[1] < distTol[0] or dist[2] < distTol[0])):
                        dist = dist[1] + dist[2]
                        addLine(line, otherLine, dist)
                        nextPointsToLook.append(cascade['eigen_features'][cid]['coords'][neighIndex]) # TODO check index to add or something else
                        for y in otherLine['points']:
                            coordIndexMap[y] = line['id']
                        if not isCheckSubLine: isCheckSubLine = otherLine['isVac'] != 2
                        subLineVacs.append((dist, line['points'][1]))
                elif neighIndex in coordIndexMapT:
                    otherLine = linesT[coordIndexMapT[neighIndex]- len(lines)]
                    #(neighIndex, line['id'])
                    dist = lineDist(line, otherLine)
                    if (dist[0] < angleTol[1] and (dist[1] < distTol[1] and dist[2] < distTol[1])):
                        #if (line['id'] == 1 and otherLine['id'] == 36): print("first", otherLine['id'], dist)
                        totalDist = dist[1] + dist[2]
                        addLine(line, otherLine, totalDist)
                        nextPointsToLook.append(cascade['eigen_features'][cid]['coords'][neighIndex]) # TODO check index to add or something else
                        for y in otherLine['points']:
                            coordIndexMap[y] = line['id']
                            coordIndexMapT.pop(y)# = line['id']                           
                        subLineVacs.append((dist[0], otherLine['points'][0]))
                        if dist[0] > distTol[0]: isCheckSubLine = True
                    else:
                        interstitialDist = line['eq'].distance_to(Point(otherLine['eq'].r2))
                        #if line['id'] == 1 and otherLine['id'] == 36:  print("check-2", otherLine['eq'].r2, interstitialDist)
                        if interstitialDist < distTol[2]: # interstitial inline
                            #if line['id'] == 1 and otherLine['id'] == 36:  print("second", otherLine['eq'].r2, interstitialDist)
                            probableLines[otherLine['points'][0]] = (otherLine['id'], interstitialDist)
                            nextPointsToLook.append(cascade['eigen_features'][cid]['coords'][otherLine['points'][1]])
                            isCheckSubLine = True
                            vacDist = line['eq'].distance_to(Point(otherLine['eq'].r))
                            subLineVacs.append((vacDist, otherLine['points'][0]))
                        # add to error                                              
                elif neighIndex in extraVs:
                    coordNeighIndex = cascade['coords'][cascade['clusters'][cid][neighIndex]][:3]
                    dist = line['eq'].distance_to(Point(coordNeighIndex))
                    if neighIndex not in extraVsMap or extraVsMap[neighIndex][0] > dist:
                        if neighIndex in extraVsMap: removeExtraV(lines[extraVsMap[neighIndex][1]], neighIndex)
                        addExtraV(line, neighIndex, dist)
                        extraVsMap[neighIndex] = (dist, line['id'])
            pointsToLookNeighs = nextPointsToLook
        subLinePs = set()
        freeVs = []
        if isCheckSubLine and len(subLineVacs) > 1:
            subLine = findSubLine(cascade, cid, line, subLineVacs)
            #if line['id'] == 13:  print(subLine)
            if subLine:
                #subLine['forceAlign'] = forceAlign(subLine['eq'])
                #subLine['points'].sort(key=sortPointsHelper)
                for subP in subLine['points']:
                    subLinePs.add(subP[1])
                    if subP[1] not in probableLines: continue
                    otherLineI = probableLines[subP[1]]
                    addLine(line, allLines[otherLineI[0]], otherLineI[1])
                    for y in allLines[otherLineI[0]]['points']:
                        coordIndexMap[y] = line['id']
                        if y in coordIndexMapT: coordIndexMapT.pop(y)
            line['subLine'] = subLine
        else:
            line['subLine'] = None
        curLinePs = list()
        curLinePs.append((0.0, line['points'][0]))
        curLinePs.append((0.0, line['points'][2]))                          
        if line['subLine'] == None or not line['points'][1] in subLinePs:
            if line['isVac'] == 0:
              freeVs.append(line['points'][1])
            else:
              curLinePs.append((line['vacErr'][1], line['points'][1]))
        if 'children' in line:
            for lineChIndex in line['children']['lines']:
                lineCh = allLines[lineChIndex[1]]
                for i, x in enumerate(lineCh['points']):
                    if line['subLine'] and x in subLinePs: continue
                    if i == 1 and 'isVac' in lineCh and lineCh['isVac'] == 0: 
                        freeVs.append(x)
                        continue
                    curLinePs.append((lineChIndex[0], x))
        allFreeVs += freeVs
        line['freeVs'] = freeVs
        line['mainPoints'] = curLinePs
        #line['mainPoints'].sort(key=sortPointsHelper)        
        #line['forceAlign'] = forceAlign(line['eq'])
    addCollinearToThresh(cascade, cid, lines, coordIndexMap, linesT, coordIndexMapT, nn, extraVs, extraVsMap)
    addCollinearToTriadsAgain(cascade, cid, lines, coordIndexMap, linesT, coordIndexMapT)
    addForceAlignAndSort(cascade, cid, lines, linesT)
    calcSize(lines, linesT)
    return allFreeVs, extraVsMap
    # TODO what if a line's neighbours are added while that line is not added?
    # possibly add Vs that don't add to subline to freeVs and points to the line?

def getThreshLines(cascade, cid, startingIndex, pairs):
    lines = []
    coordIndexMap = {}    
    if cascade['clusterSizes'][cid] < 0: return lines, coordIndexMap    
    for cCoordIndex, coordIndex in enumerate(cascade['clusters'][cid]):
        if cascade['coords'][coordIndex][5] == 1: continue
        if coordIndex not in pairs: continue
        otherIndex = pairs[coordIndex]
        cCoordIndex2 = cCoordIndex - 1
        #if cascade['coords'][coordIndex][3] == 0 or cascade['coords'][coordIndex][5] == 0: continue
        #coordIndex = cascade['clusters'][cid][cCoordIndex]
        p1 = Point(cascade['coords'][coordIndex][:3])
        p2 = Point(cascade['coords'][otherIndex][:3])
        line = Line(p1, p2)
        angles = getSymmetricPlaneAngle(line)
        includedPoints = [cCoordIndex2, cCoordIndex]
        coordIndexMap[cCoordIndex] = len(lines) + startingIndex
        coordIndexMap[cCoordIndex2] = len(lines) + startingIndex
        curLine = {'id': len(lines) + startingIndex, 'eq':line, 'points':includedPoints, 'angles':angles}
        lines.append(curLine)
    return lines, coordIndexMap


# In[1105]:


# not using currently
def getSymmetricCosineAngle(line):
    res = [0.0, 0.0, 0.0]    
    for i, x in enumerate(line.t):
        angle = abs(angular_unit*math.acos(x))
        if angle > 90: angle = 90 - angle
        if angle > 45: angle = abs(90 - angle)
        res[i] = angle
    return res

def makeLatticeGroups(cascade):
    triads = {}
    pairs = {}
    pre = 0
    for cur in cascade['coDefects']:
        if (cur - pre) == 2:
            pairs[pre + 1] = pre
        elif (cur - pre) > 2:
            triads[pre + 2] = (pre, pre + 1)
        pre = cur
    return triads, pairs

def getTriadLines(cascade, cid, triads):
    lines = []
    coordIndexMap = {}
    probablePoints = {}
    if cascade['clusterSizes'][cid] < 0: return lines, coordIndexMap, probablePoints
    dumbbellPairSurviveIs = triads
    #dumbbellPairSurviveIs = set([triads[key][1] for key in triads])
    #dumbbellPairIs = set([triads[key][0] for x in triads]) + dumbbellPairSurviveIs
    cvmap = {x: i for i, x in enumerate(cascade['clusters'][cid]) if cascade['coords'][x][3] == 0}
    freeIs = []
    for cCoordIndex, coordIndex in enumerate(cascade['clusters'][cid]):
        #if cascade['coords'][coordIndex][3] == 0 or cascade['coords'][coordIndex][5] == 0: continue
        if coordIndex not in dumbbellPairSurviveIs: 
            if cascade['coords'][coordIndex][3] == 1 and cascade['coords'][coordIndex][5] == 1:
                freeIs.append(cCoordIndex)
            continue
        p1 = Point(cascade['coords'][coordIndex][:3])
        p2 = Point(cascade['coords'][triads[coordIndex][1]][:3])
        # TODO do we change perfect line or is it okay to store the wrong line Eq
        #line = Line(p1, Point(p1.r - p3)) if perfect else Line(p1, p2)
        line = Line(p1, p2)
        angles = getSymmetricPlaneAngle(line)     
        vacIndex = triads[coordIndex][0]
        vacClusterIndex = cvmap[vacIndex]
        latticeSite = Point(cascade['coords'][vacIndex][:3])      
        line1 = Line(p1, latticeSite)        
        line2 = Line(p2, latticeSite)
        vacAngle = abs(line1.angle_to(line2))
        includedPoints = [cCoordIndex, vacClusterIndex, cCoordIndex - 1]
        for pindex in includedPoints:
            coordIndexMap[pindex] = len(lines)
        isVac = 2
        if vacAngle < 18:
            #coordIndexMap[vacClusterIndex] = len(lines)
            isVac = 2
        elif vacAngle < 25:
            isVac = 1
        else:
            isVac = 0
        vacDist = line.distance_to(latticeSite)
        curLine = {'id': len(lines), 'eq':line, 'points':includedPoints, 'angles':angles, 'isVac': isVac, 'vacErr': (vacAngle, vacDist)}
        lines.append(curLine) #({[line1, line2]})
    return lines, coordIndexMap, freeIs#, probablePoints, freeVs

def getExtraFreeVs(cascade, cid, coordIndexMap, coordIndexMapT):
    extraVs = set()
    for cCoordIndex, coordIndex in enumerate(cascade['clusters'][cid]):
        if cCoordIndex not in coordIndexMap and cCoordIndex not in coordIndexMapT:
            if cascade['coords'][coordIndex][3] == 0: extraVs.add(cCoordIndex)
    return extraVs

"""
def cookAttrs(cascade, cid, clusterLines, freeIs, freeVs):
    totalLines = 0
    for i, line in enumerate(clusterLines):
        if 'parent' in line or 'del' in line: continue
        totalLines += 1
    res = {'lineLengths': [],
           #'lineAngles': [[], [], []],
           'nPoints': [],
           'nMain': 0,
           'vacErr': [],
           'nonLines': 0,           
           'nThresh': 0,           
           'inlineErr': [],
           'nPerfect': 0,
           'perfectErr': [],
           'perfectErrMax': [],           
           'lineTypes': [],
           'dir': [[], []],
           'nSubLine': 0,           
           'subLineAng': [],
           'subLineLen': [],
           'subLineAngErr': [],
           'subLineDistErr': [],
           'neighDists': [],
           'neighDistsParOnly': [],
           'neighDistsPar': [],           
           'neighAngles': [],
           'neighAnglesPar': [],
           'neighAnglesParOnly': [],           
           'nfreeIs': len(freeIs),
           'nfreeVs': len(freeVs),
           'adjacencyDist': np.full((totalLines, totalLines), -1.0),
           'adjacencyDistParOnly': np.full((totalLines, totalLines), -1.0), 
           'adjacencyDistPar': np.full((totalLines, totalLines), -1.0),
           'adjacencyAng': np.full((totalLines, totalLines), -1.0),
           'adjacencyAngAligned': np.full((totalLines, totalLines), -1.0),
           'adjacencyLineType': np.full((totalLines, totalLines), False, dtype=bool)
           }
    curI = 0
    for i, line in enumerate(clusterLines):
        if 'parent' in line or 'del' in line: continue
        p1 = Point(cascade['eigen_features'][cid]['coords'][line['mainPoints'][0][1]])
        p2 = Point(cascade['eigen_features'][cid]['coords'][line['mainPoints'][-1][1]])
        res['lineLengths'].append(p1.distance_to(p2) / cascade['latticeConst'])
        #for angIndex, angleI in enumerate(line['angles']): res['lineAngles'][angIndex].append(angleI)
        res['nPoints'].append(len(line['mainPoints']))
        if line['subLine']: res['nPoints'][-1] += len(line['subLine']['points'])
        if 'isVac' in line:  
            res['nMain'] += 1
            res['vacErr'].append(line['vacErr'][0])
            if line['isVac'] != 2:
                res['nonLines'] += 1
        else:
            res['nThresh'] += 1
            res['vacErr'].append(-1)
        res['inlineErr'].append(sum([x[0] for x in line['mainPoints']]))
        if line['forceAlign']['err'][0]:  res['nPerfect'] += 1
        res['perfectErr'].append(line['forceAlign']['err'][1])
        res['perfectErrMax'].append(line['forceAlign']['err'][2])        
        res['lineTypes'].append(line['forceAlign']['type'])
        for angIndex, dirI in enumerate(line['forceAlign']['dir']):
            res['dir'][angIndex].append(dirI)
        if 'subLine' in line and line['subLine']:
            res['nSubLine'] += 1            
            sp1 = Point(cascade['eigen_features'][cid]['coords'][line['subLine']['points'][0][1]])
            sp2 = Point(cascade['eigen_features'][cid]['coords'][line['subLine']['points'][-1][1]])
            res['subLineLen'].append(sp1.distance_to(sp2))
            res['subLineAng'].append((line['subLine']['angles']))
            res['subLineAngErr'].append(line['subLine']['err'][0])            
            res['subLineDistErr'].append(line['subLine']['err'][1])
        else:
            res['subLineLen'].append(-1)
            res['subLineAng'].append(-1)
            res['subLineAngErr'].append(-1)            
            res['subLineDistErr'].append(-1)
        distTol = cascade['latticeConst'] * 1.8 # ~ > sqrt(3)
        curJ = 0
        for j, otherLine in enumerate(clusterLines):
            if 'parent' in otherLine or 'del' in otherLine: continue
            if curJ <= curI: 
                curJ += 1
                continue
            areTriplets = ('isVac' in line and 'isVac' in otherLine)
            areTriplets = areTriplets and ('children' not in line and 'children' not in otherLine)
            #areTriplets = areTriplets and (line['probP'] and otherLine['probP'])
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
            res['adjacencyAng'][curI][curJ] = lineAng
            res['adjacencyAngAligned'][curI][curJ] = lineAngAligned
            res['adjacencyAng'][curJ][curI] = lineAng
            res['adjacencyAngAligned'][curJ][curI] = lineAngAligned
            if (cmpLine1['forceAlign']['type1'] == cmpLine2['forceAlign']['type1']) or (not(cmpLine1['forceAlign']['err'][0] and cmpLine2['forceAlign']['err'][0]) and cmpLine1['forceAlign']['type1'] == cmpLine2['forceAlign']['type1'] and cmpLine1['forceAlign']['type1'][-1] != 0):
                res['adjacencyLineType'][curI][curJ] = True
                res['adjacencyLineType'][curJ][curI] = True
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
            #print(i, j, lineDist, distTol)
            if lineDistParOnly > distTol: 
                curJ += 1
                continue
            if not areTriplets:# and not sameForceAligned:
               #if cmpLine1['subLine']: cmpLine1 = cmpLine1['subLine']
                #if cmpLine1['forceAlign']: cmpLine1 = cmpLine1['forceAlign']                 
                cmpPoints1 = cmpLine1['subLine']['points'] if cmpLine1['subLine'] else cmpLine1['mainPoints']
                cmpPoints2 = cmpLine2['subLine']['points'] if cmpLine2['subLine'] else cmpLine2['mainPoints']
                extraPoint1 = []
                extraPoint2 = []
                if not cmpLine1['subLine'] and 'isVac' in cmpLine1 and cmpLine1['isVac'] == 0:
                    extraPoint1 = [cmpLine1['points'][1]]
                if not cmpLine2['subLine'] and 'isVac' in cmpLine2 and cmpLine2['isVac'] == 0:
                    extraPoint2 = [cmpLine2['points'][1]]
                #if cmpLine1['subLine']: cmpLine1 = cmpLine1['subLine']
                #if cmpLine2['subLine']: cmpLine2 = cmpLine2['subLine'] 
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
            if lineDist < distTol: 
                res['neighDists'].append(lineDist)
                res['neighAngles'].append(lineAng)
                res['adjacencyDist'][curI][curJ] = lineDist
                #res['adjacencyDist'][curJ][curI] = lineDist
            #areAlmostParallel =  line['forceAlign']['eq'].angle_to(otherLine['forceAlign']['eq']) < 15
            res['neighDistsPar'].append(lineDistPar)
            res['neighAnglesPar'].append(lineAng)            
            res['adjacencyDistPar'][curI][curJ] = lineDistPar
            #res['adjacencyDistPar'][curJ][curI] = lineDistPar
            #if lineAng < 15:                                   
            res['neighDistsParOnly'].append(lineDistParOnly)
            res['neighAnglesParOnly'].append(lineAng)                   
            res['adjacencyDistParOnly'][curI][curJ] = lineDistParOnly             
            #res['adjacencyDistParOnly'][curJ][curI] = lineDistParOnly             
            curJ += 1
        curI += 1
    return res

def cookAttrs(cascade, cid, clusterLines, freeIs, freeVs):
    totalLines = 0
    for i, line in enumerate(clusterLines):
        if 'parent' in line or 'del' in line: continue
        totalLines += 1
    res = {'lineLengths': [],
           #'lineAngles': [[], [], []],
           'nPoints': [],
           'nMain': 0,
           'vacErr': [],
           'nonLines': 0,           
           'nThresh': 0,           
           'inlineErr': [],
           'nPerfect': 0,
           'perfectErr': [],
           'perfectErrMax': [],           
           'lineTypes': [],
           'dir': [[], []],
           'nSubLine': 0,           
           'subLineAng': [],
           'subLineLen': [],
           'subLineAngErr': [],
           'subLineDistErr': [],
           'neighDists': [],
           'neighDistsParOnly': [],
           'neighDistsPar': [],           
           'neighAngles': [],
           'neighAnglesPar': [],
           'neighAnglesParOnly': [],           
           'nfreeIs': len(freeIs),
           'nfreeVs': len(freeVs),
           'adjacencyDist': np.full((totalLines, totalLines), -1.0),
           'adjacencyDistParOnly': np.full((totalLines, totalLines), -1.0), 
           'adjacencyDistPar': np.full((totalLines, totalLines), -1.0),
           'adjacencyAng': np.full((totalLines, totalLines), -1.0),
           'adjacencyAngAligned': np.full((totalLines, totalLines), -1.0)
           }
    curI = 0
    for i, line in enumerate(clusterLines):
        if 'parent' in line or 'del' in line: continue
        p1 = Point(cascade['eigen_features'][cid]['coords'][line['mainPoints'][0][1]])
        p2 = Point(cascade['eigen_features'][cid]['coords'][line['mainPoints'][-1][1]])
        res['lineLengths'].append(p1.distance_to(p2) / cascade['latticeConst'])
        #for angIndex, angleI in enumerate(line['angles']): res['lineAngles'][angIndex].append(angleI)
        res['nPoints'].append(len(line['mainPoints']))
        if line['subLine']: res['nPoints'][-1] += len(line['subLine']['points'])
        if 'isVac' in line:  
            res['nMain'] += 1
            res['vacErr'].append(line['vacErr'][0])
            if line['isVac'] != 2:
                res['nonLines'] += 1
        else:
            res['nThresh'] += 1
            res['vacErr'].append(-1)
        res['inlineErr'].append(sum([x[0] for x in line['mainPoints']]))
        if line['forceAlign']['err'][0]:  res['nPerfect'] += 1
        res['perfectErr'].append(line['forceAlign']['err'][1])
        res['perfectErrMax'].append(line['forceAlign']['err'][2])        
        res['lineTypes'].append(line['forceAlign']['type'])
        for angIndex, dirI in enumerate(line['forceAlign']['dir']):
            res['dir'][angIndex].append(dirI)
        if 'subLine' in line and line['subLine']:
            res['nSubLine'] += 1            
            sp1 = Point(cascade['eigen_features'][cid]['coords'][line['subLine']['points'][0][1]])
            sp2 = Point(cascade['eigen_features'][cid]['coords'][line['subLine']['points'][-1][1]])
            res['subLineLen'].append(sp1.distance_to(sp2))
            res['subLineAng'].append((line['subLine']['angles']))
            res['subLineAngErr'].append(line['subLine']['err'][0])            
            res['subLineDistErr'].append(line['subLine']['err'][1])
        else:
            res['subLineLen'].append(-1)
            res['subLineAng'].append(-1)
            res['subLineAngErr'].append(-1)            
            res['subLineDistErr'].append(-1)
        distTol = cascade['latticeConst'] * 1.8 # ~ > sqrt(3)
        curJ = 0
        for j, otherLine in enumerate(clusterLines):
            if 'parent' in otherLine or 'del' in otherLine: continue
            if curJ <= curI: 
                curJ += 1
                continue
            areTriplets = ('isVac' in line and 'isVac' in otherLine)
            areTriplets = areTriplets and ('children' not in line and 'children' not in otherLine)
            #areTriplets = areTriplets and (line['probP'] and otherLine['probP'])
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
            #print(i, j, lineDist, distTol)
            if lineDistParOnly > distTol: 
                curJ += 1
                continue
            if not areTriplets:# and not sameForceAligned:
               #if cmpLine1['subLine']: cmpLine1 = cmpLine1['subLine']
                #if cmpLine1['forceAlign']: cmpLine1 = cmpLine1['forceAlign']                 
                cmpPoints1 = cmpLine1['subLine']['points'] if cmpLine1['subLine'] else cmpLine1['mainPoints']
                cmpPoints2 = cmpLine2['subLine']['points'] if cmpLine2['subLine'] else cmpLine2['mainPoints']
                extraPoint1 = []
                extraPoint2 = []
                if not cmpLine1['subLine'] and 'isVac' in cmpLine1 and cmpLine1['isVac'] == 0:
                    extraPoint1 = [cmpLine1['points'][1]]
                if not cmpLine2['subLine'] and 'isVac' in cmpLine2 and cmpLine2['isVac'] == 0:
                    extraPoint2 = [cmpLine2['points'][1]]
                if cmpLine1['subLine']: cmpLine1 = cmpLine1['subLine']
                if cmpLine2['subLine']: cmpLine2 = cmpLine2['subLine'] 
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
            lineAng = line['eq'].angle_to(otherLine['eq'])    
            lineAngAligned = cmpLine1Aligned['eq'].angle_to(cmpLine2Aligned['eq'])    
            if lineDist < distTol: 
                res['neighDists'].append(lineDist)
                res['neighAngles'].append(lineAng)
                res['adjacencyDist'][curI][curJ] = lineDist
                #res['adjacencyDist'][curJ][curI] = lineDist
            #areAlmostParallel =  line['forceAlign']['eq'].angle_to(otherLine['forceAlign']['eq']) < 15
            res['neighDistsPar'].append(lineDistPar)
            res['neighAnglesPar'].append(lineAng)            
            res['adjacencyDistPar'][curI][curJ] = lineDistPar
            #res['adjacencyDistPar'][curJ][curI] = lineDistPar
            #if lineAng < 15:                                   
            res['neighDistsParOnly'].append(lineDistParOnly)
            res['neighAnglesParOnly'].append(lineAng)                   
            res['adjacencyDistParOnly'][curI][curJ] = lineDistParOnly             
            res['adjacencyAng'][curI][curJ] = lineAng
            res['adjacencyAngAligned'][curI][curJ] = lineAngAligned
            #res['adjacencyDistParOnly'][curJ][curI] = lineDistParOnly             
            #res['adjacencyAng'][curJ][curI] = lineAng
            #res['adjacencyAngAligned'][curJ][curI] = lineAngAligned
            curJ += 1
        curI += 1
    return res

def cookMaxAttrs(allAttrs):
    res = {'lineLengths': 0,
           'nPoints': 0,
           'vacErr': 0,   
           'inlineErr': 0,
           'perfectErr': 0,
           'perfectErrMax': 0,
           'subLineLen': 0,
           'subLineAngErr': 0,
           'subLineDistErr': 0,
           'neighDists': 0,
           'neighDistsPar': 0,
           'neighDistsParOnly': 0,           
           'neighAngles': 0,
           'nMain': 0,
           'nonLines': 0,           
           'nThresh': 0,           
           'nPerfect': 0,
           'nSubLine': 0,           
           'nfreeIs': 0,
           'nfreeVs': 0,        
           }    
    for attrs in allAttrs:
        try:
            res['lineLengths'] = max(max(attrs['lineLengths']), res['lineLengths'])
            res['nPoints'] = max(max(attrs['nPoints']), res['nPoints'])
            if attrs['vacErr']: res['vacErr'] = max(max(attrs['vacErr']), res['vacErr'])
            res['inlineErr'] = max(max(attrs['inlineErr']),  res['inlineErr'])
            if attrs['perfectErr']: res['perfectErr'] = max(max(attrs['perfectErr']),res['perfectErr'])
            if attrs['perfectErrMax']: res['perfectErrMax'] = max(max(attrs['perfectErrMax']),res['perfectErrMax'])

            if attrs['subLineLen']:
                res['subLineLen'] = max(max(attrs['subLineLen']), res['subLineLen'])
                res['subLineAngErr'] = max(max(attrs['subLineAngErr']), res['subLineAngErr'])
                res['subLineDistErr'] = max(max(attrs['subLineDistErr']), res['subLineDistErr'])

            if attrs['neighDists']:
                res['neighDists'] = max(max(attrs['neighDists']), res['neighDists'])
                res['neighDistsPar'] = max(max(attrs['neighDistsPar']), res['neighDistsPar'])
                res['neighAngles'] = max(max(attrs['neighAngles']), res['neighAngles'])
            if attrs['neighDistsParOnly']:
                res['neighDistsParOnly'] = max(max(attrs['neighDistsParOnly']), res['neighDistsParOnly'])
                
            res['nMain'] = max(attrs['nMain'], res['nMain'])
            res['nonLines'] = max(attrs['nonLines'], res['nonLines'])
            res['nThresh'] = max(attrs['nThresh'], res['nThresh'])
            res['nPerfect'] = max(attrs['nPerfect'], res['nPerfect'])
            res['nSubLine'] = max(attrs['nSubLine'], res['nSubLine'])
            res['nfreeIs'] = max(attrs['nfreeIs'], res['nfreeIs'])
            res['nfreeVs'] = max(attrs['nfreeVs'], res['nfreeVs'])
        except:
            print("Warning: possible error in maxAttr")
            pass
    res = {x:math.ceil(res[x]) for x in res}
    return res

def cookNNBins(latticeConst):
    nns = [0.8660254037844386, 1.0, 1.4142135623730951, 1.6583123951777,1.7320508075688772]
    bins = [0.0]
    interval = 0.02
    for x in nns:
        bins.append(x - interval)
        bins.append(x + interval)
    bins.append(math.ceil(nns[-1]) + 1)
    bins = [latticeConst * x for x in bins]
    return bins

def cookHistAttrs(attrs, maxes, nnDistBins):
    res = {'lineLengths': [],
           #'lineAngles': [],
           'nPoints': [],
           'nMain': attrs['nMain'],
           'vacErr': [],
           'nonLines': attrs['nonLines'],           
           'nThresh': attrs['nThresh'],           
           'inlineErr': [],
           'nPerfect': attrs['nPerfect'],
           'perfectErr': [],
           'perfectErrMax': [],           
           'lineTypes': {},
           'dir': [],
           'nSubLine': attrs['nSubLine'],           
           #'subLineAng': [],
           'subLineLen': [],
           'subLineAngErr': [],
           'subLineDistErr': [],
           'neighDists': [],
           'neighDistsPar': [], 
           'neighDistsParOnly': [],            
           'neighAngles': [],
           'neighAnglesPar': [],
           'neighAnglesParOnly': [],
           'nfreeIs': attrs['nfreeIs'],
           'nfreeVs': attrs['nfreeVs']
           }
    angleBins = [0.0, 18.0, 27.0, 45.0, 72.0, 90.0]
    res['lineLengths'], bins = np.histogram(attrs['lineLengths'], range=[0.0, maxes['lineLengths']], bins = maxes['lineLengths'])
    #res['lineAngles'], bins, _ = np.histogram2d(attrs['lineAngles'][0], attrs['lineAngles'][1], range=[[0.0, 45.0], [0.0, 45.0]], bins = 45 / 9)
    res['nPoints'], bins  = np.histogram(attrs['nPoints'], range=[0.0, maxes['nPoints']], bins = maxes['nPoints'])
    res['vacErr'], bins = np.histogram(attrs['vacErr'], range=[0.0, 90.0], bins = angleBins)
    res['inlineErr'], bins = np.histogram(attrs['inlineErr'], range=[0.0, maxes['inlineErr']], bins= 5)
    res['perfectErr'], bins = np.histogram(attrs['perfectErr'], range=[0.0, 2.0], bins = int(2.0 / 0.4))
    res['perfectErrMax'], bins = np.histogram(attrs['perfectErrMax'], range=[0.0, 1.0], bins = int(1.0 / 0.2))
    res['lineTypes'] = Counter(attrs['lineTypes'])
    res['dir'], bins, _ = np.histogram2d(attrs['dir'][0], attrs['dir'][1], range=[[0.0, 1.0], [0.0, 1.0]], bins = 1.0 / 0.2)    
    if maxes['nSubLine'] > 0 and maxes['subLineLen'] > 0:
        res['subLineLen'], bins = np.histogram(attrs['subLineLen'], range=[0.0, maxes['subLineLen']], bins=maxes['subLineLen'])
        #res['subLineAng'], bins = np.histogram2d(attrs['subLineAng'], range=[[0.0, 45.0], [0.0, 45.0]], bins = 45 / 18)
        res['subLineAngErr'], bins = np.histogram(attrs['subLineAngErr'], range=[0.0, maxes['subLineAngErr']], bins=4)
        res['subLineDistErr'], bins = np.histogram(attrs['subLineDistErr'], range=[0.0, maxes['subLineDistErr']], bins = 5)
    else:
        res['subLineLen'] = []
        #res['subLineAng'], bins = np.histogram2d(attrs['subLineAng'], range=[[0.0, 45.0], [0.0, 45.0]], bins = 45 / 18)
        res['subLineAngErr'] = []
        res['subLineDistErr'] = []
    res['neighDists'], bins = np.histogram(attrs['neighDists'], bins = nnDistBins)
    res['neighDistsPar'], bins = np.histogram(attrs['neighDistsPar'], range = [0.0, maxes['neighDistsPar']], bins = int(max(maxes['neighDistsPar']/0.75, 5)))
    res['neighDistsParOnly'], bins = np.histogram(attrs['neighDistsParOnly'], range = [0.0, maxes['neighDistsParOnly']], bins = int(max(maxes['neighDistsParOnly']/nnDistBins[4], 5)))    
    res['neighAngles'], bins = np.histogram(attrs['neighAngles'], range=[0.0, 90.0], bins = angleBins)
    res['neighAnglesPar'], bins = np.histogram(attrs['neighAnglesPar'], range=[0.0, 90.0], bins = angleBins)
    res['neighAnglesParOnly'], bins = np.histogram(attrs['neighAnglesParOnly'], range=[0.0, 90.0], bins = angleBins)
    res['neighs'], bins, _ = np.histogram2d(attrs['neighDists'], attrs['neighAngles'], range=[[0.0, maxes['neighDists']], [0.0, 90.0]], bins = [nnDistBins, angleBins])
    return res
"""
