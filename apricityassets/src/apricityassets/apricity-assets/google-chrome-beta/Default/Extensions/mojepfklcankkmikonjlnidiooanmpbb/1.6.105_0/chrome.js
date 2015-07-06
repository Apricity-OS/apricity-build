'use strict';
/*global chrome:true */

var defaults = {
  width:   990,
  height:  560
};

var getDimensions = function(screen) {
  if (!screen) {
    return defaults;
  }

  var dimensions = {
    width:   Math.floor(screen.width*0.8),
    height:  Math.floor(screen.height*0.8)
  };

  return dimensions;
};

chrome.app.runtime.onLaunched.addListener(function() {
  var dimensions = getDimensions(screen);

  var supportedLocales = ['aa-dj', 'aa-er', 'aa-et', 'aa', 'af-na', 'af-za', 'af', 'agq-cm', 'agq', 'ak-gh', 'ak', 'am-et', 'am', 'ar-001', 'ar-ae', 'ar-bh', 'ar-dj', 'ar-dz', 'ar-eg', 'ar-eh', 'ar-er', 'ar-il', 'ar-iq', 'ar-jo', 'ar-km', 'ar-kw', 'ar-lb', 'ar-ly', 'ar-ma', 'ar-mr', 'ar-om', 'ar-ps', 'ar-qa', 'ar-sa', 'ar-sd', 'ar-so', 'ar-ss', 'ar-sy', 'ar-td', 'ar-tn', 'ar-ye', 'ar', 'as-in', 'as', 'asa-tz', 'asa', 'ast-es', 'ast', 'az-cyrl-az', 'az-cyrl', 'az-latn-az', 'az-latn', 'az', 'bas-cm', 'bas', 'be-by', 'be', 'bem-zm', 'bem', 'bez-tz', 'bez', 'bg-bg', 'bg', 'bm-latn-ml', 'bm-latn', 'bm-ml', 'bm', 'bn-bd', 'bn-in', 'bn', 'bo-cn', 'bo-in', 'bo', 'br-fr', 'br', 'brx-in', 'brx', 'bs-cyrl-ba', 'bs-cyrl', 'bs-latn-ba', 'bs-latn', 'bs', 'byn-er', 'byn', 'ca-ad', 'ca-es-valencia', 'ca-es', 'ca-fr', 'ca-it', 'ca', 'cgg-ug', 'cgg', 'chr-us', 'chr', 'ckb-arab-iq', 'ckb-arab-ir', 'ckb-arab', 'ckb-iq', 'ckb-ir', 'ckb-latn-iq', 'ckb-latn', 'ckb', 'cs-cz', 'cs', 'cy-gb', 'cy', 'da-dk', 'da-gl', 'da', 'dav-ke', 'dav', 'de-at', 'de-be', 'de-ch', 'de-de', 'de-li', 'de-lu', 'de', 'dje-ne', 'dje', 'dsb-de', 'dsb', 'dua-cm', 'dua', 'dyo-sn', 'dyo', 'dz-bt', 'dz', 'ebu-ke', 'ebu', 'ee-gh', 'ee-tg', 'ee', 'el-cy', 'el-gr', 'el', 'en-001', 'en-150', 'en-ag', 'en-ai', 'en-as', 'en-au', 'en-bb', 'en-be', 'en-bm', 'en-bs', 'en-bw', 'en-bz', 'en-ca', 'en-cc', 'en-ck', 'en-cm', 'en-cx', 'en-dg', 'en-dm', 'en-dsrt-us', 'en-dsrt', 'en-er', 'en-fj', 'en-fk', 'en-fm', 'en-gb', 'en-gd', 'en-gg', 'en-gh', 'en-gi', 'en-gm', 'en-gu', 'en-gy', 'en-hk', 'en-ie', 'en-im', 'en-in', 'en-io', 'en-iso', 'en-je', 'en-jm', 'en-ke', 'en-ki', 'en-kn', 'en-ky', 'en-lc', 'en-lr', 'en-ls', 'en-mg', 'en-mh', 'en-mo', 'en-mp', 'en-ms', 'en-mt', 'en-mu', 'en-mw', 'en-my', 'en-na', 'en-nf', 'en-ng', 'en-nr', 'en-nu', 'en-nz', 'en-pg', 'en-ph', 'en-pk', 'en-pn', 'en-pr', 'en-pw', 'en-rw', 'en-sb', 'en-sc', 'en-sd', 'en-sg', 'en-sh', 'en-sl', 'en-ss', 'en-sx', 'en-sz', 'en-tc', 'en-tk', 'en-to', 'en-tt', 'en-tv', 'en-tz', 'en-ug', 'en-um', 'en-us', 'en-vc', 'en-vg', 'en-vi', 'en-vu', 'en-ws', 'en-za', 'en-zm', 'en-zw', 'en', 'eo-001', 'eo', 'es-419', 'es-ar', 'es-bo', 'es-cl', 'es-co', 'es-cr', 'es-cu', 'es-do', 'es-ea', 'es-ec', 'es-es', 'es-gq', 'es-gt', 'es-hn', 'es-ic', 'es-mx', 'es-ni', 'es-pa', 'es-pe', 'es-ph', 'es-pr', 'es-py', 'es-sv', 'es-us', 'es-uy', 'es-ve', 'es', 'et-ee', 'et', 'eu-es', 'eu', 'ewo-cm', 'ewo', 'fa-af', 'fa-ir', 'fa', 'ff-cm', 'ff-gn', 'ff-mr', 'ff-sn', 'ff', 'fi-fi', 'fi', 'fil-ph', 'fil', 'fo-fo', 'fo', 'fr-be', 'fr-bf', 'fr-bi', 'fr-bj', 'fr-bl', 'fr-ca', 'fr-cd', 'fr-cf', 'fr-cg', 'fr-ch', 'fr-ci', 'fr-cm', 'fr-dj', 'fr-dz', 'fr-fr', 'fr-ga', 'fr-gf', 'fr-gn', 'fr-gp', 'fr-gq', 'fr-ht', 'fr-km', 'fr-lu', 'fr-ma', 'fr-mc', 'fr-mf', 'fr-mg', 'fr-ml', 'fr-mq', 'fr-mr', 'fr-mu', 'fr-nc', 'fr-ne', 'fr-pf', 'fr-pm', 'fr-re', 'fr-rw', 'fr-sc', 'fr-sn', 'fr-sy', 'fr-td', 'fr-tg', 'fr-tn', 'fr-vu', 'fr-wf', 'fr-yt', 'fr', 'fur-it', 'fur', 'fy-nl', 'fy', 'ga-ie', 'ga', 'gd-gb', 'gd', 'gl-es', 'gl', 'gsw-ch', 'gsw-fr', 'gsw-li', 'gsw', 'gu-in', 'gu', 'guz-ke', 'guz', 'gv-im', 'gv', 'ha-latn-gh', 'ha-latn-ne', 'ha-latn-ng', 'ha-latn', 'ha', 'haw-us', 'haw', 'he-il', 'he', 'hi-in', 'hi', 'hr-ba', 'hr-hr', 'hr', 'hsb-de', 'hsb', 'hu-hu', 'hu', 'hy-am', 'hy', 'ia-fr', 'ia', 'id-id', 'id', 'ig-ng', 'ig', 'ii-cn', 'ii', 'in', 'is-is', 'is', 'it-ch', 'it-it', 'it-sm', 'it', 'iw', 'ja-jp', 'ja', 'jgo-cm', 'jgo', 'jmc-tz', 'jmc', 'ka-ge', 'ka', 'kab-dz', 'kab', 'kam-ke', 'kam', 'kde-tz', 'kde', 'kea-cv', 'kea', 'khq-ml', 'khq', 'ki-ke', 'ki', 'kk-cyrl-kz', 'kk-cyrl', 'kk', 'kkj-cm', 'kkj', 'kl-gl', 'kl', 'kln-ke', 'kln', 'km-kh', 'km', 'kn-in', 'kn', 'ko-kp', 'ko-kr', 'ko', 'kok-in', 'kok', 'ks-arab-in', 'ks-arab', 'ks', 'ksb-tz', 'ksb', 'ksf-cm', 'ksf', 'ksh-de', 'ksh', 'kw-gb', 'kw', 'ky-cyrl-kg', 'ky-cyrl', 'ky', 'lag-tz', 'lag', 'lb-lu', 'lb', 'lg-ug', 'lg', 'lkt-us', 'lkt', 'ln-ao', 'ln-cd', 'ln-cf', 'ln-cg', 'ln', 'lo-la', 'lo', 'lt-lt', 'lt', 'lu-cd', 'lu', 'luo-ke', 'luo', 'luy-ke', 'luy', 'lv-lv', 'lv', 'mas-ke', 'mas-tz', 'mas', 'mer-ke', 'mer', 'mfe-mu', 'mfe', 'mg-mg', 'mg', 'mgh-mz', 'mgh', 'mgo-cm', 'mgo', 'mk-mk', 'mk', 'ml-in', 'ml', 'mn-cyrl-mn', 'mn-cyrl', 'mn', 'mr-in', 'mr', 'ms-bn', 'ms-latn-bn', 'ms-latn-my', 'ms-latn-sg', 'ms-latn', 'ms-my', 'ms', 'mt-mt', 'mt', 'mua-cm', 'mua', 'my-mm', 'my', 'naq-na', 'naq', 'nb-no', 'nb-sj', 'nb', 'nd-zw', 'nd', 'ne-in', 'ne-np', 'ne', 'nl-aw', 'nl-be', 'nl-bq', 'nl-cw', 'nl-nl', 'nl-sr', 'nl-sx', 'nl', 'nmg-cm', 'nmg', 'nn-no', 'nn', 'nnh-cm', 'nnh', 'no-no', 'no', 'nr-za', 'nr', 'nso-za', 'nso', 'nus-sd', 'nus', 'nyn-ug', 'nyn', 'om-et', 'om-ke', 'om', 'or-in', 'or', 'os-ge', 'os-ru', 'os', 'pa-arab-pk', 'pa-arab', 'pa-guru-in', 'pa-guru', 'pa', 'pl-pl', 'pl', 'ps-af', 'ps', 'pt-ao', 'pt-br', 'pt-cv', 'pt-gw', 'pt-mo', 'pt-mz', 'pt-pt', 'pt-st', 'pt-tl', 'pt', 'qu-bo', 'qu-ec', 'qu-pe', 'qu', 'rm-ch', 'rm', 'rn-bi', 'rn', 'ro-md', 'ro-ro', 'ro', 'rof-tz', 'rof', 'ru-by', 'ru-kg', 'ru-kz', 'ru-md', 'ru-ru', 'ru-ua', 'ru', 'rw-rw', 'rw', 'rwk-tz', 'rwk', 'sah-ru', 'sah', 'saq-ke', 'saq', 'sbp-tz', 'sbp', 'se-fi', 'se-no', 'se-se', 'se', 'seh-mz', 'seh', 'ses-ml', 'ses', 'sg-cf', 'sg', 'shi-latn-ma', 'shi-latn', 'shi-tfng-ma', 'shi-tfng', 'shi', 'si-lk', 'si', 'sk-sk', 'sk', 'sl-si', 'sl', 'smn-fi', 'smn', 'sn-zw', 'sn', 'so-dj', 'so-et', 'so-ke', 'so-so', 'so', 'sq-al', 'sq-mk', 'sq-xk', 'sq', 'sr-cyrl-ba', 'sr-cyrl-me', 'sr-cyrl-rs', 'sr-cyrl-xk', 'sr-cyrl', 'sr-latn-ba', 'sr-latn-me', 'sr-latn-rs', 'sr-latn-xk', 'sr-latn', 'sr', 'ss-sz', 'ss-za', 'ss', 'ssy-er', 'ssy', 'st-ls', 'st-za', 'st', 'sv-ax', 'sv-fi', 'sv-se', 'sv', 'sw-ke', 'sw-tz', 'sw-ug', 'sw', 'swc-cd', 'swc', 'ta-in', 'ta-lk', 'ta-my', 'ta-sg', 'ta', 'te-in', 'te', 'teo-ke', 'teo-ug', 'teo', 'tg-cyrl-tj', 'tg-cyrl', 'tg', 'th-th', 'th', 'ti-er', 'ti-et', 'ti', 'tig-er', 'tig', 'tl', 'tn-bw', 'tn-za', 'tn', 'to-to', 'to', 'tr-cy', 'tr-tr', 'tr', 'ts-za', 'ts', 'twq-ne', 'twq', 'tzm-latn-ma', 'tzm-latn', 'tzm', 'ug-arab-cn', 'ug-arab', 'ug', 'uk-ua', 'uk', 'ur-in', 'ur-pk', 'ur', 'uz-arab-af', 'uz-arab', 'uz-cyrl-uz', 'uz-cyrl', 'uz-latn-uz', 'uz-latn', 'uz', 'vai-latn-lr', 'vai-latn', 'vai-vaii-lr', 'vai-vaii', 'vai', 've-za', 've', 'vi-vn', 'vi', 'vo-001', 'vo', 'vun-tz', 'vun', 'wae-ch', 'wae', 'wal-et', 'wal', 'xh-za', 'xh', 'xog-ug', 'xog', 'yav-cm', 'yav', 'yi-001', 'yi', 'yo-bj', 'yo-ng', 'yo', 'zgh-ma', 'zgh', 'zh-cn', 'zh-hans-cn', 'zh-hans-hk', 'zh-hans-mo', 'zh-hans-sg', 'zh-hans', 'zh-hant-hk', 'zh-hant-mo', 'zh-hant-tw', 'zh-hant', 'zh-hk', 'zh-tw', 'zh', 'zu-za', 'zu']; // Updated during the build process

  var userLang = (navigator.language || navigator.userLanguage).toLowerCase();
  var supportUserLang = (supportedLocales.indexOf(userLang) !== -1);

  var filename = (function(){
    if (supportUserLang) {
      return 'index-chrome-' + userLang +  '.html';
    }

    return 'index-chrome-en-us.html';
  }());

  chrome.app.window.create(filename,{
    id: 'sunriseWindow',
    'bounds': {
      'width':   dimensions.width,
      'height':  dimensions.height
    },
    minWidth: defaults.width,
    minHeight: defaults.height
  });
});
