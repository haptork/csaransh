import json
import numpy as np
from sklearn.decomposition import PCA
from sklearn.cluster import DBSCAN
from sklearn.manifold import TSNE
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics.cluster import adjusted_rand_score
from sklearn.metrics.cluster import adjusted_mutual_info_score
from sklearn.neighbors import NearestNeighbors
from scipy.spatial import ConvexHull
import math
import sys

import numba
import umap
import hdbscan

"""
Fits and applies PCA transformation to the given points.
Applies same transformation to other points given as second argument list.
"""


def findEigen(points, others):
    pca = PCA(n_components=3)
    pca.fit(points)
    var = pca.explained_variance_ratio_.tolist()
    eigen_coords = pca.transform(points)
    other_trans = []
    for x in others:
        temp = pca.transform(x)
        temp = [[round(x[0], 2), round(x[1], 2), round(x[2], 2)]
                for x in temp.tolist()]
        other_trans.append(temp)
    eigen_coords = [[round(x[0], 2), round(x[1], 2), round(x[2], 2)]
                    for x in eigen_coords.tolist()]
    var = [round(x, 2) for x in var]
    return eigen_coords, var, other_trans


"""
  Applies PCA transformation to the whole cascade coordinates and PKA point.
"""


def transformCascade(coords, point):
    all_coords = np.asarray(coords)
    point = np.asarray([point])
    if (len(coords) == 0):
        return(all_coords, point, [0, 0, 0])
    all_coords = all_coords[:, 0:3]
    trans_coords, var, trans_others = findEigen(all_coords, [point])
    trans_point = trans_others[0]
    return (trans_coords, trans_point, var)


"""
Applies DBSCAN to the vacancy and interstitial points separately
"""


def densityClustering(coords, trans_coords, thresh):
    vac_coords = []
    int_coords = []
    for i, x in enumerate(coords):
        if x[5] == 0:
            continue  # annihilated / psedo defect
        if x[3] == 0:
            vac_coords.append(trans_coords[i])
        else:
            int_coords.append(trans_coords[i])
    if len(vac_coords) == 0:
        labels_vac = []
    else:
        dbscan_vac = DBSCAN(eps=thresh, min_samples=3).fit(vac_coords)
        labels_vac = dbscan_vac.labels_
    dbscan_int = DBSCAN(eps=thresh, min_samples=3).fit(int_coords)
    labels_int = dbscan_int.labels_
    return (labels_vac, labels_int)


