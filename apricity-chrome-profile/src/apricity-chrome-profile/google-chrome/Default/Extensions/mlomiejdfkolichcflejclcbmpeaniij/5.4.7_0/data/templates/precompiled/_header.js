define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<div id="header">\n\n\t';
 if (show_walkthrough_progress) { 
__p+='\n\t\t<div id="walkthrough-progress">\n\t\t\t<span class="circle active"></span>\n\t\t\t<span class="circle"></span>\n\t\t\t<span class="circle"></span>\n\t\t\t<span class="circle"></span>\n\t\t\t<span class="circle"></span>\n\t\t\t<div style="clear:both"></div>\n\t\t</div>\n\t';
 } 
__p+='\n\n\t\t<div id="header-top">\n\t\t\t<img id="ghostery-about" src="data/images/ghostery_about.png" alt="'+
((__t=( t('ghostery_logo') ))==null?'':_.escape(__t))+
'">\n\t\t\t<div id="header-top-text">\n\n\t\t\t\t';
 if (typeof ratelink_url != 'undefined') { 
__p+='\n\t\t\t\t\t<div id="header-rate-survey">\n\t\t\t\t\t\t'+
((__t=( t('rate_ghostery_link1') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t<a href="'+
((__t=( ratelink_url ))==null?'':_.escape(__t))+
'">\n\t\t\t\t\t\t\t'+
((__t=( t('rate_ghostery_link2') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t</a>\n\t\t\t\t\t\t'+
((__t=( t('survey_link_also', 'https://www.ghostery.com/survey/in-app') ))==null?'':__t)+
'\n\t\t\t\t\t</div>\n\t\t\t\t';
 } else if (typeof survey_link != 'undefined') { 
__p+='\n\t\t\t\t\t<div id="header-rate-survey">\n\t\t\t\t\t\t'+
((__t=( t('survey_link', 'https://www.ghostery.com/survey/in-app') ))==null?'':__t)+
'\n\t\t\t\t\t</div>\n\t\t\t\t';
 } 
__p+='\n\n\t\t\t\t';
 if (show_walkthrough_skip) { 
__p+='\n\t\t\t\t<button id="skip-button" class="blue-button"><span>\n\t\t\t\t\t'+
((__t=( t("walkthrough_skip_button") ))==null?'':_.escape(__t))+
'\n\t\t\t\t</span></button>\n\t\t\t\t';
 } 
__p+='\n\n\t\t\t</div>\n\t\t</div>\n\t\t<div id="header-bottom">\n\t\t\t<h1 id="header-title" style="float: left; ">'+
((__t=( t('options_header') ))==null?'':_.escape(__t))+
'</h1>\n\t\t\t';
 if (show_walkthrough_link) { 
__p+='\n\t\t\t\t<div style="max-width: 320px; position: absolute; padding: 15px 20px; background-color: #078ED6; border-radius: 10px; font-size: .9em; margin-top: 10px; color: #fff; top: 20px; right: 50px; text-align: center;">\n\t\t\t\t\t'+
((__t=( t('walkthrough_link') ))==null?'':__t)+
'\n\t\t\t\t\t<!-- For a walkthrough of Ghostery\'s key options,<br />\n\t\t\t\t\ttry the <a href="walkthrough.html" id="walkthrough-link">Ghostery Configuration Wizard</a> -->\n\t\t\t\t</div>\n\t\t\t';
 } 
__p+='\n\t\t\t<div style="clear: both;"></div>\n\t\t\t';
 if (typeof show_tabs != 'undefined' && show_tabs) { 
__p+='\n\t\t\t\t<ul class="tabs" role="navigation">\n\t\t\t\t\t<li class="active" id="general-tab" href="#general" data-tab-contents-selector="#general-options" aria-label="general options section">\n\t\t\t\t\t\t'+
((__t=( t('options_general_tab') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</li>\n\t\t\t\t\t<li id="advanced-tab" href="#advanced" data-tab-contents-selector="#advanced-options" aria-label="advanced options section">\n\t\t\t\t\t\t'+
((__t=( t('options_advanced_tab') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</li>\n\t\t\t\t\t<li id="about-tab" href="#about" data-tab-contents-selector="#about-options" aria-label="about ghostery section">\n\t\t\t\t\t\t'+
((__t=( t('options_about_tab') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</li>\n\t\t\t\t</ul>\n\t\t\t';
 } 
__p+='\n\t\t</div>\n\n</div>\n';
}
return __p;
};
});