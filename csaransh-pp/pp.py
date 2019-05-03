import json
import numpy as np
np.set_printoptions(threshold=np.nan)
from sklearn.decomposition import PCA
from sklearn.cluster import DBSCAN 
from sklearn.manifold import TSNE
import math
import sys

import numba
import umap
import hdbscan

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
            lenV.append([len(subsv[x]), x])
            totalV += len(subsv[x])
        lenV.sort(reverse=True)
        lenI = []
        for x in subsi:
            lenI.append([len(subsi[x]), x])
        lenI.sort(reverse=True)
        if (len(lenV) > 0):
            countOne = lenV[0][0]
            cutOff = 0.5;
        dclustNamesV = [x[1] for i, x in enumerate(lenV) if (x[0] / (totalV/len(lenV)) > 0.55 and x[0] > 4) or (i < 2 and x[0] > 4)]
        fdata['dclust_coords'] = {}
        #fdata['dclustV_sm_coords'] = {}
        for x in subsv:
            if x in dclustNamesV: 
                fdata['dclust_coords'][x] = subsv[x]
            else:
                pass
                # fdata['density_cluster_vac_sm'][x] = subsv[x]
        # fdata['density_cluster_int'] = subsi
        fdata['dclustI_count'] = len(subsi)
        dclust_len = [0, 0]
        if (len(lenV) > 0): dclust_len[0] = lenV[0][0]
        if (len(lenV) > 1): dclust_len[1] = lenV[1][0]
        fdata['dclust_sec_impact'] = 0
        if (dclust_len[0] > 0): 
            fdata['dclust_sec_impact'] = dclust_len[1] * 100 / dclust_len[0]
        features = {}
        eigen_features = {}
        for x in fdata['clusters']:
            if len(fdata['clusters'][x]) < 3: continue
            c = [fdata['coords'][y][:3] for y in fdata['clusters'][x]]
            ec, ev = findEigen(c);
            eigen_features[x] = {'coords': ec, 'var': ev}
        fdata['eigen_features'] = eigen_features

@numba.njit()
def chiSqr(x, y, startA, startB, endA, endB): # brat_curtis
    numerator = 0.0
    denominator = 0.0
    for i, j in zip(range(startA, endA), range(startB, endB)):        
        numerator += np.abs(x[i] - y[j])
        denominator += np.abs(x[i] + y[j])

    if denominator > 0.0:
        return float(numerator) / denominator
    else:
        return 0.0

@numba.njit()
def quad(x, y):
    l = x.shape[0]
    a = chiSqr(x, y, 0, 0, 36, 36)
    d = chiSqr(x, y, 36, 36, l, l)
    preA = chiSqr(x, y, 0, 1, 35, 36)
    postA = chiSqr(x, y, 1, 0, 36, 35)
    preD = chiSqr(x, y, 36, 37, l - 1, l)
    postD = chiSqr(x, y, 37, 36, l, l - 1)
    return (0.5 * (preA + postA) + a + 0.9 * (0.1 * (preD + postD) + d)) / (2.0 + 1.2*0.9)

def dist(a, b):
    res = 0.0
    for x, y in zip(a, b):
        if (abs(x) > 1e-6):
            res += ((x - y)**2 *1.0) / (1.0*x)
    return round(res, 2)

def compareTwoClusters(a, b):
    res = {}
    alla = []
    allb = []
    for x in a:
        res[x] = dist(a[x], b[x])
        alla = alla + a[x]
        allb = allb + b[x]
    res['all'] = dist(alla, allb)
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

def clusterClassData(data):
    feat = []
    tag = []
    for i, x in enumerate(data):
        for y in x['features']:
            feat.append(x['features'][y]['angle'] + x['features'][y]['dist'])
            tag.append((i, y))
    return (feat, tag)

def clusterClasses(data):
    feat, tag = clusterClassData(data)
    rndSeed = 19
    reduced_dim = umap.UMAP(n_components=8, n_neighbors=8, min_dist=0.15, metric=quad, random_state=rndSeed).fit_transform(feat).tolist()
    show_dim = TSNE(n_components=2, metric = quad, random_state=rndSeed).fit_transform(feat).tolist()
    clusterer = hdbscan.HDBSCAN(min_cluster_size=8)
    cluster_labels = clusterer.fit_predict(reduced_dim)
    class_points = {}
    class_tags = {}
    cid_classes = {}
    for i, label in enumerate(cluster_labels):
        # if label == -1: continue
        if not label in class_points: 
            class_points[label] = [[],[],[]]
            class_tags[label] = []
        class_points[label][0].append(show_dim[i][0])
        class_points[label][1].append(show_dim[i][1])
        class_points[label][2].append(0)
        class_tags[label].append(tag[i])
    print len(class_tags)
    return (class_points, class_tags)

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
        print "Classification..."
        res = ([], []) 
        try:
            res = clusterClasses(cascades['data'])
        except:
            print "error"
            print sys.exc_info()[0]
        for label in res[1]:
            for tag in res[1][label]:
                if not 'clusterClasses' in cascades['data'][tag[0]]: cascades['data'][tag[0]]['clusterClasses'] = {}
                cascades['data'][tag[0]]['clusterClasses'][tag[1]] = label
        print "finished."
        sys.stdout.flush()
        f = open(out_fname, "w")
        f.write("var cascades = \n")
        json.dump(cascades, f)
        f.write(";\nvar cluster_classes = \n")
        json.dump({"UMAP+HDBSCAN(t-SNE_for_plot)": {"show_point":res[0], "tags":res[1]}}, f)
        f.close()
    except IOError:
      print "Could not open file " + fname