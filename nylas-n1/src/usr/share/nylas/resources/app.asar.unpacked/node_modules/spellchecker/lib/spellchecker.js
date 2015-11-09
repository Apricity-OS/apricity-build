var path = require('path');
var bindings = require('../build/Release/spellchecker.node');

var Spellchecker = bindings.Spellchecker;

var defaultSpellcheck = null;
var ensureDefaultSpellCheck = function() {
  if (defaultSpellcheck) return;

  defaultSpellcheck = new Spellchecker();

  // NB: Windows 8 uses *dashes* to set the language (i.e. en-US), so if we fail
  // to set the language, try the Windows 8 way
  var dict = getDictionaryPath();
  if (!defaultSpellcheck.setDictionary('en_US', dict)) {
    defaultSpellcheck.setDictionary('en-US', dict);
  }
};

var isMisspelled = function() {
  ensureDefaultSpellCheck();

  return defaultSpellcheck.isMisspelled.apply(defaultSpellcheck, arguments);
};

var add = function() {
  ensureDefaultSpellCheck();

  defaultSpellcheck.add.apply(defaultSpellcheck, arguments);
};

var getCorrectionsForMisspelling = function() {
  ensureDefaultSpellCheck();

  return defaultSpellcheck.getCorrectionsForMisspelling.apply(defaultSpellcheck, arguments);
};

var getAvailableDictionaries = function() {
  ensureDefaultSpellCheck();

  return defaultSpellcheck.getAvailableDictionaries.apply(defaultSpellcheck, arguments);
};

var getDictionaryPath = function() {
  var dict = path.join(__dirname, '..', 'vendor', 'hunspell_dictionaries');
  try {
    // HACK: Special case being in an asar archive
    var unpacked = dict.replace('.asar' + path.sep, '.asar.unpacked' + path.sep);
    if (require('fs').statSyncNoException(unpacked)) {
      dict = unpacked;
    }
  } catch (error) {
  }
  return dict;
}

module.exports = {
  add: add,
  isMisspelled: isMisspelled,
  getAvailableDictionaries: getAvailableDictionaries,
  getCorrectionsForMisspelling: getCorrectionsForMisspelling,
  Spellchecker: Spellchecker
};
