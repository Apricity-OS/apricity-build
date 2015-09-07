/*jshint unused:false */
var require = {
	baseUrl: '.',
	paths: {
		tpl: "data/templates/precompiled",
		jquery: "lib/vendor/jquery-1.7.2",
		jqueryui: "lib/vendor/jquery-ui-1.10.4.custom",
		underscore: "lib/vendor/underscore-1.4.3",
		backbone: "lib/vendor/backbone-0.9.10",
		apprise: "lib/vendor/apprise/apprise-1.5.full",
		tiptip: "lib/vendor/tipTip/jquery.tipTip",
		moment: "lib/vendor/moment/moment",
		scrollintogreatness: "lib/vendor/jquery.scrollintogreatness-2.0.0",
		bootstrap: "lib/vendor/bootstrap/bootstrap"
	},
	shim: {
		underscore: {
			exports: "_"
		},
		backbone: {
			deps: ["underscore", "jquery"],
			exports: "Backbone"
		},
		apprise: {
			exports: "apprise"
		},
		tiptip: {
			deps: ["jquery"]
		},
		scrollintogreatness: {
			deps: ["jquery"]
		},
		bootstrap: {
			deps: ["jquery"]
		}
	},
	waitSeconds: 0
};
