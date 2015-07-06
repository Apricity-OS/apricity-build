_window_open = function(){
	window.open.apply(window, arguments);
}
window.Guardian =  {
    type: "app",
    CDN : "",
    backend : window.BACKEND,
    ENV : window.ENV,
    version: 20150518,
	open_web: _window_open,
    _current: chrome.app.window.current(),
    reload : function(){
        chrome.runtime.reload()
    },
	open_file: function(url){
		// TODO i18n
		chrome.i18n.getAcceptLanguages(function(langs){	
			var is_china = langs.slice(0,3).filter(function(l){
				return /^zh/.test(l);
			}).length;
			var land_page = is_china ? "index_zh.html" : "index_en.html";
			chrome.app.window.create(land_page,
			{
				frame: "none",
				bounds: {width: 800, height: 600}
			},  function(createdWindow){
				createdWindow.contentWindow._FILE = url;
			});
		})	
		/*
	 chrome.app.window.create(chrome.i18n.getMessage("index"), { frame: "none", width: 800, height: 600}, function(createdWindow){
		 createdWindow.contentWindow._FILE = url;
	 })
	 */
	},
	open_auth: function(url){
		var self = this;
		chrome.app.window.create('webproxy.html', {"width":800, "height": 600}, function(createdWindow){
		createdWindow.contentWindow.URL = url;
		createdWindow.contentWindow.addEventListener("message" , function(e){
			console.log("final get", e);
			createdWindow.close();
			self._cookie = e.data.cookie;
			Vine.trigger("FAKE_COOKIE", self._cookie);
			window.postMessage("oauthed", '*')
			LocalDB.set('cookie', self._cookie);
		})
	});	
	},
	open_frame: function(url){
		var self = this;
		chrome.app.window.create('webproxy.html', {"width":800, "height": 600}, function(createdWindow){
			createdWindow.contentWindow.URL = url;
			createdWindow.contentWindow.addEventListener("message" , function(e){
				console.log('dddd', e)
				createdWindow.close();
				window.postMessage("oauthed", '*')
			})
		});	
	},
	onunload: function(callback){
		this._current.onClosed.addListener(callback);
		//window.onbeforeunload = callback;
	},
	minimize: function(){
		this._current.minimize()
	},
	maximize: function(){
		if (this._current.isMaximized()){
			this._current.restore();
		} else {
			this._current.maximize();
		}
	},
	close: function(){
		//Session.current_file && !Session.current_file.readonly &&  SessionLock.stop()
		this._current.close()
	},
	fullscreen: function(){

		!this._current.isFullscreen() ? this._current.fullscreen() : this._current.restore();
	},
	save_file: function(title, blob){

		var config = { type: 'saveFile', suggestedName:  title };
        var toastr = window.toastr || require('libs/toastr')
        var i18n = require('i18n!nls/dict');
        var MSG = function(a){
            return i18n[a] || a;
        }
		chrome.fileSystem.chooseEntry(config, function(writableEntry) {
			if (!writableEntry) return;
			toastr.wait(MSG('Waiting'));
			writableEntry.createWriter(function(fileWriter) {

				fileWriter.onwriteend = function(e) {
					/*
					status.innerText = 'Export to '+
					fileDisplayPath+' completed';
					// You need to explicitly set the file size to truncate
					// any content that could be there before
					this.onwriteend = null;
					this.truncate(e.total);
					*/
                    console.warn(e)
				    toastr.success(MSG('Done'));
				};

                fileWriter.onerror = function(e) {
                    console.warn(e)
				    toastr.error(MSG('Error'));
				};

				fileWriter.write(blob);

			});
		})
	},
	_math_defer: {},
	render_math: function(data, callback){
		//this._math_defer[data.hash] = this._math_defer[data.hash] || [];
		if (!this._math_defer[data.hash]){
			this._math_defer[data.hash] = []
			Guardian._sandbox_defer.promise().then(function(){
				setTimeout(function(){
					$('#sandbox')[0].contentWindow.postMessage(data, '*');	
				})
			})
		}
		this._math_defer[data.hash].push(callback);
	},
	_sandbox_defer : $.Deferred(),
	change_language: function(lang){
		chrome.storage.local.set( { 'prefer_lang' :"index_" + lang + ".html" }, function(value){			
				chrome.runtime.reload()
		})
	}
}

