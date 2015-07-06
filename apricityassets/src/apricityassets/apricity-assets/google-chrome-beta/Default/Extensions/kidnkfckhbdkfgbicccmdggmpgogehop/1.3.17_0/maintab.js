// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var counter=0;
var cookie;
document.addEventListener('DOMContentLoaded', function() {

	$('#opentab').click(function(){
		chrome.app.window.create('webproxy.html', {"width":800, "height": 600}, function(createdWindow){
			
			createdWindow.contentWindow.URL = "http://app.maxiang.io/pauth/redirect?service=yinxiang";
			createdWindow.contentWindow.addEventListener("message" , function(e){
				console.log("final get", e);
				$('#result').html(JSON.stringify(e.data));
				cookie = e.data.cookie
				createdWindow.close();
			})
			
		});
	})
	$('#getuser').click(function(){
		$.ajax({ url : "http://app.maxiang.io/evernote/user", headers: {'X-PCookie' : JSON.stringify(cookie) } }).done(function(data){
			$('#result').append("<pre>" + JSON.stringify(data) + "</pre>");
		})
	})
});

