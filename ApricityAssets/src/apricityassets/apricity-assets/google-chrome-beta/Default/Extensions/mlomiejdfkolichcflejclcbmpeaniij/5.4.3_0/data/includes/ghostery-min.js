/*!
 * Ghostery for Chrome
 * http://www.ghostery.com/
 *
 * Copyright 2014 Ghostery, Inc. All rights reserved.
 * See https://www.ghostery.com/eula for license.
 */
(function(){function e(){for(var e="";32>e.length;)e+=Math.random().toString(36).replace(/[^A-Za-z]/g,"");return e}function t(e){return E.createElement(e)}function n(){return t("br")}function o(e){for(var t=1;arguments.length>t;t++)e.appendChild(arguments[t])}function r(e,n){var o=t("script"),r=L.top.document.documentElement;e?o.src=e:o.textContent=n,r.insertBefore(o,r.firstChild)}function a(){var e=t("style"),n=" !important;",r="padding:0;margin:0;font:13px Arial,Helvetica;text-transform:none;font-size: 100%;vertical-align:baseline;line-height:normal;color:#fff;position:static;";e.innerHTML="@-webkit-keyframes pop"+g+" {"+"50% {"+"-webkit-transform:scale(1.2);"+"}"+"100% {"+"-webkit-transform:scale(1);"+"}"+"}"+"@keyframes pop"+g+" {"+"50% {"+"-webkit-transform:scale(1.2);"+"transform:scale(1.2);"+"}"+"100% {"+"-webkit-transform:scale(1);"+"transform:scale(1);"+"}"+"}"+"#"+g+"{"+r+"border:solid 2px #fff"+n+"box-sizing:content-box"+n+"color:#fff"+n+"display:block"+n+"height:auto"+n+"margin:0"+n+"opacity:0.9"+n+"padding:7px 10px"+n+"position:fixed"+n+"visibility:visible"+n+"width:auto"+n+"z-index:2147483647"+n+"-webkit-border-radius:5px"+n+"-webkit-box-shadow:0px 0px 20px #000"+n+"-webkit-box-sizing:content-box"+n+"}"+"."+g+"-blocked{"+r+"color:#AAA"+n+"display:inline"+n+"text-decoration:line-through"+n+"}"+"#"+g+" br{display:block"+n+r+"}"+"#"+g+" span{background:transparent"+n+r+"}"+"#"+g+" div{"+r+"border:0"+n+"margin:0"+n+"padding:0"+n+"width:auto"+n+"letter-spacing:normal"+n+"font:13px Arial,Helvetica"+n+"text-align:left"+n+"text-shadow:none"+n+"text-transform:none"+n+"word-spacing:normal"+n+"}"+"#"+g+" a{"+r+"font-weight:normal"+n+"background:none"+n+"text-decoration:underline"+n+"color:#fff"+n+"}"+"a#"+g+"-gear{"+r+"text-decoration:none"+n+"position:absolute"+n+"display:none"+n+"font-size:20px"+n+"width:20px"+n+"height:20px"+n+"line-height:20px"+n+"text-align:center"+n+"background-color:rgba(255,255,255,.8)"+n+"background-image:url("+chrome.extension.getURL("data/images/gear.svg")+")"+n+"background-size:16px 16px"+n+"background-position:center center"+n+"background-repeat:no-repeat"+n+"text-decoration:none"+n+"}"+"a#"+g+"-gear:hover{"+"-webkit-animation-name:pop"+g+n+"animation-name:pop"+g+n+"-webkit-animation-duration:0.3s"+n+"animation-duration:0.3s"+n+"}"+"#"+g+":hover #"+g+"-gear{"+"text-decoration:none"+n+"display:inline-block"+n+"}"+"@media print{#"+g+"{display:none"+n+"}}",o(E.getElementsByTagName("head")[0],e)}function i(e){var t=E.getElementById(g);t&&t.parentNode.removeChild(t),clearTimeout(h),e&&(m=!0)}function s(e,n){var r=t("a");return r.style.color="#fff",r.style.textDecoration="underline",r.style.border="none",r.href=e||"#",e&&(r.target="_blank"),o(r,E.createTextNode(n)),r}function l(e,n){var r=t("span");return n&&(r.className=n),o(r,E.createTextNode(e)),r}function d(e,n){var r=t("div");return r.id=g,r.style.setProperty(n&&"left"==n.pos_x?"left":"right","20px","important"),r.style.setProperty(n&&"bottom"==n.pos_y?"bottom":"top","15px","important"),r.style.setProperty("background","showBugs"==e?"#330033":"#777","important"),E.getElementsByTagName("body")[0]?o(E.body,r):o(E.getElementsByTagName("html")[0],r),"showBugs"==e&&(r.style.cursor="pointer",r.addEventListener("click",function(e){i(!0),e.preventDefault()}),r.addEventListener("mouseenter",function(e){clearTimeout(h),h=!1,e.preventDefault()}),r.addEventListener("mouseleave",function(e){h=setTimeout(i,1e3*n.timeout),e.preventDefault()})),r}function p(e,r,a){"showBugs"!=e&&i();var p,u,f=t("div");if(f.style.setProperty("background","showBugs"==e?"#330033":"#777","important"),"showBugs"==e){o(f,c(a));for(var m=0;r.length>m;m++)o(f,l(r[m].name,r[m].blocked?g+"-blocked":""),n())}else{if("showUpdateAlert"!=e){var b=s("https://purplebox.ghostery.com/releases/releases-chrome",v.notification_upgrade);b.addEventListener("click",function(e){e.preventDefault(),A("openTab",{url:e.target.href})}),o(f,b)}("showWalkthroughAlert"==e||"showUpdateAlert"==e)&&("showUpdateAlert"==e?(o(f,l(v.notification_update)),u=s("",v.notification_update_link)):(o(f,n(),n(),l(v.notification_reminder1),n(),l(v.notification_reminder2)),u=s("",v.notification_reminder_link)),u.addEventListener("click",function(t){A("showUpdateAlert"==e?"showNewTrackers":"openWalkthrough"),t.preventDefault()}),o(f,n(),n(),u)),u=s(!1,v.dismiss),u.addEventListener("click",function(e){i(),e.preventDefault()}),o(f,n(),n(),u)}p=E.getElementById(g),p||(p=d(e,a)),"showBugs"==e&&(p.title=x.alert_bubble_tooltip),p.innerHTML="",o(p,f),clearTimeout(h),a&&a.timeout&&h&&(h=setTimeout(i,1e3*a.timeout))}function c(e){var n=t("a");return n.innerHTML="&nbsp;",n.href="#",n.id=g+"-gear",n.title=x.alert_bubble_gear_tooltip,n.style.setProperty(e&&"left"==e.pos_x?"left":"right","0","important"),n.style.setProperty(e&&"bottom"==e.pos_y?"bottom":"top","0","important"),n.style.setProperty("border-"+(e&&"bottom"==e.pos_y?"top":"bottom")+"-"+(e&&"left"==e.pos_x?"right":"left")+"-radius","3px","important"),n.style.setProperty("border-"+(e&&"bottom"==e.pos_y?"bottom":"top")+"-"+(e&&"left"==e.pos_x?"left":"right")+"-radius","3px","important"),n.addEventListener("click",function(e){A("showPurpleBoxOptions"),e.preventDefault()}),n}function u(e,t,n){e.addEventListener("load",function(){var o=e.contentDocument;o.documentElement.innerHTML=n,t.button?(e.style.width="30px",e.style.height="19px",e.style.border="0px"):(e.style.width="100%",e.style.border="1px solid #ccc",e.style.height="80px"),t.frameColor&&(e.style.background=t.frameColor),o.getElementById("action-once").addEventListener("click",function(e){A("processC2P",{action:"once",app_ids:t.allow}),e.preventDefault()},!0),t.button||o.getElementById("action-always").addEventListener("click",function(e){A("processC2P",{action:"always",app_ids:t.allow}),e.preventDefault()},!0)},!1)}function f(e,n,r){n.forEach(function(e,n){for(var a=E.querySelectorAll(e.ele),i=0,s=a.length;s>i;i++){var l=a[i];if(e.attach&&"parentNode"==e.attach){if(l.parentNode&&"BODY"!=l.parentNode.nodeName&&"HEAD"!=l.parentNode.nodeName){var d=t("div");l.parentNode.replaceChild(d,l),l=d}}else l.textContent="";l.style.display="block";var p=t("iframe");u(p,e,r[n]),o(l,p)}})}var m=!1,g=e(),h=9999,b={},y=!1,x={},v={},w=!1,k=chrome.extension,_=chrome.runtime,E=document,L=window,B=_&&_.onMessage||k.onMessage,A=function(e,t){return(_&&_.sendMessage||k.sendMessage)({name:e,message:t})};B.addListener(function(e,t,n){if(!t.tab||0===t.tab.url.indexOf(k.getURL(""))){var o=["show","showUpgradeAlert","showWalkthroughAlert","showUpdateAlert"],i=e.name,s=e.message;"c2p"==i&&(b[s.app_id]=[s.app_id,s.data,s.html],"complete"==E.readyState&&f.apply(this,b[s.app_id])),-1!=o.indexOf(i)?(y||(y=!0,a()),"show"==i?(x=s.translations,w||m||p("showBugs",s.bugs,s.alert_cfg)):(v=s.translations,p(i),w=!0)):"surrogate"==i?r(null,s.surrogate):"reload"==i&&E.location.reload(),n({})}}),L.addEventListener("load",function(){for(var e in b)f.apply(this,b[e])},!1),A("pageInjected")})();