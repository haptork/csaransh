import json
import numpy as np
np.set_printoptions(threshold=np.nan)
from sklearn.decomposition import PCA
from sklearn.cluster import DBSCAN 
import math
import sys

# feature calc

def dotv(a, b):
    res = 0.0
    for x, y in zip(a, b):
        res += (x * y)
    return res

def calcDist(a, b):
    dist = 0.0;
    for x, y in zip(a, b):      
        dist += ((x - y) ** 2);
    return math.sqrt(dist);

def crossPros(v1, v2):
    prod = [0.0, 0.0, 0.0]
    prod[0] = v1[1]*v2[2] - v1[2]*v2[1];
    prod[1] = v1[2]*v2[0] - v1[0]*v2[2];
    prod[2] = v1[0]*v2[1] - v1[1]*v2[0];
    return prod

def calcAngle(a, b, c):
  v1 = [0.0, 0.0, 0.0]
  v2 = [0.0, 0.0, 0.0]
  pi = 3.141592653589793;
  for i, x in enumerate(a):
    v1[i] = b[i] - a[i]
    v2[i] = c[i] - a[i]
  res = dotv(v1, v2) / math.sqrt(dotv(v1, v1) * dotv(v2, v2))
  if (res >= 1.0): res = 1.0 - 1e-6
  elif (res <= -1.0): res = -1.0 + 1e-6
  return math.acos(res) * 180.0 / pi

def pairHists(v, v2, latConst):
    maxDistAssumption = 200.0
    distBinSize = 5.0
    distBins = (int)((maxDistAssumption)/distBinSize)
    adjacencyHistnn2 = [0] * 20
    adjacencyHistnn4 = [0] * 40
    distHist = [0.0] * distBins
    total = 0.0
    totalAdj = 0.0
    nn = math.sqrt(3) * latConst / 2.0 + 1e-6
    nn2 = latConst + 1e-6
    nn4 = nn * 2
    for i, x in enumerate(v):
        #if v2[i] : continue
        adjC = [0, 0]
        for j, y in enumerate(v):
            dist = calcDist(x, y)
            if (dist < nn2): adjC[0] += 1
            if (dist < nn4): adjC[1] += 1
            if j > i:
                bin = (int)(dist / distBinSize)
                if bin >= len(distHist): bin = len(distHist) - 1
                distHist[bin] += 1
                total += 1
        if (adjC[0] >= 20): adjC[0] = 19
        if (adjC[1] >= 40): adjC[1] = 39
        adjacencyHistnn2[adjC[0]] += 1
        adjacencyHistnn4[adjC[1]] += 1
        totalAdj += 1
    #print total
    if (total > 1e-6):
        for i,x in enumerate(distHist): distHist[i] = round(x/total,2)
    if (totalAdj > 1e-6):
        for i,x in enumerate(adjacencyHistnn2): adjacencyHistnn2[i] = round(x/totalAdj,2)
        for i,x in enumerate(adjacencyHistnn4): adjacencyHistnn4[i] = round(x/totalAdj,2)
    angleBinSize = 5.0
    maxAngle = 180.0
    nAngleBins = (int)((maxAngle / angleBinSize))
    angleHist = [0] * nAngleBins
    total = 0.0
    for i, x in enumerate(v):
        #if v2[i] == 1: continue  # for int subcascades?
        for j, y in enumerate(v):
            curDist = calcDist(x, y)
            if (i == j or  curDist > nn4 or curDist < 0.1): continue
            for k, z in enumerate(v):
                curDist = calcDist(v[i], v[k])
                if (i == k or j == k or v2[j] != v2[k] or curDist > nn4 or curDist < 0.1): continue
                bin = (int)(calcAngle(v[i], v[j], v[k]) / angleBinSize)
                if bin >= len(angleHist): bin = len(angleHist) - 1
                angleHist[bin] += 1
                total += 1
    if total > 1e-6: 
        for i, it in enumerate(angleHist): angleHist[i] = round(it / total, 2)
    #return {"dist":distHist, "angle":angleHist, "adjNn2":adjacencyHistnn2, "adjNn4": adjacencyHistnn4}
    return {"dist":distHist, "angle":angleHist, "adjNn2":adjacencyHistnn2}


