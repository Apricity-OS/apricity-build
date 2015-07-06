define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<p>\n\t'+
((__t=( t("options_sharing_notice") ))==null?'':__t)+
'\n</p>\n\n<p>\n\t'+
((__t=( t("options_sharing1", (typeof tm != 'undefined' ? '™' : '')) ))==null?'':__t)+
'\n</p>\n\n<div id="ghostrank-moreinfo" style="display:none">\n\t<p>\n\t\t'+
((__t=( t("options_sharing2") ))==null?'':__t)+
'\n\t</p>\n\t<p>\n\t\t'+
((__t=( t("options_sharing3") ))==null?'':__t)+
'\n\t</p>\n\t<p>\n\t\t'+
((__t=( t("options_sharing4",
    "<a href=\"http://www.aboutads.info/\" target=\"_blank\">",
    "<a href=\"https://purplebox.ghostery.com/post/1016021311\" target=\"_blank\">",
    "<a href=\"https://purplebox.ghostery.com/post/1016023393\" target=\"_blank\">",
    "<a href=\"https://wiki.digitalmethods.net/Dmi/DmiWinterSchool2012TrackingTheTrackers\" target=\"_blank\">",
    "<a href=\"https://purplebox.ghostery.com/post/1016023378\" target='_blank'>") ))==null?'':__t)+
'\n\t</p>\n\t<p>\n\t\t'+
((__t=( t("options_sharing5", "<a href=\"https://purplebox.ghostery.com/post/1016023438\" target=\"_blank\">") ))==null?'':__t)+
'\n\t</p>\n</div>\n\n<p style="margin-top:0">\n\t... <a href="#" id="ghostrank-moreinfo-link" aria-labelledby="ghostrank-moreinfo">'+
((__t=( t("see_more") ))==null?'':_.escape(__t))+
'</a>\n</p>\n';
}
return __p;
};
});