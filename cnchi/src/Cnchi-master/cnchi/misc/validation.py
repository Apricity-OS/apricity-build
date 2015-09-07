# -*- coding: utf-8; Mode: Python; indent-tabs-mode: nil; tab-width: 4 -*-

#  Miscellaneous validation of user-entered data
#
#  Copyright (C) 2005 Junta de Andalucía
#  Copyright (C) 2005, 2006, 2007, 2008 Canonical Ltd.
#
#  Validation library.
#  Created by Antonio Olmo <aolmo#emergya._info> on 26 jul 2005.


def check_grub_device(device):
    """Check that the user entered a valid boot device.
        @return True if the device is valid, False if it is not."""
    import re
    import os
    regex = re.compile(r'^/dev/([a-zA-Z0-9]+|mapper/[a-zA-Z0-9_]+)$')
    if regex.search(device):
        if not os.path.exists(device):
            return False
        return True
    # (device[,part-num])
    regex = re.compile(r'^\((hd|fd)[0-9]+(,[0-9]+)*\)$')
    if regex.search(device):
        return True
    else:
        return False

NAME_LENGTH = 1
NAME_BADCHAR = 2
NAME_BADHYPHEN = 3
NAME_BADDOTS = 4


def check(element, value):
    if element == 'username':
        return check_username(value)
    if element == 'hostname':
        return check_hostname(value)


def check_username(name):
    """ Check the correctness of a proposed user name.

        @return empty list (valid) or list of:
            - C{NAME_LENGTH} wrong length.
            - C{NAME_BADCHAR} contains invalid characters.
            - C{NAME_BADHYPHEN} starts or ends with a hyphen.
            - C{NAME_BADDOTS} contains consecutive/initial/final dots."""

    import re
    result = set()

    if len(name) < 1 or len(name) > 40:
        result.add(NAME_LENGTH)

    regex = re.compile(r'^[a-z0-9.\-]+$')
    if not regex.search(name):
        result.add(NAME_BADCHAR)
    if name.startswith('-') or name.endswith('-'):
        result.add(NAME_BADHYPHEN)
    if '.' in name:
        result.add(NAME_BADDOTS)

    return sorted(result)


def check_hostname(name):
    """ Check the correctness of a proposed host name.

        @return empty list (valid) or list of:
            - C{NAME_LENGTH} wrong length.
            - C{NAME_BADCHAR} contains invalid characters.
            - C{NAME_BADHYPHEN} starts or ends with a hyphen.
            - C{NAME_BADDOTS} contains consecutive/initial/final dots."""

    import re
    result = set()

    if len(name) < 1 or len(name) > 63:
        result.add(NAME_LENGTH)

    regex = re.compile(r'^[a-zA-Z0-9.-]+$')
    if not regex.search(name):
        result.add(NAME_BADCHAR)
    if name.startswith('-') or name.endswith('-'):
        result.add(NAME_BADHYPHEN)
    if '..' in name or name.startswith('.') or name.endswith('.'):
        result.add(NAME_BADDOTS)

    return sorted(result)

# Based on setPasswordStrength() in Mozilla Seamonkey, which is tri-licensed
# under MPL 1.1, GPL 2.0, and LGPL 2.1.


def password_strength(password):
    upper = lower = digit = symbol = 0
    for char in password:
        if char.isdigit():
            digit += 1
        elif char.islower():
            lower += 1
        elif char.isupper():
            upper += 1
        else:
            symbol += 1
    length = len(password)
    if length > 5:
        length = 5
    if digit > 3:
        digit = 3
    if upper > 3:
        upper = 3
    if symbol > 3:
        symbol = 3
    strength = (
        ((length * 0.1) - 0.2) +
        (digit * 0.1) +
        (symbol * 0.15) +
        (upper * 0.1))
    if strength > 1:
        strength = 1
    if strength < 0:
        strength = 0
    return strength


def human_password_strength(password):
    strength = password_strength(password)
    length = len(password)
    if length == 0:
        hint = ''
        color = ''
    elif length < 6:
        hint = _('Password is too short')
        color = 'darkred'
    elif strength < 0.5:
        hint = _('Weak password')
        color = 'darkred'
    elif strength < 0.75:
        hint = _('Fair password')
        color = 'darkorange'
    elif strength < 0.9:
        hint = _('Good password')
        color = 'darkgreen'
    else:
        hint = _('Strong password')
        color = 'darkgreen'
    return hint, color


def check_password(password, verified_password, password_ok,
                   password_error_label, password_strength,
                   allow_empty=False):
    complete = True
    passw = password.get_text()
    vpassw = verified_password.get_text()
    if passw != vpassw:
        complete = False
        password_ok.hide()
        if passw and (len(vpassw) / float(len(passw)) > 0.8):
            txt = _("Passwords do not match")
            txt = '<small><span foreground="darkred"><b>{0}</b></span></small>'.format(txt)
            password_error_label.set_markup(txt)
            password_error_label.show()
    else:
        password_error_label.hide()

    if allow_empty:
        password_strength.hide()
    elif not passw:
        password_strength.hide()
        complete = False
    else:
        (txt, color) = human_password_strength(passw)
        txt = '<small><span foreground="{0}"><b>{1}</b></span></small>'.format(color, txt)
        password_strength.set_markup(txt)
        password_strength.show()
        if passw == vpassw:
            password_ok.show()

    return complete
