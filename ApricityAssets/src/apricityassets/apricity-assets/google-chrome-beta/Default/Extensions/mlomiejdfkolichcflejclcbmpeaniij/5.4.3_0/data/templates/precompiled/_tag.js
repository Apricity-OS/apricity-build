define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<div id="tag-'+
((__t=( id ))==null?'':_.escape(__t))+
'" class="tag-button'+
((__t=( selected ? ' selected' : '' ))==null?'':_.escape(__t))+
'" title="'+
((__t=( description ))==null?'':_.escape(__t))+
'">\n\t'+
((__t=( name ))==null?'':_.escape(__t))+
'\n\t<span class="tag-count">'+
((__t=( count ))==null?'':_.escape(__t))+
'</span>\n</div>\n';
}
return __p;
};
});