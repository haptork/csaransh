import React from "react";
import fscreen from "fscreen";
import * as THREE from "three";
import reactCSS from "reactcss";
import Palette from "../palette";
import { ChromePicker } from "react-color";
import { uniqueKey } from "../utils";

const invertcolor = require("invert-color");
var OrbitControls = require("three-orbit-controls")(THREE);

export function getCurrentCascade(loadedCascadesOrig, row) {
  return [row];
}

export function removeCurrentCascade(loadedCascades, row) {
  var currIdx = undefined;
  for (var i = 0; i < loadedCascades.length; i++) {
    if (uniqueKey(loadedCascades[i]) === uniqueKey(row)) {
      currIdx = i;
      break;
    }
  }
  if (currIdx == undefined) {
  } else {
    loadedCascades.splice(currIdx, 1);
  }
  return loadedCascades;
}

export class CascadeVisualizer3D extends React.Component {
  constructor(props) {
    super(props);
    this.start = this.start.bind(this);
    this.stop = this.stop.bind(this);
    this.animate = this.animate.bind(this);
    this.onMouseOver = this.onMouseOver.bind(this);
    this.handleClick = this.handleClick.bind(this);
    this.showCurrentFrame = this.showCurrentFrame.bind(this);
    this.goFull = this.goFull.bind(this);
    this.state = {
      mdviewHt: "294px",
      fontSize: "6pt",
      insetSize: "30px",
      cameraState: "Perspective View ",
      anhilatedOpacityStatus: "transparent",
      displayColorPicker: false,
      displayColor: { r: "204", g: "204", b: "204", a: "1" },
      selected_frame: undefined
    };
    this.fscreen = fscreen;
    this.handler = this.handler.bind(this);
    this.fscreen.addEventListener("fullscreenchange", this.handler, false);
    this.toggleCamera = this.toggleCamera.bind(this);
    this.toggleAnhilatedOpacity = this.toggleAnhilatedOpacity.bind(this);
    this.resetCameraOrientation = this.resetCameraOrientation.bind(this);
    this.setCameraZ = this.setCameraZ.bind(this);
    this.colorPickerClick = this.colorPickerClick.bind(this);
    this.colorPickerClose = this.colorPickerClose.bind(this);
    this.colorPickerChange = this.colorPickerChange.bind(this);
    this.changeCascadeInViz = this.changeCascadeInViz.bind(this);
  }

