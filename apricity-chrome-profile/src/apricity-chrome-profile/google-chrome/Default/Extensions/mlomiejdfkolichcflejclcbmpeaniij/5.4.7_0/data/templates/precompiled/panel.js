define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<div id="tooltip" class="message"></div>\n<div id="close-panel"></div>\n<div id="header">\n\t<div id="header-icon"></div>\n\t<div id="header-text">\n\t\t<div id="ghostery-findings">\n\t\t\t<div id="ghostery-findings-text" class="ellipsis">\n\t\t\t\t'+
((__t=( t('panel_title_not_scanned') ))==null?'':_.escape(__t))+
'\n\t\t\t</div>\n\t\t\t<div id="website-info">\n\t\t\t\t<div id="website-url" class="ellipsis"></div>\n\t\t\t</div>\n\t\t</div>\n\t</div>\n\t<div class="tooltip" id="settings-button" title="'+
((__t=( t('panel_tooltip_settings') ))==null?'':_.escape(__t))+
'">\n\t</div>\n</div>\n\n<div id="settings" class="no-select">\n\t<div class="settings-buttons" id="options-button">'+
((__t=( t('panel_settings_options') ))==null?'':_.escape(__t))+
'</div>\n\t<div class="settings-buttons" id="support-button">'+
((__t=( t('panel_settings_support') ))==null?'':_.escape(__t))+
'</div>\n\t<div class="settings-buttons" id="about-button">'+
((__t=( t('panel_settings_about') ))==null?'':_.escape(__t))+
'</div>\n\t<div class="settings-buttons" id="share-button">'+
((__t=( t('panel_settings_share') ))==null?'':_.escape(__t))+
'</div>\n</div>\n\n<div id="apps-div">\n\t<div class="no-trackers">\n\t\t<div class="vertical-center">\n\t\t\t'+
((__t=( t('panel_not_scanned') ))==null?'':__t)+
'\n\t\t</div>\n\t</div>\n</div>\n\n<div id="reload">'+
((__t=( t('panel_needs_reload') ))==null?'':__t)+
'</div>\n<div id="whitelisted">'+
((__t=( t('panel_title_whitelisted',"", "") + "<br>" + t('panel_needs_reload') ))==null?'':__t)+
'</div>\n<div id="paused">'+
((__t=( t('panel_title_paused', "", "") + "<br>" + t('panel_needs_reload') ))==null?'':__t)+
'</div>\n<div id="paused-arrow"></div>\n<div id="whitelisted-arrow"></div>\n\n<div id="footer" class="no-select">\n\t<div unselectable="on" class="footer-button ellipsis left" id="pause-blocking-button" style="margin-left: 7px;" title="'+
((__t=( t('panel_button_pause_blocking_tooltip') ))==null?'':_.escape(__t))+
'">\n\t\t<div class="footer-button-desc">'+
((__t=( t('panel_button_pause_blocking') ))==null?'':_.escape(__t))+
'</div>\n\t</div>\n\t<div unselectable="on" class="footer-button ellipsis left disabled" id="whitelisting-button" title="'+
((__t=( t('panel_button_whitelist_site_tooltip') ))==null?'':_.escape(__t))+
'">\n\t\t<div class="footer-button-desc">'+
((__t=( t('panel_button_whitelist_site') ))==null?'':_.escape(__t))+
'</div>\n\t</div>\n\t<div class="footer-button left tooltip" id="help-button" title="'+
((__t=( t('panel_tooltip_help') ))==null?'':_.escape(__t))+
'"></div>\n</div>\n\n<div id="tutorial-container">\n\n\t<div class="tutorial-arrow" id="tutorial-arrow-left"></div>\n\t<div class="tutorial-arrow" id="tutorial-arrow-right"></div>\n\n\t<div id="tutorial-controls" class="no-select">\n\t\t<div class="tutorial-control" id="tutorial-control-1"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-2"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-3"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-4"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-5"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-6"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-7"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-8"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-9"></div>\n\t\t<div class="tutorial-control" id="tutorial-control-10"></div>\n\t</div>\n\n\t<div id="tutorial-close">'+
((__t=( t('panel_tutorial_close') ))==null?'':_.escape(__t))+
'</div>\n\n\t<div id="tutorial-screens">\n\n\t\t<div id="tutorial-screen-1" class="tutorial-screen table">\n\t\t\t\t<div class="valign">\n\t\t\t\t\t<div class="title-large">'+
((__t=( t('panel_tutorial_screen_welcome_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_welcome_content') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-2" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_findings_panel_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<div class="vertical-center">\n\t\t\t\t\t<img class="center-image" src="data/images/panel/tutorial/panel.png" style="height:100px; width:100px">\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_findings_panel_content') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t\t</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-3" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_blocking_controls_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<img class="center-image" src="data/images/panel/tutorial/blocking_controls.png">\n\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_blocking_controls_content') ))==null?'':__t)+
'</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-4" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_global_block_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<div class="vertical-center">\n\t\t\t\t\t<div class="blocking-control-image">\n\t\t\t\t\t\t<img class="left-image right" src="data/images/panel/tutorial/indicator_cross_red.png">\n\t\t\t\t\t</div>\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_global_block_content') ))==null?'':__t)+
'</div>\n\t\t\t\t</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-5" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_global_unblock_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<div class="vertical-center">\n\t\t\t\t\t<div class="blocking-control-image">\n\t\t\t\t\t\t<img class="left-image" src="data/images/panel/tutorial/indicator_cross_blue.png">\n\t\t\t\t\t</div>\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_global_unblock_content') ))==null?'':__t)+
'</div>\n\t\t\t\t</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-6" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_site_specific_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<div class="vertical-center">\n\t\t\t\t\t<div class="blocking-control-image right-only">\n\t\t\t\t\t\t<img class="right-image" src="data/images/panel/tutorial/indicator_check_green.png">\n\t\t\t\t\t</div>\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_site_specific_content') ))==null?'':__t)+
'</div>\n\t\t\t\t</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-7" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_pause_blocking_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<div class="vertical-center">\n\t\t\t\t\t<div class="blocking-control-image">\n\t\t\t\t\t\t<img class="left-image right" src="data/images/panel/tutorial/indicator_cross_yellow.png">\n\t\t\t\t\t</div>\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_pause_blocking_content', t('panel_button_pause_blocking')) ))==null?'':__t)+
'</div>\n\t\t\t\t</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-8" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_whitelist_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<div class="vertical-center">\n\t\t\t\t\t<div class="blocking-control-image">\n\t\t\t\t\t\t<img class="left-image right" src="data/images/panel/tutorial/indicator_cross_green.png">\n\t\t\t\t\t</div>\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_whitelist_content', t('panel_button_whitelist_site')) ))==null?'':__t)+
'</div>\n\t\t\t\t</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-9" class="tutorial-screen">\n\t\t\t<div class="title-small">'+
((__t=( t('panel_tutorial_screen_more_options_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="text-center">\n\t\t\t\t<div class="vertical-center">\n\t\t\t\t\t<img class="center-image" src="data/images/panel/tutorial/settings.png">\n\t\t\t\t\t<div class="text-large">'+
((__t=( t('panel_tutorial_screen_more_options_content', t('panel_settings_options')) ))==null?'':__t)+
'</div>\n\t\t\t\t</div>\n\t\t\t</div>\n\t\t</div>\n\n\t\t<div id="tutorial-screen-10" class="tutorial-screen table">\n\t\t\t<div class="valign">\n\t\t\t\t<div class="title-large">'+
((__t=( t('panel_tutorial_screen_final_title') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t\t<div class="text-small">'+
((__t=( t('panel_tutorial_screen_final_content1') ))==null?'':__t)+
'</div>\n\t\t\t\t<br>\n\t\t\t\t<div class="text-small">'+
((__t=( t('panel_tutorial_screen_final_content2') ))==null?'':__t)+
'</div>\n\t\t\t\t<br>\n\t\t\t\t<div class="text-small">'+
((__t=( t('panel_tutorial_screen_final_content3') ))==null?'':__t)+
'</div>\n\t\t\t</div>\n\t\t</div>\n\t</div>\n</div>\n';
}
return __p;
};
});