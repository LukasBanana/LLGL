
  var Module = typeof Module !== 'undefined' ? Module : {};

  if (!Module.expectedDataFileDownloads) {
    Module.expectedDataFileDownloads = 0;
  }

  Module.expectedDataFileDownloads++;
  (function() {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    if (Module['ENVIRONMENT_IS_PTHREAD'] || Module['$ww']) return;
    var loadPackage = function(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'Example_MorphTargets.data';
      var REMOTE_PACKAGE_BASE = 'Example_MorphTargets.data';
      if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
        Module['locateFile'] = Module['locateFilePackage'];
        err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
      }
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        if (typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string') {
          require('fs').readFile(packageName, function(err, contents) {
            if (err) {
              errback(err);
            } else {
              callback(contents.buffer);
            }
          });
          return;
        }
        var xhr = new XMLHttpRequest();
        xhr.open('GET', packageName, true);
        xhr.responseType = 'arraybuffer';
        xhr.onprogress = function(event) {
          var url = packageName;
          var size = packageSize;
          if (event.total) size = event.total;
          if (event.loaded) {
            if (!xhr.addedTotal) {
              xhr.addedTotal = true;
              if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
              Module.dataFileDownloads[url] = {
                loaded: event.loaded,
                total: size
              };
            } else {
              Module.dataFileDownloads[url].loaded = event.loaded;
            }
            var total = 0;
            var loaded = 0;
            var num = 0;
            for (var download in Module.dataFileDownloads) {
            var data = Module.dataFileDownloads[download];
              total += data.total;
              loaded += data.loaded;
              num++;
            }
            total = Math.ceil(total * Module.expectedDataFileDownloads/num);
            if (Module['setStatus']) Module['setStatus'](`Downloading data... (${loaded}/${total})`);
          } else if (!Module.dataFileDownloads) {
            if (Module['setStatus']) Module['setStatus']('Downloading data...');
          }
        };
        xhr.onerror = function(event) {
          throw new Error("NetworkError for: " + packageName);
        }
        xhr.onload = function(event) {
          if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            var packageData = xhr.response;
            callback(packageData);
          } else {
            throw new Error(xhr.statusText + " : " + xhr.responseURL);
          }
        };
        xhr.send(null);
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS() {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
Module['FS_createPath']("/", "assets", true, true);

      /** @constructor */
      function DataRequest(start, end, audio) {
        this.start = start;
        this.end = end;
        this.audio = audio;
      }
      DataRequest.prototype = {
        requests: {},
        open: function(mode, name) {
          this.name = name;
          this.requests[name] = this;
          Module['addRunDependency'](`fp ${this.name}`);
        },
        send: function() {},
        onload: function() {
          var byteArray = this.byteArray.subarray(this.start, this.end);
          this.finish(byteArray);
        },
        finish: function(byteArray) {
          var that = this;
          // canOwn this data in the filesystem, it is a slide into the heap that will never change
          Module['FS_createDataFile'](this.name, null, byteArray, true, true, true);
          Module['removeRunDependency'](`fp ${that.name}`);
          this.requests[this.name] = null;
        }
      };

      var files = metadata['files'];
      for (var i = 0; i < files.length; ++i) {
        new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio'] || 0).open('GET', files[i]['filename']);
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
          var files = metadata['files'];
          for (var i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }          Module['removeRunDependency']('datafile_Example_MorphTargets.data');

      };
      Module['addRunDependency']('datafile_Example_MorphTargets.data');

      if (!Module.preloadResults) Module.preloadResults = {};

      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS();
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/assets/Book.png", "start": 0, "end": 240771}, {"filename": "/assets/BookLeftPage.obj", "start": 240771, "end": 243867}, {"filename": "/assets/BookPageKeyframe_0.obj", "start": 243867, "end": 246640}, {"filename": "/assets/BookPageKeyframe_1.obj", "start": 246640, "end": 248584}, {"filename": "/assets/BookPageKeyframe_10.obj", "start": 248584, "end": 250722}, {"filename": "/assets/BookPageKeyframe_11.obj", "start": 250722, "end": 252856}, {"filename": "/assets/BookPageKeyframe_12.obj", "start": 252856, "end": 254983}, {"filename": "/assets/BookPageKeyframe_13.obj", "start": 254983, "end": 257085}, {"filename": "/assets/BookPageKeyframe_14.obj", "start": 257085, "end": 259135}, {"filename": "/assets/BookPageKeyframe_15.obj", "start": 259135, "end": 261160}, {"filename": "/assets/BookPageKeyframe_2.obj", "start": 261160, "end": 263156}, {"filename": "/assets/BookPageKeyframe_3.obj", "start": 263156, "end": 265182}, {"filename": "/assets/BookPageKeyframe_4.obj", "start": 265182, "end": 267289}, {"filename": "/assets/BookPageKeyframe_5.obj", "start": 267289, "end": 269394}, {"filename": "/assets/BookPageKeyframe_6.obj", "start": 269394, "end": 271533}, {"filename": "/assets/BookPageKeyframe_7.obj", "start": 271533, "end": 273696}, {"filename": "/assets/BookPageKeyframe_8.obj", "start": 273696, "end": 275827}, {"filename": "/assets/BookPageKeyframe_9.obj", "start": 275827, "end": 277962}, {"filename": "/assets/BookRightPage.obj", "start": 277962, "end": 281026}, {"filename": "/assets/BookShell.obj", "start": 281026, "end": 287825}, {"filename": "/assets/Book_Page0.png", "start": 287825, "end": 447569}, {"filename": "/assets/Book_PaperDetailMap.png", "start": 447569, "end": 530285}, {"filename": "/assets/Crate.jpg", "start": 530285, "end": 589971}, {"filename": "/assets/Logo_Direct3D11.png", "start": 589971, "end": 598531}, {"filename": "/assets/Logo_Direct3D12.png", "start": 598531, "end": 613888}, {"filename": "/assets/Logo_LLGL.png", "start": 613888, "end": 629558}, {"filename": "/assets/Logo_Metal.png", "start": 629558, "end": 643641}, {"filename": "/assets/Logo_OpenGL.png", "start": 643641, "end": 709588}, {"filename": "/assets/Logo_Vulkan.png", "start": 709588, "end": 720542}, {"filename": "/assets/MorphTargets.PMorphTargetMesh.300es.frag", "start": 720542, "end": 722182}, {"filename": "/assets/MorphTargets.PStaticMesh.300es.frag", "start": 722182, "end": 723015}, {"filename": "/assets/MorphTargets.VMorphTargetMesh.300es.vert", "start": 723015, "end": 724202}, {"filename": "/assets/MorphTargets.VStaticMesh.300es.vert", "start": 724202, "end": 725185}], "remote_package_size": 725185});

  })();
