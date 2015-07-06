/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */

/*global performance */

var adStandards = {
		standards: { // height: [width, width, ...]
			31: [88],
			50: [300, 320, 924],
			60: [120, 234, 300, 468, 954],
			66: [970],
			90: [120, 728, 990],
			100: [300],
			120: [290],
			125: [125, 300, 740],
			150: [180, 490],
			160: [300],
			177: [225],
			200: [200, 410],
			240: [120],
			250: [250, 300, 970],
			280: [336],
			300: [720],
			310: [300],
			360: [640],
			400: [240],
			480: [640],
			600: [120, 160, 300, 425],
			850: [336]
		},
		exists: function (height, width) {
			if (!this.standards.hasOwnProperty(height)) { return false; }
			if (this.standards[height].indexOf(width) === -1) { return false; }
			return true;
		}
	};

// Chrome 20-25 uses chrome.extension.sendMessage, Chrome >= 26 uses chrome.runtime.sendMessage
var cr = chrome.runtime,
	sendMessage = cr && cr.sendMessage || chrome.extension.sendMessage;

var seenChildren = [];
// helps analyzePageInfo() mark elements as seen and avoid counting ad spots twice
function markChildren(ele) {
	for (var n = 0; n < ele.length; n++) {
		if (ele[n].hasChildNodes()) {
			markChildren(ele[n].childNodes);
		}
		seenChildren.push(ele[n]);
	}
}

// calculates page domain, # of adSpots and latency. sends pageInfo to background.js
function analyzePageInfo() {
	var el, i, h, w,
		pageLatency = 0,
		spots = 0,
		d = document,
		html = d.querySelectorAll('iframe, div, img, object');
	
	for (i = 0; i < html.length; i++) {
		el = html[i];
		// check if element has already been seen
		if (seenChildren.indexOf(el) !== -1) { continue; }
		h = parseInt(window.getComputedStyle(el).getPropertyValue('height'), 10);
		w = parseInt(window.getComputedStyle(el).getPropertyValue('width'), 10);
		// if element matches adStandard
		if (adStandards.exists(h, w)) {
			// mark element's children as seen
			markChildren(el.childNodes);
			// count adSpot
			spots++;
		}
	}

	pageLatency = (performance.timing.domContentLoadedEventStart - performance.timing.requestStart);

	if ((pageLatency > 0) || (spots > 0)) {
		var host = d.location.host,
			pathname = d.location.pathname,
			protocol = d.location.protocol;
		sendMessage({
			name: 'recordPageInfo',
			message: {
				domain: protocol + "//" + host + pathname,
				spots: spots,
				latency: pageLatency
			}
		});
	}
	spots = null;
	seenChildren = null;
}

// since this script runs on document_idle, which does not guarantee
// onLoad event has triggered (used for latency calculation), check for it.
var state = document.readyState;
if (state != "complete") {
	document.onreadystatechange = function () {
		state = document.readyState;
		if (state == "complete") {
			analyzePageInfo();
		}
	};
} else {
	analyzePageInfo();
}