def addEigenAndSubcascades(data):
    for fdata in data:
        sys.stdout.write('\rto ' + fdata["infile"] + " "*10)
        sys.stdout.flush()
        # eigen coords and variance for cascade
        pka = [fdata['xrec'], fdata['yrec'], fdata['zrec']]
        eigen_coords, eigen_pka, var = transformCascade(fdata['coords'], pka)
        fdata['eigen_coords'] = eigen_coords
        fdata['eigen_pka'] = eigen_pka
        fdata['eigen_var'] = var
        # eigen coords and variance for each cluster
        eigen_features = {}
        for x in fdata['clusters']:
            if len(fdata['clusters'][x]) < 3:
                continue
            c = [fdata['coords'][y][:3] for y in fdata['clusters'][x]]
            ec, ev, _ = findEigen(c, [])
            eigen_features[x] = {'coords': ec, 'var': ev}
        fdata['eigen_features'] = eigen_features
        if len(fdata['coords']) == 0:
            fdata['eigen_coords'] = []
            fdata['eigen_pka'] = [round(x[0], 2)
                                  for x in fdata['eigen_pka'].tolist()]
            fdata['dclust_coords'] = {}
            fdata['dclustI_count'] = 0
            fdata['dclust_sec_impact'] = 0
            continue
        # density clustering
        bias = 3.0
        nn4 = fdata['latticeConst'] * (3**0.5)
        labelsV, labelsI = densityClustering(
            fdata['coords'], eigen_coords, bias * nn4)
        indicesV = [i for i, x in enumerate(
            fdata['coords']) if x[3] == 0 and x[5] == 1]
        subsv = {}
        for i, x in enumerate(labelsV):
            if x == -1:
                continue
            x = str(x) + "v"
            if not x in subsv:
                subsv[x] = []
            subsv[x].append(indicesV[i])
        indicesI = [i for i, x in enumerate(
            fdata['coords']) if x[3] == 1 and x[5] == 1]
        subsi = {}
        for i, x in enumerate(labelsI):
            if x == -1:
                continue
            x = str(x) + "i"
            if not x in subsi:
                subsi[x] = []
            subsi[x].append(indicesI[i])
        # sorting subsv by length
        lenV = []
        totalV = 0
        for x in subsv:
            lenV.append([len(subsv[x]), x])
            totalV += len(subsv[x])
        lenV.sort(reverse=True)
        # criterion to filter only substantial subcascades
        dclustNamesV = [x[1] for i, x in enumerate(lenV) if (
            x[0] / (totalV/len(lenV)) > 0.55 and x[0] > 4) or (i < 2 and x[0] > 4)]
        fdata['dclust_coords'] = {}
        for x in subsv:
            if x in dclustNamesV:
                fdata['dclust_coords'][x] = subsv[x]
        fdata['dclustI_count'] = len(subsi)
        dclust_len = [0, 0]
        if (len(lenV) > 0):
            dclust_len[0] = lenV[0][0]
        if (len(lenV) > 1):
            dclust_len[1] = lenV[1][0]
        fdata['dclust_sec_impact'] = 0
        if (dclust_len[0] > 0):
            fdata['dclust_sec_impact'] = dclust_len[1] * 100 / dclust_len[0]

# chiSqr distance criterion for cluster comparison


def dist(a, b):
    res = 0.0
    for x, y in zip(a, b):
        if (abs(x) > 1e-6):
            res += ((x - y)**2 * 1.0) / (1.0*x)
    return round(res, 2)


"""
compare different histograms of a cluster with all other clusters
"""

def compareWithCluster(pivotI, pivotCid, pivot, data, size, dim):
    res = {}
    res_with_size = {}
    size_tol = .4
    dim_tol = .2
    tol_cur = (int)(size * size_tol)
    for i, fdata in enumerate(data):
        for cid in fdata['features']:
            if i == pivotI and cid == pivotCid:
                continue
            score = compareTwoClusters(pivot, fdata['features'][cid])
            for key in score:
                if (not key in res):
                    res[key] = []
                res[key].append((score[key], i, cid))
                var = fdata['eigen_features'][cid]['var']
                tol1 = abs(var[0] - dim[0])
                tol2 = abs(var[0] + var[1] - dim[0] - dim[1])
                if abs(len(fdata['clusters'][cid]) - size) < tol_cur and tol1 < dim_tol and tol2 < dim_tol:
                    if (not key in res_with_size):
                        res_with_size[key] = []
                    res_with_size[key].append((score[key], i, cid))
    return res, res_with_size


"""
Helper distance function for dimensionality reduction
"""
@numba.njit()
def chiSqr(x, y, startA, startB, endA, endB):  # brat_curtis
    numerator = 0.0
    denominator = 0.0
    for i, j in zip(range(startA, endA), range(startB, endB)):
        numerator += np.abs(x[i] - y[j])
        denominator += np.abs(x[i] + y[j])

    if denominator > 0.0:
        return float(numerator) / denominator
    else:
        return 0.0

"""
Distance function for dimensionality reduction
"""
@numba.njit()
def quad1(x, y):
    l = x.shape[0]
    a = chiSqr(x, y, 0, 0, 36, 36)
    d = chiSqr(x, y, 36, 36, l, l)
    preA = chiSqr(x, y, 0, 1, 35, 36)
    postA = chiSqr(x, y, 1, 0, 36, 35)
    preD = chiSqr(x, y, 36, 37, l - 1, l)
    postD = chiSqr(x, y, 37, 36, l, l - 1)
    return (0.5 * (preA + postA) + a + 0.9 * (0.1 * (preD + postD) + d)) / (2.0 + 1.2*0.9)