def transformPoint(coords, point, thresh):
    all_coords  = np.asarray(coords)
    point       = np.asarray([point])
    if (len(coords) == 0):
        return(all_coords, point, [0,0,0], [], [], [],[])
    vac_coords = [x for x in all_coords if x[3] == 0 and x[5] == 1]
    int_coords = [x for x in all_coords if x[3] == 1 and x[5] == 1]
    vac_helper = [i for i,x in enumerate(all_coords) if x[3] == 0 and x[5] == 1]
    int_helper = [i for i,x in enumerate(all_coords) if x[3] == 1 and x[5] == 1]
    vac_coords = np.asarray(vac_coords)
    int_coords = np.asarray(int_coords)
    all_coords  = all_coords[:,0:3]
    vac_coords = vac_coords[:,0:3]
    int_coords = int_coords[:,0:3]
    pca = PCA(n_components=3)
    pca.fit(all_coords)
    var = pca.explained_variance_ratio_
    trans_coords = pca.transform(all_coords)
    vac_trans_coords = pca.transform(vac_coords)
    int_trans_coords = pca.transform(int_coords)
    trans_point  = pca.transform(point)
    dbscan_vac   = DBSCAN(eps=thresh,min_samples=3).fit(vac_trans_coords)
    labels_vac   = dbscan_vac.labels_       
    dbscan_int   = DBSCAN(eps=thresh,min_samples=3).fit(int_trans_coords)
    labels_int   = dbscan_int.labels_       
    return (trans_coords, trans_point, var, labels_vac, labels_int, vac_helper, int_helper)

def findEigen(coords):
    pca = PCA(n_components=3)
    pca.fit(coords)
    var = pca.explained_variance_ratio_.tolist()
    eigen_coords = pca.transform(coords)
    eigen_coords = [[round(x[0],2),round(x[1],2), round(x[2],2)] for x in [y for y in eigen_coords.tolist()]]
    var = [round(x,2) for x in var]
    return eigen_coords, var 

def addEigenAndSubcascades(data):
    for fdata in data:
        sys.stdout.write('\rto ' + fdata["infile"] + " "*10)
        sys.stdout.flush()
        nn4 = fdata['latticeConst'] * (3**0.5);
        bias = 3.0;
        pka = [fdata['xrec'], fdata['yrec'], fdata['zrec']]
        c, p, var, lv, li, vh, ih = transformPoint(fdata['coords'], pka, bias * nn4)
        fdata['eigen_coords'] = [[round(x[0],2),round(x[1],2), round(x[2],2)] for x in [y for y in c.tolist()]]
        fdata['eigen_pka'] = [[round(x[0],2),round(x[1],2), round(x[2],2)] for x in p]
        fdata['eigen_var'] = [round(x,2) for x in var]
        subsv = {}
        for i, x in enumerate(lv):
            if x == -1: continue
            x = str(x) + "v"
            if not x in subsv: subsv[x] = []
            subsv[x].append(vh[i])
        subsi = {}
        for i, x in enumerate(li):
            if x == -1: continue
            x = str(x) + "i"
            if not x in subsi: subsi[x] = []
            subsi[x].append(ih[i])
        lenV = []
        totalV = 0
        for x in subsv:
            fdata['clusters'][x] = subsv[x]
            lenV.append([len(subsv[x]), x])
            totalV += len(subsv[x])
        lenV.sort(reverse=True)
        lenI = []
        for x in subsi:
            fdata['clusters'][x] = subsi[x]
            lenI.append([len(subsi[x]), x])
        lenI.sort(reverse=True)
        if (len(lenV) > 0):
            countOne = lenV[0][0]
            cutOff = 0.5;
        fdata['density_cluster_vac'] = [x[1] for i, x in enumerate(lenV) if (x[0] / (totalV/len(lenV)) > 0.55 and x[0] > 4) or (i < 2 and x[0] > 4)]
        #fdata['density_cluster_vac'] = [x[1] for i, x in enumerate(lenV)]
        #fdata['density_cluster_vac'] = [x[1] for i, x in enumerate(lenV) if (x[0] / (totalV/len(lenV)) > 0.5 and x[0] > 4) or (i < 2)]
        fdata['density_cluster_int'] = [x[1] for i, x in enumerate(lenI)]
        features = {}
        eigen_features = {}
        for x in fdata['clusters']:
            if len(fdata['clusters'][x]) < 3: continue
            c = [fdata['coords'][y][:3] for y in fdata['clusters'][x]]
            ec, ev = findEigen(c);
            eigen_features[x] = {'coords': ec, 'var': ev}
            if x in subsv or x in subsi:
                b = [fdata['coords'][y][3] for y in fdata['clusters'][x]]
                fdata['features'][x] = pairHists(c, b, fdata['latticeConst'])
        #eigen_features[x] = {'eigen_var': ev}
        #fdata['features'] = features
        fdata['eigen_features'] = eigen_features

