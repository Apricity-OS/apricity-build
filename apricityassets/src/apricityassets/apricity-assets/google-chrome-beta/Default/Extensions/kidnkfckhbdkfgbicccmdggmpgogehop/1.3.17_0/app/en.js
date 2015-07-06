var _msg ={
  "$lang": "en",
  "app.name": "Maxiang",
  "app.title": "Maxiang - Markdown Editor for Evernote",
  "app.description": "The Best Markdown Editor for Evernote",
  "Preparing": "Preparing for greatness",
  "New Doc": "New Doc",
  "Evernote International": "Evernote",
  "Link Evernote": "Link with Evernote",
  "Save Evernote": "Save to Evernote",
  "Link account first": "Link account first",
  "FILE_CONFLICT_MSG": "Detected that there are local modifications. The local version would be opened. Please carefully check the two versions and decide whether to sync or delete the local one. Please backup your data fisrt.",
  "Are you sure you want to delete?": "Are you sure to delete <code id='remove-filename'></code>?",
  "MESSAGE": {
    "UNAUTHORIZED_NOTE": "Unauthorized to open this note. Please check your account.",
    "ERROR_OPEN_FILE": "Fail to open file",
    "READONLY_MODE": "Detected the file is editing by another window. Opened in readonly mode.",
    "NOT_AUTHED": "Haven't linked with Evernote",
    "EXCEED_NOTE_LIMIT": "Exceed note limit",
    "EXCEED_ACCOUNT_LIMIT": "Exceed user storage quota",
    "AUTH_ERROR": "Authentication error. Please link with  Evernote account again",
    "AUTH_EXPIRED": "Authentication expired. Please link with Evernote account again",
    "RATE_LIMIT": "Operations too frequency. Please retry next hour.",
    "ILLEGAL_CONTENT": "Illegal content",
    "UNKNOWN": "Unknown error",
    "BAD_DATA_FORMAT": "Bad data format",
    "PERMISSION_DENIED": "Permission denied",
    "INTERNAL_ERROR": "Internal error",
    "DATA_REQUIRED": "Data required"
  }
}
 var MSG = function(key){ return _msg[key] || key }