"""
Distance function for dimensionality reduction
"""
@numba.njit()
def quad(x, y):
    l = x.shape[0]
    a = chiSqr(x, y, 0, 0, 36, 36)
    d = chiSqr(x, y, 36, 36, l, l)
    preA = chiSqr(x, y, 0, 1, 35, 36)
    postA = chiSqr(x, y, 1, 0, 36, 35)
    preD = chiSqr(x, y, 36, 37, l - 1, l)
    postD = chiSqr(x, y, 37, 36, l, l - 1)
    wA = 1.2
    wD = 0.9
    wAs = 0.4
    wDs = 0.25
    cA = (wAs * (preA + postA) + a) * wA / (2.0 * wAs + 1.0)
    cD = (wDs * (preD + postD) + d) * wD / (2.0 * wDs + 1.0)
    return  (cA + cD) / (wA + wD)

def clusterClassData(data):
    feat = []
    tag = []
    for i, x in enumerate(data):
        for y in x['features']:
            feat.append(x['features'][y]['angle'] + x['features'][y]['dist'])
            tag.append((i, y))
    return (feat, tag)

def quadCustom(wA, wD):
    def quad(x, y):
        l = x.shape[0]
        a = chiSqr(x, y, 0, 0, 36, 36)
        d = chiSqr(x, y, 36, 36, l, l)
        preA = chiSqr(x, y, 0, 1, 35, 36)
        postA = chiSqr(x, y, 1, 0, 36, 35)
        preD = chiSqr(x, y, 36, 37, l - 1, l)
        postD = chiSqr(x, y, 37, 36, l, l - 1)
        wAs = 0.4
        wDs = 0.25
        cA = (wAs * (preA + postA) + a) * wA / (2.0 * wAs + 1.0)
        cD = (wDs * (preD + postD) + d) * wD / (2.0 * wDs + 1.0)
        return  (cA + cD) / (wA + wD)
    return quad

"""
Adds top 5 matching clusters for each cluster in the data
"""
def addClusterCmp(data):
    topsize = 5
    feat, tag = clusterClassData(data)
    neigh = {}
    keys = ['angle', 'dist', 'all']
    quadAngle = quadCustom(1.0, 0.0)
    quadDist = quadCustom(0.0, 1.0)
    quadBoth = quad
    neigh[keys[0]] = NearestNeighbors(topsize * 3, metric=quadAngle)
    neigh[keys[1]] = NearestNeighbors(topsize * 3, metric=quadDist)
    neigh[keys[2]] = NearestNeighbors(topsize * 3, metric=quadBoth)
    dists = {} 
    neighbours = {}
    for key in neigh:
        neigh[key].fit(feat)
        dists[key], neighbours[key] = neigh[key].kneighbors()
    for index, (cascadeIndex, cid) in enumerate(tag):
        cascade = data[cascadeIndex]
        if not 'clust_cmp' in cascade:
            cascade['clust_cmp'] = {}
            cascade['clust_cmp_size'] = {}
            cascade['clust_cmp'][cid] = {}
            cascade['clust_cmp_size'][cid] = {}
        elif not cid in cascade['clust_cmp']:
            cascade['clust_cmp'][cid] = {}
            cascade['clust_cmp_size'][cid] = {}
        for key in neigh:
            cascade['clust_cmp'][cid][key] = [(x, tag[y][0], tag[y][1]) for x, y in zip(dists[key][index][:topsize], neighbours[key][index][:topsize])] 
            curLen = len(cascade['clusters'][cid])
            lenDiff = [(abs(curLen - len(data[tag[x][0]]['clusters'][tag[x][1]])), i) for i, x in enumerate(neighbours[key][index])]
            lenDiff.sort()
            cascade['clust_cmp_size'][cid][key] = [(dists[key][index][x[1]], tag[neighbours[key][index][x[1]]][0], tag[neighbours[key][index][x[1]]][1]) for x in lenDiff[:topsize]]


