define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='';
 var catTags = [t('category_' + category)].concat(tags || []).join(", ") 
__p+='\n<div class="app-info-container" title="'+
((__t=( name + " (" + catTags + ")" ))==null?'':_.escape(__t))+
'">\n\t<div class="app-arrow ';
 if (expand_sources) print('down') 
__p+='"></div>\n\t<div class="app-text">\n\t\t<div style="display: table-cell; vertical-align: middle; max-width: 185px;">\n\t\t\t<div class="app-name ellipsis ';
 if (blocked) print('blocked') 
__p+='">'+
((__t=( name ))==null?'':_.escape(__t))+
'</div>\n\t\t\t<div class="app-type ellipsis">'+
((__t=( catTags ))==null?'':_.escape(__t))+
'</div>\n\t\t</div>\n\t</div>\n</div>\n<div class="blocking-controls no-select" title="">\n\t<div class="selective-unblock-control right">\n\t\t<div class="app-site-blocking '+
((__t=( siteSpecificUnblocked ? 'on' : 'off' ))==null?'':_.escape(__t))+
'"\n\t\t\ttitle="'+
((__t=( t('panel_tracker_site_specific_unblock_tooltip_' + (siteSpecificUnblocked ? 'on' : 'off'), name, page_host) ))==null?'':_.escape(__t))+
'"></div>\n\t</div>\n\t<div class="global-blocking-control right ';
 print(globalBlocked ? 'blocked' : 'unblocked') 
__p+='">\n\t\t<div class="app-global-blocking ';
 print(globalBlocked ? 'blocked' : 'unblocked') 
__p+=' ';
 if (whitelisted) print('whitelisted') 
__p+=' ';
 if (pauseBlocking) print('paused') 
__p+='" title="'+
((__t=( t('panel_tracker_global_block_tooltip', name) ))==null?'':_.escape(__t))+
'"></div>\n\t</div>\n\t<div class="clear"></div>\n</div>\n';
 if (hasCompatibilityIssue) { 
__p+='\n<div class="tracker-alert right" title="'+
((__t=( t('panel_tracker_compatibility_warning', page_host) ))==null?'':_.escape(__t))+
'">\n</div>\n';
 } 
__p+='\n<div class="clear"></div>\n<div class="app-moreinfo ellipsis" ';
 if (!expand_sources) print('style="display:none"') 
__p+='>\n\t<a class="app-moreinfo-link ellipsis" target="_blank"\n\t\thref="https://www.ghostery.com/apps/'+
((__t=( encodeURIComponent(name.replace(/\s+/g, '_').toLowerCase()) ))==null?'':_.escape(__t))+
'">\n\t\t'+
((__t=( t('panel_tracker_more_info', name) ))==null?'':_.escape(__t))+
'\n\t</a>\n</div>\n<div class="app-srcs-container" ';
 if (!expand_sources) print('style="display:none"') 
__p+='>\n\t<div style="margin-bottom:0px" class="app-srcs-title ellipsis">'+
((__t=( t('panel_tracker_found_sources_title') ))==null?'':_.escape(__t))+
'</div>\n\t<div class="app-srcs">\n\n\t';
 sources.forEach(function (source) { 
__p+='\n\t\t<div class="app-src">\n\t\t\t<div class="clip">\n\t\t\t\t<div class="message">'+
((__t=( t('copied_to_clipboard') ))==null?'':_.escape(__t))+
'</div>\n\t\t\t\t<div class="message-arrow bottom"></div>\n\t\t\t</div>\n\t\t\t<!--\n\t\t\t<div class="zero-clip" title="';
/*- source.src*/ 
__p+='"></div>\n\t\t\t-->\n\t\t\t<div class="ellipsis">\n\t\t\t\t<a target="_blank" class="app-src-link';
 if (source.blocked) print(' blocked') 
__p+='"\n\t\t\t\t\thref="https://www.ghostery.com/gcache/?n='+
((__t=( encodeURIComponent(name) ))==null?'':_.escape(__t))+
'&s='+
((__t=( encodeURIComponent(source.src) ))==null?'':_.escape(__t))+
'&v=2&t='+
((__t=( source.type ))==null?'':_.escape(__t))+
'"\n\t\t\t\t\ttitle="'+
((__t=( source.src ))==null?'':_.escape(__t))+
'">\n\t\t\t\t\t'+
((__t=( source.src ))==null?'':_.escape(__t))+
'\n\t\t\t\t</a>\n\t\t\t</div>\n\t\t</div>\n\t';
 }) 
__p+='\n\n\t</div>\n</div>\n';
}
return __p;
};
});