window.addEventListener('message', function(e){
	if (e.data.type == 'render_math'){
		//var callback = Guardian._math_defer[e.data.hash]
		(Guardian._math_defer[e.data.hash] || []).map(function(callback){
			callback(e.data.content);	
		})
		delete Guardian._math_defer[e.data.hash];
		//callback && callback(e.data.content);
	}
})

var loadImage = function(uri, callback) {
  var xhr = new XMLHttpRequest();
  xhr.responseType = 'blob';
  xhr.onload = function() {
    callback(window.URL.createObjectURL(xhr.response), uri);
  }
  xhr.open('GET', uri, true);
  xhr.send();
}
// http://davidwalsh.name/detect-node-insertion
document.addEventListener("webkitAnimationStart", function(event){
	console.log('animation', event);
	if (event.animationName=='img_load'){
		var img = event.target;
		 loadImage(img.src, function(blob_uri, requested_uri) {
			 img.src = blob_uri;
		 })	 
	}
}, false); // Chrome + Safari

$('document').ready(function(){

    $('#titlebar .window-file').on('click', function(){
        $('.open-folder-btn').click();
    })
    $('#sandbox')[0].addEventListener('contentload', function() {
        Guardian._sandbox_defer.resolve();
    });
    /*
	if (!/windows/i.test(navigator.userAgent)) return;
	var window_start_pos;
	var start_pos;
	var pos;
    var is_moving = false;

	$("#titlebar").on('mousedown', function(event){
		//console.warn(event);
		var bounds = Guardian._current.outerBounds || Guardian._current.getBounds();
		window_start_pos = { x : bounds.left, y: bounds.top }
		start_pos = { x: event.screenX, y: event.screenY }
		is_moving = true
		//console.log('mousedown', window_start_pos, start_pos)
	})
	$("#titlebar, #editor").on('mousemove', function(event){
		if (!is_moving) return;
		//console.warn(event.clientX, event.pageX, event.screenX);
		var x = window_start_pos.x + event.screenX - start_pos.x, y = window_start_pos.y + event.screenY - start_pos.y;
		//console.log(x, y)
		move_window(x, y);	
		//Guardian._current.moveTo(x, y)
	}).on('mouseup', function(event){
		if (!is_moving) return;
		is_moving = false;
		var x = window_start_pos.x + event.screenX - start_pos.x, y = window_start_pos.y + event.screenY - start_pos.y;
		//console.log('mouseup', x, y, event.screenX - start_pos.x, event.screenY - start_pos.y);
		move_window(x, y);	
	});
	
	Guardian._current.onMinimized.addListener(function(){
		$('#dragbar').addClass('disabled');
    })

	function move_window(x, y){
		try {
			Guardian._current.outerBounds.left = x;
			Guardian._current.outerBounds.top = y;
		} catch(e){
			Guardian._current.moveTo(x, y);
		}
	}
    */
});

$(document).on('click', ".preview-container a[href^='#']", function(event){
    event.preventDefault();
    event.stopPropagation();
    var target = document.getElementById($.attr(event.currentTarget, 'href').substr(1))
    var container = $('.full-preview .layout-wrapper-l3, .preview-container')[0];
    container.scrollTop = target.offsetTop - 10;
})

var isMac = navigator.platform.toUpperCase().indexOf('MAC')!==-1;

$(window).focus(function(){
    var saver = $('#ime-focus-saver')
    saver.focus()
    setTimeout(function(){
        var editor = require('editor');
        saver.focus()
        editor && editor.focus &&  editor.focus()
    }, 100)
})
$(window).blur(function(){
    var editor = require('editor');
    editor.blur()
})

window.onKeyDown = function(e) { if (e.keyCode == 27 /* ESC */) { e.preventDefault(); } };

