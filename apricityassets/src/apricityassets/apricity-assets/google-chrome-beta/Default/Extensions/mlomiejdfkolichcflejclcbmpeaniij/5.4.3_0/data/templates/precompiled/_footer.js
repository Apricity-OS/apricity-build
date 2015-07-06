define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<div id="footer" role="contentinfo" style="border: none;">\n\t'+
((__t=( t("copyright", "GHOSTERY, Inc., 10 East 39<sup>th</sup> Street, 8<sup>th</sup> Floor, New York, NY 10016") ))==null?'':__t)+
'\n</div>\n';
}
return __p;
};
});