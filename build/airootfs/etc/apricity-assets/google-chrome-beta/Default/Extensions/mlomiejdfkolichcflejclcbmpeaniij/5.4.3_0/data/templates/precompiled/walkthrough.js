define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='';

var _header = require('tpl/_header'),
	_app_browser = require('tpl/_app_browser'),
	_ghostrank = require('tpl/_ghostrank'),
	_select = require('tpl/_select'),
	_footer = require('tpl/_footer'),
	_block_by_default_helper = require('tpl/_default_block_all');

__p+='\n\n'+
((__t=( _header({
	show_tabs: false,
	show_walkthrough_link: false,
	show_walkthrough_progress: true,
	show_walkthrough_skip: true
}) ))==null?'':__t)+
'\n\n<a href="#" id="arrow-prev" class="arrow" tabindex="1" role="navigation" aria-label="previous section" style="display:none"></a>\n<a href="#" id="arrow-next" class="arrow" tabindex="1" role="navigation" aria-label="next section"></a>\n\n<div id="slider">\n\n\t<div style="display:block" class="options-div">\n\t\t<p style="font-weight: bold;">\n\t\t\t'+
((__t=( t('welcome_to_ghostery', 'Chrome', '<span id="version-text"></span>') ))==null?'':__t)+
'\n\t\t</p>\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_intro1') ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_intro2') ))==null?'':__t)+
'\n\t\t</p>\n\t</div>\n\n\t<div style="display:none" class="options-div">\n\t\t'+
((__t=( _ghostrank() ))==null?'':__t)+
'\n\t\t<p>\n\t\t\t<label>\n\t\t\t\t<input type="checkbox" id="ghostrank"';
 if (conf.ghostrank) print(' checked') 
__p+='>\n\t\t\t\t'+
((__t=( t('walkthrough_ghostrank_checkbox') ))==null?'':_.escape(__t))+
'\n\t\t\t</label>\n\t\t</p>\n\t</div>\n\n\t<div style="display:none" class="options-div">\n\t\t<p>\n\t\t\t<img src="data/images/help/alert_bubble.png" class="example">\n\t\t\t'+
((__t=( t('walkthrough_notification1') ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_notification2') ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\n\t\t<p>\n\t\t\t<label>\n\t\t\t\t<input type="checkbox" id="show-alert"';
 if (conf.show_alert) print(' checked') 
__p+='>\n\t\t\t\t'+
((__t=( t('walkthrough_notification_checkbox') ))==null?'':_.escape(__t))+
'\n\t\t\t</label>\n\t\t</p>\n\n\t\t<div style="clear:both"></div>\n\t</div>\n\n\t<div style="display:none" class="options-div">\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_blocking1') ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_blocking2') ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_blocking3') ))==null?'':__t)+
'\n\t\t</p>\n\t\t<p style="margin-bottom: 20px;">\n\t\t\t'+
((__t=( t('walkthrough_blocking4') ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\n\t\t<div id="app-browser">\n\t\t\t'+
((__t=( _app_browser({ _select: _select }) ))==null?'':__t)+
'\n\t\t</div>\n\n\t\t<input type="checkbox" id="block-by-default" style="display:none"';
 if (conf.block_by_default) print(' checked') 
__p+='>\n\n\t</div>\n\n\t<div style="display:none" class="options-div">\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_finished2') ))==null?'':__t)+
'\n\t\t</p>\n\t\t<p>\n\t\t\t'+
((__t=( t('walkthrough_finished3') ))==null?'':__t)+
'\n\t\t</p>\n\t\t<p>\n\t\t\t'+
((__t=( t('thanks_for_using_ghostery') ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\t</div>\n\n</div>\n\n<div class="options-div">\n\t'+
((__t=( _footer() ))==null?'':__t)+
'\n</div>\n\n'+
((__t=( _block_by_default_helper() ))==null?'':__t)+
'\n';
}
return __p;
};
});