def chiSqr(a, b):
    res = 0.0
    for x, y in zip(a, b):
        if (abs(x) > 1e-6):
            res += ((x - y)**2 *1.0) / (1.0*x)
    return round(res, 2)

dist = chiSqr

def compareTwoClusters(a, b):
    res = {}
    alla = []
    allb = []
    for x in a:
        res[x] = dist(a[x], b[x])
        alla = alla + a[x]
        allb = allb + b[x]
    res['all'] = chiSqr(alla, allb)
    return res

def compareWithCluster(pivotI, pivotCid, pivot, data, size, dim):
    res = {}
    res_with_size = {}
    size_tol = .4;
    dim_tol = .2;
    tol_cur = (int)(size * size_tol)
    for i, fdata in enumerate(data):
        for cid in fdata['features']:
            if i == pivotI and cid == pivotCid: continue
            score = compareTwoClusters(pivot, fdata['features'][cid])
            for key in score:
                if (not key in res): res[key] = []
                res[key].append((score[key], i, cid))
                var = fdata['eigen_features'][cid]['var']
                tol1 = abs(var[0] - dim[0])
                tol2 = abs(var[0] + var[1] - dim[0] - dim[1])
                if abs(len(fdata['clusters'][cid]) - size) < tol_cur and tol1 < dim_tol and tol2 < dim_tol:
                    if (not key in res_with_size): res_with_size[key] = []
                    res_with_size[key].append((score[key], i, cid))
    return res, res_with_size

def addClusterCmp(data):
  topsize = 5
  for i, fdata in enumerate(data):
      sys.stdout.write('\rto ' + fdata["infile"] + " "*10)
      sys.stdout.flush()
      fdata['clust_cmp'] = {}
      fdata['clust_cmp_size'] = {}
      for cid in fdata['features']:
          res, res_with_size = compareWithCluster(i, cid, fdata['features'][cid], data, len(fdata['clusters'][cid]), fdata['eigen_features'][cid]['var'])
          for key in res:
              res[key].sort()
              #rev = res[key][-topsize:]
              #rev.reverse()
              res[key] = res[key][:topsize]#[res[key][:topsize], rev]
              # res_with_size
              if not key in res_with_size:
                res_with_size[key] = [[],[]]
                continue
              res_with_size[key].sort()
              #rev = res_with_size[key][-topsize:]
              #rev.reverse()
              res_with_size[key] = res_with_size[key][:topsize]#[res_with_size[key][:topsize], rev]            
          fdata['clust_cmp_size'][cid] = res_with_size   
          fdata['clust_cmp'][cid] = res

if __name__ == "__main__":
    fname = "cascades-data.json"
    out_fname = "cascades-data.js"
    if len(sys.argv) > 1: fname = sys.argv[1]
    try:
        f = open(fname, "r+")
        cascades = json.load(f)
        f.close()
        print "Adding basis vectors and features..."
        addEigenAndSubcascades(cascades['data'])
        print 'finished.'
        print "Adding cluster comparison..."
        addClusterCmp(cascades['data'])
        print 'finished.'
        sys.stdout.flush()
        f = open(out_fname, "w")
        f.write("var cascades = \n")
        json.dump(cascades, f)
        f.close()
    except IOError:
      print "Could not open file " + fname