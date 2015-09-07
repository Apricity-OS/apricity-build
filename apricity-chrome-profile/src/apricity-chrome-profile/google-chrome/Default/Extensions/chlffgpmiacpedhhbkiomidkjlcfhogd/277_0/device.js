'use strict';

pb.addEventListener('signed_in', function(e) {
    pb.addEventListener('devices_ready', function(e) {
        createOrAttachDevice();
    });

    if (localStorage.device) {
        pb.local.device = JSON.parse(localStorage.device);
    }
});

var inProgress, retryTimeout;
var createOrAttachDevice = function() {
    clearTimeout(retryTimeout);

    if (inProgress) {
        pb.log('Device is already being created or attached');
        return;
    } else {
        inProgress = true;
    }

    var attachTo, device;
    Object.keys(pb.local.devices).forEach(function(key) {
        device = pb.local.devices[key];
        if (device.active) {
            if ((pb.local.device && pb.local.device.iden == device.iden)
                || (window.chrome && device.type == 'chrome' && !pb.isOpera)
                || (pb.isOpera && device.type == 'opera')
                || (window.safari && device.type == 'safari')
                || (!window.chrome && !window.safari && device.type == 'firefox')) {
                attachTo = device;
            }
        }
    });

    if (attachTo) {
        if (!pb.local.device) {
            pb.log('Attaching to existing device:');
            pb.log(attachTo);
        }

        pb.local.device = attachTo;
        localStorage.device = JSON.stringify(attachTo);
        
        inProgress = false;
    } else if (pb.local.device && !attachTo) { // Device has been deleted
        inProgress = false;
        pb.signOut();
    } else {
        createDevice(function(response) {
            if (response) {
                pb.local.device = response;
                localStorage.device = JSON.stringify(response);
            } else {
                var retryTimeout = setTimeout(function() {
                     pb.dispatchEvent('devices_changed');
                }, 30 * 1000);
            }

            inProgress = false;
        });
    }
};

var getDeviceBody = function() {
    if (window.chrome) {
        if (pb.isOpera) {
            return {
                'type': 'opera',
                'manufacturer': 'Opera',
                'model': 'Opera',
                'app_version': pb.version
            };
        }
        return {
            'type': 'chrome',
            'manufacturer': 'Google',
            'model': 'Chrome',
            'app_version': pb.version
        };
    } else if (window.safari) {
        return {
            'type': 'safari',
            'manufacturer': 'Apple',
            'model': 'Safari',
            'app_version': pb.version
        };
    } else {
        return {
            'type': 'firefox',
            'manufacturer': 'Mozilla',
            'model': 'Firefox',
            'app_version': pb.version
        };
    }
};

var createDevice = function(done) {
    pb.log('Creating device');

    var body = getDeviceBody();
    body.nickname = window.chrome ? pb.isOpera ? 'Opera' : 'Chrome' : window.safari ? 'Safari' : 'Firefox';

    pb.post(pb.api + '/v2/devices', body, function(response) {
        done(response);
    });
};

var updateDevice = function(done) {
    pb.log('Updating device');

    pb.post(pb.api + '/v2/devices/' + pb.local.device.iden, getDeviceBody(), function(response) {
        done(response);
    });
};
