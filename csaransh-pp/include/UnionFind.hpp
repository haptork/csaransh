 /* UnionFind.cpp
 *
 * Copyright 2015 utkarsh <haptork@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <algorithm>
#include <functional>
#include <iostream>
#include <initializer_list>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

using std::map; using std::vector;  using std::transform; using std::make_tuple;
using std::tuple; using std::tuple_element_t; using std::get; using std::set;
using std::transform; using std::initializer_list; using std::max_element;

template<size_t Index, class Tobj>
class UnionFind {
public:
  using keyType = std::tuple_element_t<Index, Tobj>;
  using valType = tuple<keyType, Tobj>;
  using const_iterator = typename map<keyType, valType>::const_iterator;

  UnionFind() = default;

  // find
  Tobj operator [] (Tobj obj) {
    auto curId = get<Index>(obj);
    if (parents.find(curId) == std::end(parents)) {
      parents[curId] = make_tuple(curId, obj);
      weights[curId] = 1;
      return obj;
    }
    vector<keyType> path { curId };
    auto root = get<0>(parents[curId]);
    while (root != path[path.size()-1]) {
      path.push_back(root);
      root = get<0>(parents[root]);
    }
    for (auto& it : path) {
      get<0>(parents[it]) = root;
    }
    return get<1>(parents[root]);
  }

  auto find (keyType curId) {
    using std::make_pair;
    if (parents.find(curId) == std::end(parents)) {
      return make_pair(false, curId);
    }
    vector<keyType> path { curId };
    auto root = get<0>(parents[curId]);
    while (root != path[path.size()-1]) {
      path.push_back(root);
      root = get<0>(parents[root]);
    }
    for (auto& it : path) {
      get<0>(parents[it]) = root;
    }
    return make_pair(true, get<0>(parents[root]));
  }


  auto begin () const {
    return std::begin(parents);
  }

  auto end () const {
    return std::end(parents);
  }

  void unite(vector<Tobj> lobj) {
    set<keyType> roots;
    for (auto it : lobj) {
      roots.insert(get<Index>((*this)[it]));
    }
    vector<std::pair<int, keyType>> wTemp;
    wTemp.resize(roots.size());
    transform(std::begin(roots), std::end(roots), std::begin(wTemp),
        [this](keyType key) {
          return std::make_pair(this->weights[key], key);
        });
    auto heaviestPair = max_element(std::begin(wTemp), std::end(wTemp));
    auto heaviest = heaviestPair->second;
    for (auto& r : roots) {
      if (r != heaviest) {
        weights[heaviest] += weights[r];
        get<0>(parents[r]) = heaviest;
      }
    }
  }

  void unite(initializer_list<Tobj> lobj) {
    unite(vector<Tobj> {lobj.begin(), lobj.end()});
  }

  template<class Pred>
  void uniteIf(const Tobj& obj, Pred&& pred) {
    vector<Tobj> vobj {obj};
    for (auto& it : parents) {
      auto& val = get<1>(it.second);
      if (pred(obj, val)) {
        vobj.push_back(val);
      }
    }
    unite(vobj);
  }

/*
  auto getAll() {
    using retType = decltype(tuple_cat(tuple<keyType>{}, Tobj{}));
    vector<retType> res;
    res.reserve(parents.size());
    for (auto it : parents) {
      auto& val = get<1>(it.second);
      res.push_back(tuple_cat(std::make_tuple(find(get<0>(it.second)).second), val));
    }
    return res;
  }
  */

  auto getAll() {
    using retType = Tobj;
    vector<retType> res;
    res.reserve(parents.size());
    for (auto it : parents) {
      auto val = get<1>(it.second);
      get<Index>(val) = find(get<0>(it.second)).second;
      res.emplace_back(val);
    }
    return res;
  }



  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    //ar & parents;
  }

  private:
    map<keyType, valType> parents;
    map<keyType, int> weights;
};

/*
int main() {
  UnionFind<0, std::tuple<int>> uf;
  uf.uniteIf<0>(std::tuple<int>{7}, [](int x, int y) { return abs(x - y) <2; });
  uf.uniteIf<0>(std::tuple<int>{9}, [](int x, int y) { return abs(x - y) <2; });
  uf.uniteIf<0>(std::tuple<int>{8}, [](int x, int y) { return abs(x - y) <2; });
  uf.uniteIf<0>(std::tuple<int>{5}, [](int x, int y) { return abs(x - y) <2; });
  auto res = uf.getAll();
  for(auto it : res) {
    std::cout<<get<0>(it)<<": "<<get<0>(get<1>(it))<<"| \n";
  }
  return 0;
}
*/
