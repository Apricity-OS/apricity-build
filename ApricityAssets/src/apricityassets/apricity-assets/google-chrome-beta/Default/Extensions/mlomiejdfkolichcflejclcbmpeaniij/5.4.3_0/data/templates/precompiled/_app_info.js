define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<div class="app-info';
 if (loading) print(' loading') 
__p+='">\n\t<div ';
 if (hidden) print(' style="display:none"') 
__p+='>\n\n\t';
 if (!loading) { 
__p+='\n\n\t\t\t';
 if (success) { 
__p+='\n\n\t\t\t\t<img class="company-logo" src="'+
((__t=( company_logo_url ))==null?'':_.escape(__t))+
'">\n\n\t\t\t\t<h1>'+
((__t=( t('company_about', company_name) ))==null?'':_.escape(__t))+
'</h1>\n\n\t\t\t\t';
 if (company_app.length && !(company_app.length == 1 && company_name == name)) { 
__p+='\n\t\t\t\t\t<p>\n\t\t\t\t\t\t'+
((__t=( t('company_operates', company_name) ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t\t'+
((__t=( _.pluck(company_app, 'ca_name').join(', ') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</p>\n\t\t\t\t';
 } 
__p+='\n\n\t\t\t\t';
 if (company_in_their_own_words.length > 0) { 
__p+='\n\n\t\t\t\t\t<p>\n\t\t\t\t\t\t<h2>'+
((__t=( t('company_in_their_own_words') ))==null?'':_.escape(__t))+
'</h2>\n\t\t\t\t\t</p>\n\n\t\t\t\t\t<p>\n\t\t\t\t\t\t'+
((__t=( company_in_their_own_words ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</p>\n\n\t\t\t\t';
 } else if (company_description) { 
__p+='\n\n\t\t\t\t\t<p>\n\t\t\t\t\t\t<h2>'+
((__t=( t('company_description') ))==null?'':_.escape(__t))+
'</h2>\n\t\t\t\t\t</p>\n\n\t\t\t\t\t<p>\n\t\t\t\t\t\t'+
((__t=( company_description ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</p>\n\n\t\t\t\t';
 } 
__p+='\n\t\t\t\t<p>\n\t\t\t\t\t<h2>'+
((__t=( t('company_website') ))==null?'':_.escape(__t))+
'</h2>\n\t\t\t\t\t<a href="'+
((__t=( company_website_url ))==null?'':_.escape(__t))+
'" rel="nofollow" target="_blank">'+
((__t=( company_website_url ))==null?'':_.escape(__t))+
'</a>\n\t\t\t\t</p>\n\n\t\t\t\t<p>\n\t\t\t\t\t<h2>'+
((__t=( t('company_affiliations') ))==null?'':_.escape(__t))+
'</h2>\n\t\t\t\t\t';
 if (!affiliation_groups.length) { 
__p+='\n\t\t\t\t\t\t'+
((__t=( t('none') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t';
 } else {
						_(affiliation_groups).chain().pluck('ag_logo_url').each(function (logo_url) {
							
__p+='<img src="'+
((__t=( logo_url ))==null?'':_.escape(__t))+
'" height=20> ';

						})
					} 
__p+='\n\t\t\t\t</p>\n\n\t\t\t\t<p>\n\t\t\t\t\t<h2>'+
((__t=( t('company_tags') ))==null?'':_.escape(__t))+
'</h2>\n\t\t\t\t\t<span>\n\t\t\t\t\t\t'+
((__t=( tags.length > 0 ? tags.map(function (tag) { return tag.tag_title }).join(", ") : t('none') ))==null?'':_.escape(__t))+
'\n\t\t\t\t\t</span>\n\t\t\t\t</p>\n\n\t\t\t';
 } else { 
__p+='\n\n\t\t\t\t<h1>'+
((__t=( t('company_profile_error') ))==null?'':_.escape(__t))+
'</h1>\n\t\t\t\t<br>\n\n\t\t\t';
 } 
__p+='\n\n\t\t\t<h2>\n\t\t\t\t<a target="_blank" href="https://www.ghostery.com/apps/'+
((__t=( encodeURIComponent(name.replace(/\s+/g, '_').toLowerCase()) ))==null?'':_.escape(__t))+
'">'+
((__t=( t('company_profile_link') ))==null?'':_.escape(__t))+
'</a>\n\t\t\t</h2>\n\n\t';
 } 
__p+='\n\n\t</div>\n</div>\n';
}
return __p;
};
});