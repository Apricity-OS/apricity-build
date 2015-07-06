/**
 * Listens for the app launching then creates the window
 *
 * @see http://developer.chrome.com/apps/app.runtime.html
 * @see http://developer.chrome.com/apps/app.window.html
 */
chrome.app.runtime.onLaunched.addListener(function(launchData) {
  chrome.i18n.getAcceptLanguages(function(langs){	
      langs.unshift(chrome.i18n.getUILanguage());
	  var is_china = langs.slice(0,3).filter(function(l){
		return /^zh/.test(l);
	  }).length;
      var land_page = is_china ? "index_zh.html" : "index_en.html";//(value && value.prefer_lang) || chrome.i18n.getMessage('index')
	  chrome.app.window.create(land_page,
		{
			frame: "none",
			bounds: {width: 900, height: 600}
		},  function(createdWindow){
			console.log(launchData)
            if (launchData.url) createdWindow.contentWindow._FILE = launchData.url;
            createdWindow.contentWindow.document.addEventListener('keydown', function(evt) {
                if (evt.which == 27){
                    evt.preventDefault();
                }
            });
		});
	})
});