def trainKnn(feat, true_labels, n_neighbors, metric, weights):
    neigh = KNeighborsClassifier(n_neighbors=n_neighbors, metric=metric, weights=weights)
    neigh.fit(feat, true_labels) 
    return neigh

def getTrainData():
    f = open("training-data.json", 'r')
    di = json.load(f)
    f.close()
    return di['feat'], di['label']

def supervisedClustersClassification(testFeat):
    # raw
    trainFeat, trainLabels = getTrainData()
    raw_knn = trainKnn(trainFeat, trainLabels, 3, quad, "distance")
    # umap
    umap_feat = umap.UMAP(n_components=5, n_neighbors=12, min_dist=0.20, metric=quad, random_state=41).fit_transform(trainFeat+testFeat)
    umap_knn = trainKnn(umap_feat[:len(trainFeat)], trainLabels, 4, "minkowski", "distance")
    # tsne
    tsne_feat = TSNE(n_components=2, metric = quad, random_state=7).fit_transform(trainFeat+testFeat)
    tsne_knn = trainKnn(tsne_feat[:len(trainFeat)], trainLabels, 3, "minkowski", "distance")

    raw_test_proba = raw_knn.predict_proba(testFeat)
    umap_test_proba = umap_knn.predict_proba(umap_feat[len(trainFeat):])
    tsne_test_proba = tsne_knn.predict_proba(tsne_feat[len(trainFeat):])

    allLabels = list(set(trainLabels))
    allLabels.sort()
    soft_ensemble_labels = []
    noiseThreshold = [1.5, 0.0]
    for thresh in noiseThreshold:
        soft_ensemble_labels.append([])
        for x, y, z in zip(raw_test_proba, umap_test_proba, tsne_test_proba):
            total = 100.0 * x + y + z
            labelIndex = total.argmax()
            labelScore = total[labelIndex]
            label = allLabels[labelIndex]
            if (labelScore > thresh):    
                soft_ensemble_labels[-1].append(label)
            else:
                soft_ensemble_labels[-1].append("noise")
    return soft_ensemble_labels[0]

def unsupervisedClustersClassification(feat):
    rndSeed = 42
    reduced_dim = umap.UMAP(n_components=20, n_neighbors=30, min_dist=0.0,
                            metric=quad, random_state=rndSeed).fit_transform(feat).tolist()
    clusterer = hdbscan.HDBSCAN(min_cluster_size=8)
    return clusterer.fit_predict(reduced_dim)

def classesDataToSave(cluster_labels, show_dim, tag):
    class_points = {}
    class_tags = {}
    for i, label in enumerate(cluster_labels):
        label = str(label)
        if not label in class_points: 
            class_points[label] = [[],[],[]]
            class_tags[label] = []
        class_points[label][0].append(show_dim[i][0])
        class_points[label][1].append(show_dim[i][1])
        class_points[label][2].append(0)
        class_tags[label].append(tag[i])
    return class_points, class_tags

def clusterClasses(data):
    feat, tag = clusterClassData(data)
    rndSeed = 7
    show_dim = TSNE(n_components=2, metric=quad,
                    random_state=rndSeed).fit_transform(feat).tolist()
    supervisedLabels = supervisedClustersClassification(feat)
    unsupervisedLabels = unsupervisedClustersClassification(feat)
    classesData = [] 
    classesData.append({'name':'supervised (kNN)', 'data':classesDataToSave(supervisedLabels, show_dim, tag)})
    classesData.append({'name':'unsupervised (UMAP + HDBSCAN)', 'data':classesDataToSave(unsupervisedLabels, show_dim, tag)})
    return classesData

