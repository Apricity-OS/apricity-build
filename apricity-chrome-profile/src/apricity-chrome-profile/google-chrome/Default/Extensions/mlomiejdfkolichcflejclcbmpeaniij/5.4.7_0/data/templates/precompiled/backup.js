define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='';

var _app_browser = require('tpl/_app_browser'),
	_ghostrank = require('tpl/_ghostrank'),
	_select = require('tpl/_select'),
	_header = require('tpl/_header'),
	_footer = require('tpl/_footer'),
	_library_li = require('tpl/_library_li'),
	_block_by_default_helper = require('tpl/_default_block_all');

__p+='\n\n'+
((__t=( _header({
	ratelink_url: 'https://chrome.google.com/webstore/detail/mlomiejdfkolichcflejclcbmpeaniij/reviews',
	show_tabs: false,
	show_walkthrough_link: false,
	show_walkthrough_progress: false,
	show_walkthrough_skip: false,
	survey_link: true
}) ))==null?'':__t)+
'\n\n<div class="options-div" id="general-options">\n\t<table>\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t('backup_export_header') ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n        <p>\n          '+
((__t=( t('backup_export') ))==null?'':_.escape(__t))+
'\n        </p>\n        <br />\n\t\t\t\t<button id="backup-button" class="blue-button" disabled>\n\t\t\t\t\t<span>'+
((__t=( t('backup_export_button') ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t</button>\n\t\t\t</td>\n    </tr>\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t('backup_import_header') ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n        <p>\n          '+
((__t=( t('backup_import') ))==null?'':_.escape(__t))+
'\n        </p>\n        <p>\n          '+
((__t=( t('backup_import_warning') ))==null?'':_.escape(__t))+
'\n        </p>\n        <br />\n\t\t\t\t<input type="file" id="restore-file"></input>\n        <br />\n\t\t\t\t<button id="restore-button" class="blue-button" disabled>\n\t\t\t\t\t<span>'+
((__t=( t('backup_import_button') ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t</button>\n        <div id="restore-error" style="display: none">\n          '+
((__t=( t('backup_import_error') ))==null?'':__t)+
'\n        </div>\n\t\t\t</td>\n\t\t</tr>\n\t\t<tr>\n\t\t\t<th style="border-bottom: none;">&nbsp;</th>\n\t\t\t<td style="border-bottom: none; border-left: none; text-align: left;">\n\t\t\t\t<a id="goto-options" href="options.html">'+
((__t=( t('backup_options_link') ))==null?'':_.escape(__t))+
'</a>\n\t\t\t</td>\n\t\t</tr>\n\t</table>\n</div>\n\n<div id="saving-options-notice-overlay"></div>\n<div id="saving-options-notice">\n\t<div>'+
((__t=( t("options_saving_exit_message") ))==null?'':_.escape(__t))+
'</div>\n</div>\n\n'+
((__t=( _block_by_default_helper() ))==null?'':__t)+
'\n';
}
return __p;
};
});