window.onload = function () { 
	$("#foo").attr("src", window.URL || "");
	var webview = $("#foo")[0];
	webview.addEventListener('contentload', function() {
		webview.contentWindow.postMessage("init", "*")
	});

}
