/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2010, Ajax.org B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Ajax.org B.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AJAX.ORG B.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

define('ace/theme/github', ['require', 'exports', 'module' , 'ace/lib/dom'], function(require, exports, module) {

exports.isDark = false;
exports.cssClass = "ace-github";
exports.cssText = ".ace-github .ace_gutter {\
background: #e8e8e8;\
color: #333\
}\
/* CSS style content from github's default pygments highlighter template.\
Cursor and selection styles from textmate.css. */\
.ace-github .ace_gutter {\
background: #e8e8e8;\
color: #AAA\
}\
.ace-github {\
background: #fff;\
color: #000\
}\
.ace-github .ace_boolean,\
.ace-github .ace_constant.ace_language,\
.ace-github .ace_keyword,\
.ace-github .ace_paren {\
font-weight: bold\
}\
.ace-github .ace_string {\
color: #D14\
}\
.ace-github .ace_variable.ace_class,\
.ace-github .ace_variable.ace_instance {\
color: teal\
}\
.ace-github .ace_constant.ace_numeric {\
color: #099\
}\
.ace-github .ace_constant.ace_buildin,\
.ace-github .ace_support.ace_function,\
.ace-github .ace_variable.ace_language {\
color: #0086B3\
}\
.ace-github .ace_comment {\
color: #998;\
font-style: italic\
}\
.ace-github .ace_fold {\
background-color: #DD1144;\
border-color: #555555\
}\
.ace-github .ace_entity.ace_name.ace_tag,\
.ace-github .ace_keyword.ace_other.ace_unit,\
.ace-github .ace_storage.ace_type {\
color: #445588\
}\
.ace-github .ace_string.ace_regexp {\
color: #009926;\
font-weight: normal\
}\
.ace-github .ace_constant.ace_character,\
.ace-github .ace_constant.ace_other,\
.ace-github .ace_entity.ace_other.ace_attribute-name,\
.ace-github .ace_storage,\
.ace-github .ace_support.ace_constant {\
color: #008080\
}\
.ace-github .ace_entity.ace_name.ace_function,\
.ace-github .ace_support.ace_constant.ace_property-value,\
.ace-github .ace_variable {\
color: #DD1144\
}\
.ace-github .ace_constant.ace_character.ace_entity {\
color: #800080\
}\
.ace-github .ace_support.ace_class,\
.ace-github .ace_support.ace_type,\
.ace-github .ace_variable.ace_parameter {\
font-style: italic;\
color: #008080\
}\
.ace-github .ace_invalid,\
.ace-github .ace_invalid.ace_deprecated {\
color: #f8f8f0;\
background-color: #00a8c6\
}\
.ace-github .ace_cursor {\
color: black\
}\
.ace-github .ace_marker-layer .ace_active-line {\
background: rgb(255, 255, 204)\
}\
.ace-github .ace_marker-layer .ace_selection {\
background: rgb(181, 213, 255)\
}\
.ace-github .ace_invalid.ace_illegal {\
color: #A61717;\
background-color: #E3D2D2\
}\
.ace-github.ace_multiselect .ace_selection.ace_start {\
box-shadow: 0 0 3px 0px white;\
border-radius: 2px\
}\
/* bold keywords cause cursor issues for some fonts */\
/* this disables bold style for editor and keeps for static highlighter */\
.ace-github.ace_nobold .ace_line > span {\
font-weight: normal !important\
}\
.ace-github .ace_marker-layer .ace_step {\
background: rgb(252, 255, 0)\
}\
.ace-github .ace_marker-layer .ace_stack {\
background: rgb(164, 229, 101)\
}\
.ace-github .ace_marker-layer .ace_bracket {\
margin: -1px 0 0 -1px;\
border: 1px solid rgb(192, 192, 192)\
}\
.ace-github .ace_gutter-active-line {\
background-color : rgba(0, 0, 0, 0.07)\
}\
.ace-github .ace_marker-layer .ace_selected-word {\
background: rgb(250, 250, 255);\
border: 1px solid rgb(200, 200, 250)\
}\
.ace-github .ace_print-margin {\
width: 1px;\
background: #e8e8e8\
}\
.ace-github .ace_markup.ace_heading {\
color: #AAAAAA\
}\
.ace-github .ace_markup.ace_heading.ace_1 {\
color: #999999\
}\
.ace-github .ace_indent-guide {\
background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAACCAYAAACZgbYnAAAAE0lEQVQImWP4////f4bLly//BwAmVgd1/w11/gAAAABJRU5ErkJggg==\") right repeat-y\
}";

var dom = require("../lib/dom");
dom.importCssString(exports.cssText, exports.cssClass);
});
