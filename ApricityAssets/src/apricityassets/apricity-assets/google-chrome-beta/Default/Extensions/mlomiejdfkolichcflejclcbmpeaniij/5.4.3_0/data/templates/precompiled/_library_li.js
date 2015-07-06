define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<li>\n\t<div class="license-line">\n\t\t<a href="'+
((__t=( url ))==null?'':_.escape(__t))+
'" target="_blank">'+
((__t=( name ))==null?'':_.escape(__t))+
'</a>\n\t\t<a href="'+
((__t=( license_url ))==null?'':_.escape(__t))+
'" class="license-link" id="license-link-'+
((__t=( id ))==null?'':_.escape(__t))+
'">'+
((__t=( t("credits_show_license_link") ))==null?'':_.escape(__t))+
'</a>\n\t</div>\n\t<div class="license-text" id="license-text-'+
((__t=( id ))==null?'':_.escape(__t))+
'">\n\t\t<a href="'+
((__t=( license_url ))==null?'':_.escape(__t))+
'" target="_blank">'+
((__t=( license_url ))==null?'':_.escape(__t))+
'</a>\n\t\t<pre>'+
((__t=( license_text ))==null?'':_.escape(__t))+
'</pre>\n\t</div>\n</li>\n';
}
return __p;
};
});