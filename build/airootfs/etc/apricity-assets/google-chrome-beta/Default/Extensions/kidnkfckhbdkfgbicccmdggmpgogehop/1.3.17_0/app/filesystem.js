var FileSystem = {
	request_entry: function(file){
		var self = this;
		if (!file.file_entry){
			return self.request_file_entry(file).then(function(){
				return self.request_entry(file);
			})
		}
		if (file.images && !file.assets_entry){
			return self.request_assets_entry(file)		
		}
	},
	choose_directory: function(){
		chrome.fileSystem.chooseEntry({type: 'openDirectory'}, function(theEntry) {
			if (!theEntry) {
				
				return;
			}
			// use local storage to retain access to this file
			chrome.storage.local.set({'chosenFile': chrome.fileSystem.retainEntry(theEntry)});
			loadDirEntry(theEntry);
		});	
	}
}
