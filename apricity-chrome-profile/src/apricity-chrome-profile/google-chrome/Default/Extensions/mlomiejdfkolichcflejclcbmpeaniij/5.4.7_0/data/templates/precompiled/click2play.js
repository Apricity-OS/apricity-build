define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<!doctype html>\n<html>\n<head>\n\t<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">\n\t<style>\n\t\tbody {\n\t\t\tmargin: 0;\n\t\t\tpadding: 0;\n\t\t}\n\t\tp {\n\t\t\tmargin: 3px;\n\t\t\tfont-family: Helvetica, Arial, sans-serif;\n\t\t\tfont-size: 13px;\n\t\t}\n\t\ttable {\n\t\t\tborder-spacing: 0;\n\t\t\twidth: 100%;\n\t\t\theight: 100%;\n\t\t\ttext-align: center;\n\t\t\tvertical-align: middle;\n\t\t}\n\t\ttd {\n\t\t\tpadding: 0;\n\t\t}\n\t</style>\n</head>\n<body>\n\t<table><tr><td>\n\n\t\t';
 if (button) { 
__p+='\n\n\t\t\t<a id="action-once" href="#" onclick="return false">\n\t\t\t\t<img src="'+
((__t=( allow_once_src ))==null?'':_.escape(__t))+
'" title="'+
((__t=( allow_once_title ))==null?'':_.escape(__t))+
'">\n\t\t\t</a>\n\n\t\t';
 } else { 
__p+='\n\n\t\t\t';
 if (typeof click2play_text != "undefined" && click2play_text) { 
__p+='<p id="text">'+
((__t=( click2play_text ))==null?'':_.escape(__t))+
'</p>';
 } 
__p+='\n\n\t\t\t<img id="ghostery-blocked" src="'+
((__t=( ghostery_blocked_src ))==null?'':_.escape(__t))+
'" title="'+
((__t=( ghostery_blocked_title ))==null?'':_.escape(__t))+
'">\n\n\t\t\t<a id="action-once" href="#" onclick="return false"><img src="'+
((__t=( allow_once_src ))==null?'':_.escape(__t))+
'" title="'+
((__t=( allow_once_title ))==null?'':_.escape(__t))+
'"></a>\n\n\t\t\t<a id="action-always" href="#" onclick="return false"><img src="'+
((__t=( allow_always_src ))==null?'':_.escape(__t))+
'" title="'+
((__t=( t('click2play_allow_always_tooltip') ))==null?'':_.escape(__t))+
'"></a>\n\n\t\t';
 } 
__p+='\n\n\t</td></tr></table>\n</body>\n</html>\n';
}
return __p;
};
});