def addHull(data):
    for cascade in data:
        li = [x for x in cascade['coords'] if x[3] == 0 and x[5] == 1]
        li = [x[0:3] for x in li]
        cascade['hull_vol'] = 0.0
        cascade['hull_area'] = 0.0
        cascade['hull_density'] = 0.0
        cascade['hull_vertices'] = []
        cascade['hull_simplices'] = []
        cascade['hull_nvertices'] = 0
        cascade['hull_nsimplices'] = 0
        #cascade['hull_neigh'] = []
        if (len(li) < 4):
            continue
        try:
            hull = ConvexHull(li)
            cascade['hull_vol'] = round(hull.volume, 2)
            cascade['hull_area'] = round(hull.area, 2)
            cascade['hull_density'] = (cascade['n_defects'] / hull.volume)
            cascade['hull_vertices'] = hull.vertices.tolist()
            cascade['hull_simplices'] = hull.simplices.tolist()
            cascade['hull_nvertices'] = len(hull.vertices)
            cascade['hull_nsimplices'] = hull.nsimplex
        except:
            pass
            #cascade['hull_neigh'] = hull.neighbors.tolist()

# top level user functions
# -------------------------

def analyse(cascades, isAddClusterComparison=True, isAddClassification=True):
    print("Adding coordinates in eigen dimensions and convex hulls for cascades...")
    addEigenAndSubcascades(cascades)
    addHull(cascades)
    print('finished.')
    print("Adding cluster comparison...")
    addClusterCmp(cascades)
    print('finished.')
    sys.stdout.flush()
    return cascades


def analyseAndClassify(cascades, isAddClusterComparison=True, isAddClassification=True):
    print("Adding coordinates in eigen dimensions and convex hulls for cascades...")
    addEigenAndSubcascades(cascades)
    addHull(cascades)
    print('finished.')
    print("Adding cluster comparison...")
    addClusterCmp(cascades)
    print('finished.')
    sys.stdout.flush()
    print("Classification...")
    res = ([], [])
    classifications = []
    try:
        classifications = clusterClasses(cascades)
        if (len(classifications) > 0): res = classifications[0]['data']
    except:
        print("Warning during classification")
        print(sys.exc_info()[0])
    for label in res[1]:
        for tag in res[1][label]:
            if not 'clusterClasses' in cascades[tag[0]]:
                cascades[tag[0]]['clusterClasses'] = {}
            cascades[tag[0]]['clusterClasses'][tag[1]] = label
    print("finished.")
    sys.stdout.flush()
    return cascades, classifications


def saveJs(cascades, config, classification, filePath):
    di = {"meta": config, "data": cascades}
    try:
        f = open(filePath, "w")
        f.write("var cascades = \n")
        json.dump(di, f)
        f.write(";\nvar cluster_classes = \n")
        di = {}
        for x in classification:
            di[x['name']] = {"show_point":x['data'][0], "tags":x['data'][1]}
        json.dump(di, f)
        f.close()
    except IOError:
        print("Could not open file " + filePath)


def analyseAndSaveJs(cascades, config, filePath, isAddClusterComparison=True, isAddClassification=True):
    cascades, classification = analyseAndClassify(cascades)
    saveJs(cascades, config, classification, filePath)
    return cascades, classification


if __name__ == "__main__":
    fname = "cascades-data.json"
    out_fname = "cascades-data.js"
    if len(sys.argv) > 1:
        fname = sys.argv[1]
        nameWoExt = fname.split('.')[0]
        out_fname = nameWoExt + ".js"
    try:
        f = open(fname, "r+")
        cascadesWithConfig = json.load(f)
        cascades = cascadesWithConfig['data']
        f.close()
        cascades, classification = analyseAndClassify(cascades)
        saveJs(cascades, cascadesWithConfig['meta'], classification, out_fname)
    except IOError:
        print("Could not open file " + fname)
