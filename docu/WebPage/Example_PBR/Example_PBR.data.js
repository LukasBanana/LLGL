
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
      var PACKAGE_NAME = 'Example_PBR.data';
      var REMOTE_PACKAGE_BASE = 'Example_PBR.data';
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
          }          Module['removeRunDependency']('datafile_Example_PBR.data');

      };
      Module['addRunDependency']('datafile_Example_PBR.data');

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
    loadPackage({"files": [{"filename": "/assets/Example.Mesh.450core.frag", "start": 0, "end": 3808}, {"filename": "/assets/Example.Mesh.450core.vert", "start": 3808, "end": 4855}, {"filename": "/assets/Example.Mesh.frag", "start": 4855, "end": 8166}, {"filename": "/assets/Example.Mesh.vert", "start": 8166, "end": 9014}, {"filename": "/assets/Example.PMesh.300es.frag", "start": 9014, "end": 11381}, {"filename": "/assets/Example.PSky.300es.frag", "start": 11381, "end": 12127}, {"filename": "/assets/Example.Sky.450core.frag", "start": 12127, "end": 12913}, {"filename": "/assets/Example.Sky.450core.vert", "start": 12913, "end": 14064}, {"filename": "/assets/Example.Sky.frag", "start": 14064, "end": 14694}, {"filename": "/assets/Example.Sky.vert", "start": 14694, "end": 15747}, {"filename": "/assets/Example.VMesh.300es.vert", "start": 15747, "end": 16874}, {"filename": "/assets/Example.VSky.300es.vert", "start": 16874, "end": 17347}, {"filename": "/assets/Metal04_col.jpg", "start": 17347, "end": 1259397}, {"filename": "/assets/Metal04_met.jpg", "start": 1259397, "end": 1661521}, {"filename": "/assets/Metal04_nrm.jpg", "start": 1661521, "end": 2699894}, {"filename": "/assets/Metal04_rgh.jpg", "start": 2699894, "end": 3340459}, {"filename": "/assets/Tiles22_col.jpg", "start": 3340459, "end": 4485527}, {"filename": "/assets/Tiles22_nrm.jpg", "start": 4485527, "end": 5801560}, {"filename": "/assets/Tiles22_rgh.jpg", "start": 5801560, "end": 6573767}, {"filename": "/assets/Tiles26_col.jpg", "start": 6573767, "end": 7920799}, {"filename": "/assets/Tiles26_nrm.jpg", "start": 7920799, "end": 10302643}, {"filename": "/assets/Tiles26_rgh.jpg", "start": 10302643, "end": 10935028}, {"filename": "/assets/UVCube.obj", "start": 10935028, "end": 10945187}, {"filename": "/assets/UVSphere.obj", "start": 10945187, "end": 11257508}, {"filename": "/assets/WiredBox.obj", "start": 11257508, "end": 11275430}, {"filename": "/assets/Wood13_col.jpg", "start": 11275430, "end": 12321864}, {"filename": "/assets/Wood13_nrm.jpg", "start": 12321864, "end": 13423218}, {"filename": "/assets/Wood13_rgh.jpg", "start": 13423218, "end": 14025331}, {"filename": "/assets/alpha-island_bk.tga", "start": 14025331, "end": 14811781}, {"filename": "/assets/alpha-island_dn.tga", "start": 14811781, "end": 15830041}, {"filename": "/assets/alpha-island_ft.tga", "start": 15830041, "end": 16616491}, {"filename": "/assets/alpha-island_lf.tga", "start": 16616491, "end": 17402941}, {"filename": "/assets/alpha-island_rt.tga", "start": 17402941, "end": 18189391}, {"filename": "/assets/alpha-island_up.tga", "start": 18189391, "end": 19204107}], "remote_package_size": 19204107});

  })();
