define(function(require){
var t=require("lib/i18n").t;
return function(obj){
var __t,__p='',__j=Array.prototype.join,print=function(){__p+=__j.call(arguments,'');};
with(obj||{}){
__p+='<div id="block-default-message-container">\r\n\t<div class="ellipsis" id="block-default-message">\r\n\t\t'+
((__t=( t("block_by_default_helper_text") ))==null?'':_.escape(__t))+
'\r\n\t\t<span>\r\n\t\t\t<a href="#" id="block-default-close">'+
((__t=( t("block_by_default_helper_accept") ))==null?'':_.escape(__t))+
'</a>\r\n\t\t\t<a href="#" id="block-default-disable">'+
((__t=( t("block_by_default_helper_cancel") ))==null?'':_.escape(__t))+
'</a>\r\n\t\t</span>\r\n\t\t<!-- <span id=\'block-default-msg-close\'></span> -->\r\n\t</div>\r\n</div>';
}
return __p;
};
});