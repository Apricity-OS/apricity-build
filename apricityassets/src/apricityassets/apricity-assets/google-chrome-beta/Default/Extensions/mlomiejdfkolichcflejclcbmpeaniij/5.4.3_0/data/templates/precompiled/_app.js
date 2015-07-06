define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<div class="app-arrow">&nbsp;</div>\n<div class="app-checkbox">\n\t<input class="'+
((__t=( name ))==null?'':_.escape(__t))+
'" type="checkbox" ';
 if (selected) print(' checked') 
__p+='>\n</div>\n<div class="app-name">\n\t<a href="https://www.ghostery.com/apps/'+
((__t=( encodeURIComponent(name.replace(/\s+/g, '_').toLowerCase()) ))==null?'':_.escape(__t))+
'">'+
((__t=( name))==null?'':_.escape(__t))+
'</a>\n</div>\n<div class="app-tags">\n\t';
 tags.each(function (tag) { 
__p+='\n\t\t';
 if (tag === undefined) return 
__p+='\n\t\t<a href="#" data-id="'+
((__t=( tag.get('id')))==null?'':_.escape(__t))+
'">'+
((__t=( tag.get('name') ))==null?'':_.escape(__t))+
'</a><span class="separator"> | </span>\n\t';
 }) 
__p+='\n</div>\n';
}
return __p;
};
});