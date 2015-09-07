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
	show_tabs: true,
	show_walkthrough_link: true,
	show_walkthrough_progress: false,
	show_walkthrough_skip: false,
	survey_link: true
}) ))==null?'':__t)+
'\n\n<div class="options-div" id="general-options">\n\t<table>\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_sharing_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t'+
((__t=( _ghostrank({
					tm: true
				}) ))==null?'':__t)+
'\n\t\t\t\t<br />\n\t\t\t\t<input type="checkbox" id="ghostrank"';
 if (conf.ghostrank) print(' checked') 
__p+='>\n\t\t\t\t<label for="ghostrank">'+
((__t=( t("options_ghostrank") ))==null?'':_.escape(__t))+
'</label>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_autoupdate_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t\t'+
((__t=( t("walkthrough_autoupdate1") ))==null?'':_.escape(__t))+
'\n\t\t\t\t</p>\n\t\t\t\t<br />\n\t\t\t\t<input type="checkbox" id="enable_autoupdate"';
 if (conf.enable_autoupdate) print(' checked') 
__p+='>\n\t\t\t\t<label id="update" for="enable_autoupdate">'+
((__t=( t("options_autoupdate") ))==null?'':_.escape(__t))+
'</label>\n\t\t\t\t<br />\n\t\t\t\t<input type="checkbox" style="visibility:hidden">\n\t\t\t\t<span style="font-size:13px; padding-left:3px">\n\t\t\t\t\t<span id="apps-last-updated">\n\t\t\t\t\t\t'+
((__t=( t('library_never_updated') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</span>\n\t\t\t\t\t<span id="update-now-span">\n\t\t\t\t\t\t<a href="#" id="update-now-link" aria-label="Press to immediately update Ghostery tracker lists">'+
((__t=( t('library_update_now_link') ))==null?'':_.escape(__t))+
'</a>\n\t\t\t\t\t</span>\n\t\t\t\t</span>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_blocking_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t\t'+
((__t=( t("options_blocking1") ))==null?'':_.escape(__t))+
'\n\t\t\t\t</p>\n\t\t\t\t<p style="margin-bottom:25px">\n\t\t\t\t\t<em>'+
((__t=( t("note") ))==null?'':_.escape(__t))+
'</em>\n\t\t\t\t\t'+
((__t=( t("options_blocking2") ))==null?'':__t)+
'\n\t\t\t\t</p>\n\n\t\t\t\t<div id="tabs-apps-container">\n\t\t\t\t\t<ul class="tabs app-browser-tabs" id="tabs-apps" role="navigation">\n\t\t\t\t\t\t<li class="active" id="apps-tab" href="#apps" data-tab-contents-selector="#trackers-app-browser" aria-label="Tracker browser tab">\n\t\t\t\t\t\t\t'+
((__t=( t("options_trackers_tab") ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t</li>\n\t\t\t\t\t\t<li id="sites-tab" href="#sites" data-tab-contents-selector="#whitelist-div" aria-label="Blocking-exempt sites tab">\n\t\t\t\t\t\t\t'+
((__t=( t("options_sites_tab") ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t</li>\n\t\t\t\t\t</ul>\n\t\t\t\t</div>\n\t\t\t\t<div style="clear: both;"></div>\n\n\t\t\t\t'+
((__t=( _app_browser() ))==null?'':__t)+
'\n\t\t\t\t<table id="whitelist-div" class="app-browser" style="display:none;">\n\t\t\t\t\t<thead>\n\t\t\t\t\t\t<tr>\n\t\t\t\t\t\t\t<th colspan="2" style="font-weight: 100; padding: 35px 55px 0 55px; font-size: .75em;">\n\t\t\t\t\t\t\t\t<p>\n\t\t\t\t\t\t\t\t\t'+
((__t=( t("site_whitelist_description") ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t\t\t</p>\n\t\t\t\t\t\t\t</th>\n\t\t\t\t\t\t</tr>\n\t\t\t\t\t</thead>\n\t\t\t\t\t<tbody>\n\t\t\t\t\t\t<tr>\n\t\t\t\t\t\t\t<th style="font-weight: bold; padding: 35px 55px 0 55px; font-size: .875em;" colspan="2">\n\t\t\t\t\t\t\t\t'+
((__t=( t("site_whitelist_help") ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t\t</th>\n\t\t\t\t\t\t</tr>\n\t\t\t\t\t\t<tr>\n\t\t\t\t\t\t\t<td style="width: 72%; padding: 10px 0 0 55px; vertical-align: top; font-size: .875em;">\n\t\t\t\t\t\t\t\t<input type="text" id="whitelist-add-input" value="" autocomplete="off" placeholder="example.com">\n\t\t\t\t\t\t\t\t<div id="whitelist-error" style="display:none">\n\t\t\t\t\t\t\t\t\t<span id=\'whitelist-error-msg\'></span>\n\t\t\t\t\t\t\t\t\t<span id="whitelist-error-msg-close"></span>\n\t\t\t\t\t\t\t\t</div>\n\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t<td style="padding: 10px 55px 0 15px; vertical-align: top; font-size: .875em;">\n\t\t\t\t\t\t\t\t<button id="whitelist-add-button" class="blue-button">\n\t\t\t\t\t\t\t\t\t<span>'+
((__t=( t("whitelist_add_button") ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t\t\t\t\t</button>\n\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t</tr>\n\t\t\t\t\t\t<tr>\n\t\t\t\t\t\t\t<th style="font-weight: bold; padding: 35px 55px 0 55px; font-size: .875em;" colspan="2">\n\t\t\t\t\t\t\t\t'+
((__t=( t("whitelisted_sites_header") ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t\t</th>\n\t\t\t\t\t\t</tr>\n\t\t\t\t\t\t<tr>\n\t\t\t\t\t\t\t<td style="width: 72%; padding: 10px 0px 35px 55px; vertical-align: top; font-size: .875em;">\n\t\t\t\t\t\t\t\t<select multiple="multiple" id="whitelist"></select>\n\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t<td style="padding: 10px 55px 35px 15px; vertical-align: top; font-size: .875em;">\n\t\t\t\t\t\t\t\t<button id="whitelist-remove-button" class="blue-button">\n\t\t\t\t\t\t\t\t\t<span>'+
((__t=( t("whitelist_remove_button") ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t\t\t\t\t</button>\n\t\t\t\t\t\t\t\t<br />\n\t\t\t\t\t\t\t\t<button id="whitelist-remove-all-button" class="blue-button" style="margin-top:10px; clear: left;">\n\t\t\t\t\t\t\t\t\t<span>'+
((__t=( t("whitelist_remove_all_button") ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t\t\t\t\t</button>\n\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t</tr>\n\t\t\t\t\t</tbody>\n\t\t\t\t</table>\n\t\t\t</td>\n\t\t</tr>\n\t\t<tr>\n\t\t\t<th style="border-bottom: none;">&nbsp;</th>\n\t\t\t<td style="border-bottom: none; border-left: none; text-align: left;">\n\t\t\t\t<div id="buttons">\n\t\t\t\t\t<button class="save-button blue-button" disabled>\n\t\t\t\t\t\t<span>'+
((__t=( t("options_save_button") ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t\t</button>\n\t\t\t\t\t<button class="cancel-button blue-button" disabled>\n\t\t\t\t\t\t<span>'+
((__t=( t("options_cancel_button") ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t\t</button>\n\t\t\t\t</div>\n\t\t\t</td>\n\t\t</tr>\n\t\t<tr>\n\t\t\t<th style="border: none; padding-top: 0; padding-bottom: 0;">&nbsp;</th>\n\t\t\t<td style="border: none; padding-top: 0; padding-bottom: 0;">\n\t\t\t\t'+
((__t=( _footer() ))==null?'':__t)+
'\n\t\t\t</td>\n\t\t</tr>\n\t</table>\n</div>\n\n<div class="options-div" id="advanced-options" style="display:none">\n\t<table>\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_display_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<div id="alert-bubble-options">\n\t\t\t\t\t<p class="first">\n\t\t\t\t\t\t<input type="checkbox" id="show-alert"';
 if (conf.show_alert) print(' checked') 
__p+='>\n\t\t\t\t\t\t<label for="show-alert">\n\t\t\t\t\t\t\t'+
((__t=( t('options_alert_bubble_show', '<span id="alert-bubble-help" class="help">' + t('alert_bubble') + '</span>') ))==null?'':__t)+
'\n\t\t\t\t\t\t</label>\n\t\t\t\t\t</p>\n\t\t\t\t\t<div id="alert-bubble-sub-options"';
 if (!conf.show_alert) print (' style="display:none"') 
__p+='>\n\t\t\t\t\t\t<p class="suboption">\n\t\t\t\t\t\t\t'+
((__t=( t('options_alert_bubble_position', _select({
								id: 'alert-bubble-pos',
								options: [
									{ name: t("corner1"), value: 'tr' },
									{ name: t("corner2"), value: 'tl' },
									{ name: t("corner3"), value: 'br' },
									{ name: t("corner4"), value: 'bl' }
								],
								selected: conf.alert_bubble_pos
							})) ))==null?'':__t)+
'\n\t\t\t\t\t\t</p>\n\t\t\t\t\t\t<p class="suboption">\n\t\t\t\t\t\t\t'+
((__t=( t('options_alert_bubble_timeout', _select({
								id: "alert-bubble-timeout",
								options: [60, 30, 25, 20, 15, 10, 5, 3],
								selected: conf.alert_bubble_timeout
							})) ))==null?'':__t)+
'\n\t\t\t\t\t\t</p>\n\t\t\t\t\t</div>\n\t\t\t\t</div>\n\n\t\t\t\t<p class="not-first">\n\t\t\t\t\t<input type="checkbox" id="expand_sources"';
 if (conf.expand_sources) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="expand_sources">'+
((__t=( t('options_script_sources') ))==null?'':_.escape(__t))+
'</label>\n\t\t\t\t\t'+
((__t=( t('options_in_the_findings_panel') ))==null?'':__t)+
'\n\t\t\t\t</p>\n\n\t\t\t\t<p>\n\t\t\t\t\t<input type="checkbox" id="show-badge"';
 if (conf.show_badge) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="show-badge">'+
((__t=( t('options_badge') ))==null?'':__t)+
'</label>\n\t\t\t\t</p>\n\n\t\t\t\t<p>\n\t\t\t\t\t<input type="checkbox" id="show-cmp"';
 if (conf.show_cmp) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="show-cmp">'+
((__t=( t('options_cmp') ))==null?'':__t)+
'</label>\n\t\t\t\t</p>\n\t\t\t</td>\n\t\t</tr>\n\n \t\t<tr>\n \t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_blocking_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t\t<input type="checkbox" id="ignore-first-party"';
 if (conf.ignore_first_party) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="ignore-first-party">'+
((__t=( t("options_ignore_first_party") ))==null?'':_.escape(__t))+
'</label>\n\t\t\t\t\t<span id="ignore-first-party-help">\n\t\t\t\t\t\t'+
((__t=( t("options_ignore_first_party_help") ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</span>\n\t\t\t\t</p>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_autoupdate_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t\t<input type="checkbox" id="block-by-default"';
 if (conf.block_by_default) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="block-by-default">'+
((__t=( t("options_block_by_default") ))==null?'':_.escape(__t))+
'</label>\n\t\t\t\t</p>\n\t\t\t\t<p>\n\t\t\t\t\t<input type="checkbox" id="notify-library-updates"';
 if (conf.notify_library_updates) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="notify-library-updates">'+
((__t=( t("options_notify_of_library_updates") ))==null?'':_.escape(__t))+
'</label>\n\t\t\t\t</p>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_click2play_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t\t<input type="checkbox" id="click2play"';
 if (conf.enable_click2play) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="click2play">'+
((__t=( t("options_click2play1") ))==null?'':_.escape(__t))+
'</label>\n\t\t\t\t</p>\n\t\t\t\t<p>\n\t\t\t\t\t<input type="checkbox" style="visibility:hidden">\n\t\t\t\t\t<label><span id="click2play-help" class="help">'+
((__t=( t("options_click2play2")))==null?'':_.escape(__t))+
'</span></label>\n\t\t\t\t</p>\n\n\t\t\t\t<p class="suboption" id="show-c2p-buttons"';
 if (!conf.enable_click2play) print(' style="display:none"') 
__p+='>\n\t\t\t\t\t<input type="checkbox" id="click2play-buttons"';
 if (conf.enable_click2play_social) print(' checked') 
__p+='>\n\t\t\t\t\t<label for="click2play-buttons">'+
((__t=( t("options_click2play_buttons1") ))==null?'':_.escape(__t))+
'</label>\n\t\t\t\t\t<span id="click2play-buttons-help" class="help">\n\t\t\t\t\t\t'+
((__t=( t("options_click2play_buttons2") ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</span>\n\t\t\t\t</p>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("options_language_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t';

				// ensure that "Show Ghostery in LANGUAGE" is grammatical in that language
				languages[conf.language] = t('options_language_language');

				// convert to _select's format
				languages = _(languages)
					.chain()
					.reduce(function (memo, value, key) {
						memo.push({
							name: value,
							value: key
						});
						return memo;
					}, [])
					.sortBy(function (l) { return l.value; })
					.value();
				
__p+='\n\t\t\t\t'+
((__t=( t('options_language', _select({
					id: 'language',
					options: languages,
					selected: conf.language
				})) ))==null?'':__t)+
'\n\t\t\t\t</p>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t('options_backup_header') ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n        <p>\n          <a id="goto-backup" href="backup.html">'+
((__t=( t('options_backup') ))==null?'':_.escape(__t))+
'</a>\n        </p>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th style="border-bottom: none;">&nbsp;</th>\n\t\t\t<td style="border-bottom: none; border-left: none; text-align: left;">\n\t\t\t\t<div id="buttons">\n\t\t\t\t\t<button class="save-button blue-button" disabled>\n\t\t\t\t\t\t<span>'+
((__t=( t("options_save_button") ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t\t</button>\n\t\t\t\t\t<button class="cancel-button blue-button" disabled>\n\t\t\t\t\t\t<span>'+
((__t=( t("options_cancel_button") ))==null?'':_.escape(__t))+
'</span>\n\t\t\t\t\t</button>\n\t\t\t\t</div>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th style="border: none; padding-top: 0; padding-bottom: 0;">&nbsp;</th>\n\t\t\t<td style="border: none; padding-top: 0; padding-bottom: 0;">\n\t\t\t\t'+
((__t=( _footer() ))==null?'':__t)+
'\n\t\t\t</td>\n\t\t</tr>\n\t</table>\n</div>\n\n<div class="options-div" id="about-options" style="display:none">\n\t\t<h1 style="padding: 0; margin: 0;">'+
((__t=( t("help_version_text", "Chrome", ghostery_version) ))==null?'':_.escape(__t))+
'</h1>\n\t\t<p>\n\t\t\t'+
((__t=( t("short_description") ))==null?'':_.escape(__t))+
'\n\t\t</p>\n\t\t<p style="margin-bottom: 50px;">\n\t\t\t<a class="about-links" href="https://www.ghostery.com/'+
((__t=( conf.language ))==null?'':_.escape(__t))+
'/eula" target="_blank">'+
((__t=( t("license_link") ))==null?'':_.escape(__t))+
'</a>\n\t\t\t<span class="vr"></span>\n\t\t\t<a class="about-links" href="https://www.ghostery.com/'+
((__t=( conf.language ))==null?'':_.escape(__t))+
'/privacy-addon" target="_blank">'+
((__t=( t("privacy_policy_link") ))==null?'':_.escape(__t))+
'</a>\n\t\t\t<span class="vr"></span>\n\t\t\t<a class="about-links" href="https://www.ghostery.com" target="_blank">'+
((__t=( t("homepage_link") ))==null?'':_.escape(__t))+
'</a>\n\t\t</p>\n\t\t<div style="clear:both;"></div>\n\n\t<table>\n\t\t<tr>\n\t\t\t<th>\n\t\t\t\t'+
((__t=( t("help_help_header") ))==null?'':_.escape(__t))+
'\n\t\t\t</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t\t'+
((__t=( t("help_text1") ))==null?'':__t)+
'\n\t\t\t\t</p>\n\t\t\t\t<p>\n\t\t\t\t\t'+
((__t=( t("help_text2") ))==null?'':__t)+
'\n\t\t\t\t</p>\n\t\t\t</td>\n\t\t</tr>\n\n\t\t<tr>\n\t\t\t<th>'+
((__t=( t("help_credits_header") ))==null?'':_.escape(__t))+
'</th>\n\t\t\t<td>\n\t\t\t\t<p>\n\t\t\t\t\t'+
((__t=( t("credits_description") ))==null?'':_.escape(__t))+
'\n\t\t\t\t</p>\n\t\t\t\t<ul id="code-libraries">\n\t\t\t\t\t';
 _(libraries)
						.chain()
						.sortBy(function (l) { return l.name.toLowerCase() })
						.each(function (library, i) {
							library.id = i 
__p+='\n\t\t\t\t\t\t\t'+
((__t=( _library_li(library, { variable: 'library' }) ))==null?'':__t)+
'\n\t\t\t\t\t\t';
 }) 
__p+='\n\t\t\t\t</ul>\n\t\t\t</td>\n\t\t</tr>\n\t\t<tr>\n\t\t\t<th style="border: none; padding-top: 0; padding-bottom: 0;">&nbsp;</th>\n\t\t\t<td style="border: none; padding-top: 0; padding-bottom: 0;">\n\t\t\t\t'+
((__t=( _footer() ))==null?'':__t)+
'\n\t\t\t</td>\n\t\t</tr>\n\t</table>\n</div>\n\n<div id="saving-options-notice-overlay"></div>\n<div id="saving-options-notice">\n\t<div>'+
((__t=( t("options_saving_exit_message") ))==null?'':_.escape(__t))+
'</div>\n</div>\n\n'+
((__t=( _block_by_default_helper() ))==null?'':__t)+
'\n';
}
return __p;
};
});