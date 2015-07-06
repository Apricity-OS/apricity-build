'use strict';

pb.phonebook = { };

pb.addEventListener('signed_out', function(e) {
    pb.phonebook = { };
});

pb.getPhonebook = function(device, done) {
    var cached = pb.phonebook[device.iden];
    if (cached && cached.phonebook && cached.phonebook.length > 0) {
        done(cached);
    } else {
        pb.get(pb.api + '/v2/permanents/phonebook_' + device.iden, function(response) {
            pb.phonebook[device.iden] = response;
            done(response);
        });
    }
};