  componentDidMount() {
    const width = this.mdview.clientWidth;
    const height = this.mdview.clientHeight;

    const scene = new THREE.Scene();

    const aspect = width / height;
    const fov = 60;
    const near = 1;
    const far = 2000;
    const cameraPerspective = new THREE.PerspectiveCamera(
      fov,
      aspect,
      near,
      far
    );
    const cameraOrtho = new THREE.OrthographicCamera(
      width / -2,
      width / 2,
      height / 2,
      height / -2,
      near,
      far
    );
    const activeCamera = cameraPerspective;
    cameraOrtho.rotation.y = Math.PI;
    cameraPerspective.rotation.y = Math.PI;
    const cameraRig = new THREE.Group();
    cameraRig.add(cameraPerspective);
    cameraRig.add(cameraOrtho);
    scene.add(cameraRig);

    const controls = new OrbitControls(activeCamera, this.mdview);

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });

    activeCamera.position.z = 4;
    // scene.add(cube)
    renderer.setClearColor("#CCCCCC");
    renderer.setSize(width, height);

    const inset = this.inset;
    const renderer2 = new THREE.WebGLRenderer();
    renderer2.setClearColor(0xeeeeee, 1);
    renderer2.setSize(30, 30);
    const scene2 = new THREE.Scene();
    const camera2 = new THREE.PerspectiveCamera(60, 1, 1, 1000);
    camera2.up = new THREE.Vector3(0, 1, 0);
    const axisHelper = new THREE.AxesHelper(150);
    scene2.add(axisHelper);
    inset.appendChild(renderer2.domElement);

    const mouse = new THREE.Vector2();
    const base_atom_size = 2.0;

    const raycaster = new THREE.Raycaster();
    raycaster.params.Points.threshold = 0.5 * base_atom_size;
    const raycaster2 = new THREE.Raycaster();
    raycaster2.params.Points.threshold = 1.0 * base_atom_size;

    const cube = new THREE.BoxBufferGeometry(1, 1, 1);
    const edges = new THREE.EdgesGeometry(cube);
    const wireframe = new THREE.LineSegments(
      edges,
      new THREE.LineBasicMaterial({ color: 0x000000 })
    );
    scene.add(wireframe);

    this.scene = scene;
    this.cameraPerspective = cameraPerspective;
    this.cameraOrtho = cameraOrtho;
    this.activeCamera = activeCamera;
    this.renderer = renderer;

    this.controls = controls;

    this.scene2 = scene2;
    this.camera2 = camera2;
    this.renderer2 = renderer2;
    this.raycaster = raycaster;
    this.raycaster2 = raycaster2;
    this.mouse = mouse;
    this.wireframe = wireframe;

    this.all_frames = {};

    var vac = new THREE.TextureLoader().load(
      "images/vacancy-1.png",
      THREE.SphericalRefractionMapping
    );
    vac.magFilter = THREE.NearestFilter;
    vac.minFilter = THREE.LinearMipMapLinearFilter;
    var inter = new THREE.TextureLoader().load(
      "images/metal-ball-1.png",
      THREE.SphericalRefractionMapping
    );
    inter.magFilter = THREE.NearestFilter;
    inter.minFilter = THREE.LinearMipMapLinearFilter;
    this.textures = {
      0: vac,
      1: inter
    };

    this.anhilatedOpacityValue = 0.4;
    this.n_frames_prev = undefined;

    this.mdview.appendChild(this.renderer.domElement);
    this.start();
  }

  componentWillUnmount() {
    this.stop();
    this.mdview.removeChild(this.renderer.domElement);
  }

  start() {
    if (!this.frameId) {
      this.frameId = requestAnimationFrame(this.animate);
    }
  }

  stop() {
    cancelAnimationFrame(this.frameId);
  }

  animate() {
    this.renderScene();

    this.frameId = window.requestAnimationFrame(this.animate);

    this.controls.object = this.activeCamera;
    this.controls.update();
    //this.activeCamera.lookAt(this.scene.position);
    this.camera2.position.copy(this.activeCamera.position);
    this.camera2.position.sub(this.controls.target);
    this.camera2.position.setLength(400);
    this.camera2.lookAt(this.scene2.position);
    this.renderer2.render(this.scene2, this.camera2);
    // if(!this.last_frame_name){
    //   // clear the select box
    //   // show aprop msg
    //   this.whichCascade.innerHTML = "";
    // }
    // else{
    //   // atleast one data pt is there
    //   // fill the selectbox with whatever is there in all_frames
    //   // current selection is last last_frame_name in this.props.data
    //   this.whichCascade.innerHTML = "";
    //   for(var i=0; i<this.props.data.length; i++){
    //     // this.whichCascade.add(<option>i</option>);
    //   }
    //   // this.whichCascade.innerHTML = this.last_frame_name + " " + this.props.data.length;
    // }
  }

  setCameraZ() {
    const frame = this.all_frames[this.last_frame_name];
    if (frame !== undefined) {
      const coords = frame["coords"];
      var geom = new THREE.Geometry();
      coords.forEach(e => {
        const [x, y, z, iorv, clusterid, isanhilated] = e;
        geom.vertices.push(new THREE.Vector3(x, y, z));
      });
      geom.computeBoundingSphere();
      const r = geom.boundingSphere.radius;
      this.activeCamera.position.z = 1.1 * r;
      this.activeCamera.lookAt(geom.boundingSphere.center);
    } else {
      this.activeCamera.position.z = 4;
    }
  }

  showCurrentFrame() {
    const frame = this.all_frames[this.last_frame_name];
    if (frame !== undefined) {
      const boxsize = frame["boxSize"];
      this.wireframe.scale.set(boxsize, boxsize, boxsize);
      this.resetCameraOrientation();
      frame["ps"].forEach(ps => {
        this.scene.add(ps);
        ps.children.forEach(p => {
          if (p.material.userData.anh == 0) {
            p.material.opacity = this.anhilatedOpacityValue;
          }
        });
      });
      if (frame['pka']) this.scene.add(frame["pka"]);
      if (this.props.focusOn) {
        const [row_name, cluster_id] = this.props.focusOn;
        if (row_name == this.last_frame_name) {
          var geom = new THREE.Geometry();
          coords.forEach(e => {
            const [x, y, z, iorv, clusterid, isanhilated] = e;
            if (clusterid === cluster_id) {
              geom.vertices.push(new THREE.Vector3(x, y, z));
            }
          });
          geom.computeBoundingSphere();
          const r = geom.boundingSphere.radius;
          this.activeCamera.position.z = 1.3 * r;
          // this.activeCamera.lookAt(geom.boundingSphere.center);
        }
        const coords = frame["coords"];
        var geom = new THREE.Geometry();
        coords.forEach(e => {
          const [x, y, z, iorv, clusterid, isanhilated] = e;
          geom.vertices.push(new THREE.Vector3(x, y, z));
        });
        geom.computeBoundingSphere();
        const r = geom.boundingSphere.radius;
        this.activeCamera.position.z = 1.3 * r;
        // this.activeCamera.lookAt(geom.boundingSphere.center);
      }
    }
  }

  resetCameraOrientation() {
    const frame = this.all_frames[this.last_frame_name];
    const pos = [frame['xrec'], frame['yrec'], frame['zrec']];
    this.wireframe.position.set(pos[0], pos[1], pos[2]);
    this.controls.target.set(pos[0], pos[1], pos[2]);
    this.activeCamera.position.x = pos[0];
    this.activeCamera.position.y = pos[1];
    this.setCameraZ();
  }

  renderScene() {
    if (this.props.data) {
      // check if last frame is still the same or is it shuffled
      var data = this.props.data;
      var n_frames = data.length;
      var this_frame_data = data[n_frames - 1];
      if (n_frames === 0) {
        // handle deletion of all frames
        // clear all drawn stuff
        if (this.last_frame_name !== undefined) {
          this.all_frames[this.last_frame_name]["ps"].forEach(ps => {
            this.scene.remove(ps);
          });
          if (this.all_frames[this.last_frame_name]["pka"]) {
            this.scene.remove(this.all_frames[this.last_frame_name]["pka"]);
          }
        }
        delete this.all_frames[this.last_frame_name];
        this.last_frame_name = undefined;
      }
      if (n_frames > 0 && uniqueKey(data[n_frames - 1]) !== this.last_frame_name) {
        // remove pka-arrow and frame-group from the scene
        if (this.last_frame_name !== undefined) {
          this.all_frames[this.last_frame_name]["ps"].forEach(ps => {
            this.scene.remove(ps);
          });
          if (this.all_frames[this.last_frame_name]["pka"]) {
            this.scene.remove(this.all_frames[this.last_frame_name]["pka"]);
          }
        }
        this.last_frame_name = uniqueKey(data[n_frames - 1]);
        //console.log(this.last_frame_name);
        if (this.last_frame_name in this.all_frames) {
          // if it already exists
          this.showCurrentFrame();
        } else {
          this.all_frames[this.last_frame_name] = {};
          // create ps and pka
          var coords = this_frame_data["coords"];
          this.all_frames[this.last_frame_name]["coords"] = coords;
          this.all_frames[this.last_frame_name]["xyzFilePath"] = this_frame_data['xyzFilePath'];
          var energy = this_frame_data["energy"];
          var frequency = this_frame_data["clusterSizes"];
          var classes = this_frame_data.hasOwnProperty("clusterClasses") ? this_frame_data.clusterClasses.savi : {};
          var clusterids = new Set();
          var points_dict = {};
          coords.forEach(c => {
            const [x, y, z, iorv, clusterid, isanhilated] = c;
            const key =
              String(parseInt(iorv)) +
              "_" +
              String(parseInt(clusterid)) +
              "_" +
              String(parseInt(isanhilated));
            if (!(key in points_dict)) {
              points_dict[key] = [];
            }
            var particle = new THREE.Vector3(x, y, z);
            points_dict[key].push(particle);
            clusterids.add(clusterid);
          });
          const n_clusters = clusterids.size;
          clusterids = Array.from(clusterids);
          const colors = Palette("mpn65", Math.min(n_clusters, 65));
          const my_palette = {};
          for (var i = 0; i < n_clusters; i++) {
            my_palette[clusterids[i]] = colors[i % colors.length];
          }
          var particles_systems = [];
          for (var key in points_dict) {
            var temp = key.split("_");
            var iorv = parseInt(temp[0]);
            var cid = parseInt(temp[1]);
            var anh = parseInt(temp[2]);
            var points = points_dict[key];
            var color = "#" + my_palette[cid];
            if (cid == 0) {
              color = "#ffffff";
            }
            var spriteMap = this.textures[iorv];
            var matname = "";
            if (cid != 0) {
              if (classes.hasOwnProperty(cid)) matname += "click to view cluster comparison below<br/>";
              matname += "cluster-id: " + cid;
              matname += "<br/>cluster-size: " + Math.abs(frequency[cid]);
              matname += "<br/>cluster-type: ";
              matname += (frequency[cid] < 0) ? "vacancy" : "interstitial";
              if (classes.hasOwnProperty(cid) && classes[cid] >= 0) matname += "<br/>cluster-class: " + classes[cid];
            } else {
              matname += "Single defect";
            }
            matname += "<br/> defect-type: ";
            matname += (iorv == 0) ? "vacancy" : "interstitial";
            matname += "<br/>Is anhilated: ";
            matname += (anh === 1) ? "no" : "yes";
            var opacity = anh === 0 ? this.anhilatedOpacityValue : 0.8;

            var material = new THREE.SpriteMaterial({
              map: spriteMap,
              color: new THREE.Color(color),
              opacity: opacity,
              userData: { name: matname, iorv: iorv, anh: anh, cluster_id: cid }
            });

            var grp = new THREE.Group();
            points.forEach(pt => {
              var tt = new THREE.Sprite(material);
              tt.position.set(pt.x, pt.y, pt.z);
              grp.add(tt);
            });
            particles_systems.push(grp);
          }
          this.all_frames[this.last_frame_name]["ps"] = particles_systems;

          var xrec = this_frame_data["xrec"];
          var yrec = this_frame_data["yrec"];
          var zrec = this_frame_data["zrec"];
          var recphi = this_frame_data["recphi"];
          var rectheta = this_frame_data["rectheta"];
          this.all_frames[this.last_frame_name]["xrec"] = xrec;
          this.all_frames[this.last_frame_name]["yrec"] = yrec;
          this.all_frames[this.last_frame_name]["zrec"] = zrec;
          if (this_frame_data['isPkaGiven'] === 1) {
            var theta = Math.PI * (parseFloat(rectheta) / 180.0);
            var phi = Math.PI * (parseFloat(recphi) / 180.0);
            var dir = new THREE.Vector3(
              Math.sin(theta) * Math.cos(phi),
              Math.sin(theta) * Math.sin(phi),
              Math.cos(theta)
            );
            dir.normalize();
            var size = energy / 10.0;
            this.all_frames[this.last_frame_name]["pka"] = new THREE.ArrowHelper(
              dir,
              new THREE.Vector3(xrec, yrec, zrec),
              size * 2,
              0xff0000,
              size,
              size / 2
            );
          }
         this.showCurrentFrame();
        }
      }
    }
    this.renderer.render(this.scene, this.activeCamera);
    if (this.props.data && this.last_frame_name !== this.state.selected_frame) {
      this.selected_frame = this.last_frame_name;
      var content = "";
      var idx = -1;
      for (var i = 0; i < this.props.data.length; i++) {
        content = content + "<option>" + this.props.data[i].name + "</option>";
        if (uniqueKey(this.props.data[i]) === this.selected_frame) {
          idx = i;
        }
      }
      this.whichCascade.innerHTML = content;
      this.whichCascade.selectedIndex = idx;
    } else if (this.props.data && this.props.data.length == 0) {
      this.whichCascade.innerHTML = "";
    }
  }

  handleClick(evt) {
    var rect = evt.target.getBoundingClientRect();
    this.mouse.x =
      ((evt.clientX - rect.left) / this.mdview.clientWidth) * 2 - 1;
    this.mouse.y =
      -((evt.clientY - rect.top) / this.mdview.clientHeight) * 2 + 1;
    this.raycaster2.setFromCamera(this.mouse, this.activeCamera);
    if (this.all_frames[this.last_frame_name] !== undefined) {
      var mindist = 999999.0;
      var minis = undefined;
      var frame = this.all_frames[this.last_frame_name];
      var ps = frame["ps"];
      for (var idx = 0; idx < ps.length; idx++) {
        var isect = this.raycaster2.intersectObjects(ps[idx].children);
        if (isect.length > 0) {
          isect.forEach(is_ => {
            if (is_.distance < mindist) {
              mindist = is_.distance;
              minis = is_;
            }
          });
        }
      }
      if (minis) {
        var obj = minis.object;
        this.props.clickHandler(obj.material.userData.cluster_id);
      }
    }
  }
 

  onMouseOver(evt) {
    var rect = evt.target.getBoundingClientRect();
    this.mouse.x =
      ((evt.clientX - rect.left) / this.mdview.clientWidth) * 2 - 1;
    this.mouse.y =
      -((evt.clientY - rect.top) / this.mdview.clientHeight) * 2 + 1;
    this.raycaster2.setFromCamera(this.mouse, this.activeCamera);
    if (this.all_frames[this.last_frame_name] !== undefined) {
      var mindist = 999999.0;
      var minis = undefined;
      var frame = this.all_frames[this.last_frame_name];
      var ps = frame["ps"];
      for (var idx = 0; idx < ps.length; idx++) {
        var isect = this.raycaster2.intersectObjects(ps[idx].children);
        if (isect.length > 0) {
          isect.forEach(is_ => {
            if (is_.distance < mindist) {
              mindist = is_.distance;
              minis = is_;
            }
          });
        }
      }
      if (minis) {
        var obj = minis.object;
        var pos = obj.position;
        var name = obj.material.userData.name;
        var fg = "#" + obj.material.color.getHexString();
        var bg = invertcolor(fg, true);
        var msg =
          "<div style='opacity: 0.7; background-color:" +
          bg +
          "; color:" +
          fg +
          ";'>" +
          name +
          "<br/>coords: " +
          pos.x +
          ", " +
          pos.y +
          ", " +
          pos.z +
          "</div>";
        this.infoBox.innerHTML = msg;
      } else {
        this.infoBox.innerHTML = "";
      }
    }
  }

  goFull() {
    this.fscreen.requestFullscreen(this.viz);
  }

  handler() {
    if (this.fscreen.fullscreenElement !== null) {
      var width = window.innerWidth;
      var height = window.innerHeight - 100;
      this.setState({ mdviewHt: height });
      this.setState({ fontSize: "12pt" });
      this.setState({ insetSize: "100px" });

      this.cameraPerspective.aspect = width / height;
      this.cameraPerspective.updateProjectionMatrix();

      this.cameraOrtho.left = -width / 2;
      this.cameraOrtho.right = width / 2;
      this.cameraOrtho.top = height / 2;
      this.cameraOrtho.bottom = -height / 2;

      //this.activeCamera.lookAt(this.scene);

      this.renderer.setSize(width, height);
      this.renderer.render(this.scene, this.activeCamera);
      this.renderer2.setSize(100, 100);
      this.controls.update();
    } else {
      this.setState({ mdviewHt: "294px" });
      this.setState({ fontSize: "6pt" });
      this.setState({ insetSize: "30px" });
      var width = this.mdview.clientWidth;
      var height = this.mdview.clientHeight;

      this.cameraPerspective.aspect = width / height;
      this.cameraPerspective.updateProjectionMatrix();

      this.cameraOrtho.left = -width / 2;
      this.cameraOrtho.right = width / 2;
      this.cameraOrtho.top = height / 2;
      this.cameraOrtho.bottom = -height / 2;

      //this.activeCamera.lookAt(this.scene);

      this.renderer.setSize(width, height);
      this.renderer2.setSize(30, 30);
      this.renderer.render(this.scene, this.activeCamera);
      this.controls.update();
    }
  }

  toggleCamera() {
    if (this.activeCamera === this.cameraPerspective) {
      this.activeCamera = this.cameraOrtho;
      this.setState({ cameraState: "Orthographic View" });
    } else {
      this.activeCamera = this.cameraPerspective;
      this.setState({ cameraState: "Perspective View " });
    }
    this.showCurrentFrame();
  }

  toggleAnhilatedOpacity() {
    if (this.last_frame_name !== undefined) {
      if (this.state.anhilatedOpacityStatus == "transparent") {
        this.anhilatedOpacityValue = 0.0;
        this.setState({ anhilatedOpacityStatus: "hidden     " });
      } else if (this.state.anhilatedOpacityStatus == "hidden     ") {
        this.setState({ anhilatedOpacityStatus: "visible    " });
        this.anhilatedOpacityValue = 0.8;
      } else if (this.state.anhilatedOpacityStatus == "visible    ") {
        this.setState({ anhilatedOpacityStatus: "transparent" });
        this.anhilatedOpacityValue = 0.4;
      }
      var frame = this.all_frames[this.last_frame_name];
      if (frame !== undefined) {
        var psgrp = frame["ps"];
        psgrp.forEach(ps => {
          ps.children.forEach(pt => {
            if (pt.material.userData.anh === 0) {
              pt.material.opacity = this.anhilatedOpacityValue;
            }
          });
        });
      }
    }
  }

  colorPickerClick() {
    this.setState({ displayColorPicker: !this.state.displayColorPicker });
  }

  colorPickerClose() {
    this.setState({ displayColorPicker: false });
  }

  colorPickerChange(color) {
    this.setState({ displayColor: color.rgb });
    this.renderer.setClearColor(color.hex);
  }

  changeCascadeInViz() {
    // var idx = this.whichCascade.selectedIndex;
    // if(this.props.data){
    //   this.last_frame_name = this.props.data[idx].name;
    //   // rotate this.props.data
    //   this.props.d = rotateAr(this.props.data, idx + 1 - this.props.data.length)
    // }
  }

  render() {
    const styles = reactCSS({
      default: {
        popover: {
          position: "absolute",
          bottom: "0px",
          right: "0px",
          zIndex: "2"
        },
        cover: {
          position: "fixed",
          top: "0px",
          right: "0px",
          left: "0px",
          bottom: "0px"
        }
      }
    });
    return (
      <div
        id="viz"
        style={{ position: "relative" }}
        ref={viz => {
          this.viz = viz;
        }}
      >
        <div
          id="mdview"
          style={{
            width: "100%",
            height: this.state.mdviewHt,
            margin: 0,
            padding: 0
          }}
          ref={mdview => {
            this.mdview = mdview;
          }}
          onMouseMove={this.onMouseOver}
          onClick={this.handleClick}
        />
        <div
          id="inset"
          style={{
            width: this.state.insetSize,
            height: this.state.insetSize,
            position: "absolute",
            top: 0,
            right: 0,
            zIndex: 10
          }}
          ref={inset => {
            this.inset = inset;
          }}
        />
        <div
          id="infoBox"
          style={{
            position: "absolute",
            top: 0,
            left: 0,
            fontSize: this.state.fontSize,
            zIndex: 10
          }}
          ref={infoBox => {
            this.infoBox = infoBox;
          }}
        />
        <button className={`btn btn-sm btn-light`} onClick={this.goFull}>
         Fullscreen
        </button>
        <button className={`btn btn-sm btn-light`} onClick={this.resetCameraOrientation}>
         Reset Camera
        </button>
        <button
          className={`btn btn-sm btn-light`} 
          style={{ backgroundColor: this.state.displayColor }}
          onClick={this.colorPickerClick}
        >
          BG
        </button>

        <button className={`btn btn-sm btn-light`} onClick={this.toggleCamera}>
        {this.state.cameraState}
        </button>
        <button className={`btn btn-sm btn-light`} onClick={this.toggleAnhilatedOpacity}>
          Anhilated: {this.state.anhilatedOpacityStatus}
        </button>
        {this.state.displayColorPicker ? (
          <div style={styles.popover}>
            <div style={styles.cover} onClick={this.colorPickerClose} />
            <ChromePicker
              disableAlpha
              color={this.state.displayColor}
              onChange={this.colorPickerChange}
            />
          </div>
        ) : null}
        <select
          style={{ visibility: "hidden" }}
          value={this.last_frame_name}
          id="whichCascade"
          ref={whichCascade => {
            this.whichCascade = whichCascade;
          }}
          onChange={this.changeCascadeInViz}
        />
      </div>
    );
  }
}
