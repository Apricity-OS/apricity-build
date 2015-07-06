define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='';

var _select = require('tpl/_select');

__p+='\n\n<div id="trackers-app-browser" class="app-browser">\n\t<div class="app-browser-header">\n\t\t<div class="app-browser-header-left">\n\t\t\t<span id="block-status"></span>\n\t\t\t<br>\n\t\t\t<br>\n\t\t\t<span style="font-size: .75em;">\n\t\t\t\t'+
((__t=( t("options_blocking3") ))==null?'':__t)+
'\n\t\t\t</span>\n\t\t</div>\n\t\t<div class="app-browser-header-divider"></div>\n\t\t<div class="app-browser-header-right">\n\t\t\t<p>\n\t\t\t\t'+
((__t=( t('tracker_browser_type_filter', _select({
					id: 'trackers-app-list-filter-type',
					options: [
						{ name: t("all"), value: 'all' },
						{ name: t("blocked"), value: 'blocked' },
						{ name: t("unblocked"), value: 'unblocked' },
						{ name: t("new_since_last_update"), value: 'new' }
					]
				})) ))==null?'':__t)+
'\n\t\t\t</p>\n\t\t\t<p>\n\t\t\t\t\t'+
((__t=( t('tracker_browser_name_filter', 
							'<input type="text" class="app-list-filter-name" id="trackers-app-list-filter-name" placeholder="'
							+ t("tracker_browser_name_filter_placeholder") + '">'
							+ '<span id="app-list-reset-search"></span>') ))==null?'':__t)+
'\n\t\t\t</p>\n\t\t\t<!-- <p>\n\t\t\t\t<a href="#" id="app-list-reset-search">'+
((__t=( t("tracker_browser_filters_reset") ))==null?'':_.escape(__t))+
'</a>\n\t\t\t</p> -->\n\t\t</div>\n\t\t<div style="clear:both"></div>\n\t</div>\n\n\t<div id="tag-list" class="small">\n\t</div>\n\n\t<div id="filter-desc">\n\t\t<div>\n\t\t\t<a href="#" id="filter-clear">'+
((__t=( t('tracker_browser_filters_reset') ))==null?'':_.escape(__t))+
'</a>\n\t\t\t<span id="filter-desc-text"></span>\n\t\t</div>\n\t</div>\n\n\t<div class="app-toggles-divs">\n\t\t<a href="#" id="select-all">'+
((__t=( t("toggle_select_all") ))==null?'':_.escape(__t))+
'</a>\n\t\t<span class="vr"></span>\n\t\t<a href="#" id="select-none">'+
((__t=( t("toggle_select_none") ))==null?'':_.escape(__t))+
'</a>\n\t\t<span class="vr"></span>\n\t\t<a href="#" id="expand-all">'+
((__t=( t("toggle_expand_all") ))==null?'':_.escape(__t))+
'</a>\n\t\t<span class="vr"></span>\n\t\t<a href="#" id="collapse-all">'+
((__t=( t("toggle_collapse_all") ))==null?'':_.escape(__t))+
'</a>\n\t</div>\n\n\t<div id="trackers">\n\t</div>\n\n\t<div id="no-results">\n\t\t'+
((__t=( t('no_results') ))==null?'':_.escape(__t))+
'\n\t</div>\n\n</div>\n';
}
return __p;
};
});