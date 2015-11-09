(function() {
  var keytar;

  keytar = require('../build/Release/keytar.node');

  module.exports = {
    getPassword: function(service, account) {
      if (!((service != null ? service.length : void 0) > 0)) {
        throw new Error("Service is required.");
      }
      if (!((account != null ? account.length : void 0) > 0)) {
        throw new Error("Account is required.");
      }
      return keytar.getPassword(service, account);
    },
    addPassword: function(service, account, password) {
      if (!((service != null ? service.length : void 0) > 0)) {
        throw new Error("Service is required.");
      }
      if (!((account != null ? account.length : void 0) > 0)) {
        throw new Error("Account is required.");
      }
      if (!((password != null ? password.length : void 0) > 0)) {
        throw new Error("Password is required.");
      }
      return keytar.addPassword(service, account, password);
    },
    deletePassword: function(service, account) {
      if (!((service != null ? service.length : void 0) > 0)) {
        throw new Error("Service is required.");
      }
      if (!((account != null ? account.length : void 0) > 0)) {
        throw new Error("Account is required.");
      }
      return keytar.deletePassword(service, account);
    },
    replacePassword: function(service, account, password) {
      if (!((service != null ? service.length : void 0) > 0)) {
        throw new Error("Service is required.");
      }
      if (!((account != null ? account.length : void 0) > 0)) {
        throw new Error("Account is required.");
      }
      if (!((password != null ? password.length : void 0) > 0)) {
        throw new Error("Password is required.");
      }
      keytar.deletePassword(service, account);
      return keytar.addPassword(service, account, password);
    },
    findPassword: function(service) {
      if (!((service != null ? service.length : void 0) > 0)) {
        throw new Error("Service is required.");
      }
      return keytar.findPassword(service);
    }
  };

}).call(this);
