define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<select id="'+
((__t=( id ))==null?'':_.escape(__t))+
'">\n\t';
 _.each(options, function (option) { 
__p+='\n\t\t';
 if (option.hasOwnProperty('value')) { 
__p+='\n\t\t\t<option value="'+
((__t=( option.value ))==null?'':_.escape(__t))+
'"';
 if (typeof selected != 'undefined' && option.value === selected) print(' selected') 
__p+='>\n\t\t\t\t'+
((__t=( option.name ))==null?'':_.escape(__t))+
'\n\t\t\t</option>\n\t\t';
 } else { 
__p+='\n\t\t\t<option';
 if (typeof selected != 'undefined' && option === selected) print(' selected') 
__p+='>\n\t\t\t\t'+
((__t=( option ))==null?'':_.escape(__t))+
'\n\t\t\t</option>\n\t\t';
 } 
__p+='\n\t';
 }) 
__p+='\n</select>\n';
}
return __p;
};
});