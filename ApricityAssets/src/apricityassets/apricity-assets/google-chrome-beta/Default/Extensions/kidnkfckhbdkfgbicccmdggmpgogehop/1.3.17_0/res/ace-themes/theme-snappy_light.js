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

define('ace/theme/snappy_light', ['require', 'exports', 'module' , 'ace/lib/dom'], function(require, exports, module) {

exports.isDark = false;
exports.cssClass = "ace-snappy-light";
exports.cssText = "/* THIS THEME WAS AUTOGENERATED BY Theme.tmpl.css (UUID: f95e005b-d9eb-f5b6-9a70-38262a6c2180) */\
.ace-snappy-light .ace_gutter {\
background: #e8e8e8;\
color: #333;\
}\
.ace-snappy-light .ace_print-margin {\
width: 1px;\
background: #e8e8e8;\
}\
.ace-snappy-light {\
background-color: #ffffff;\
color: #555555;\
}\
.ace-snappy-light .ace_cursor {\
color: #444444;\
}\
.ace-snappy-light .ace_marker-layer .ace_selection {\
background: #808dd3;\
}\
.ace-snappy-light.ace_multiselect .ace_selection.ace_start {\
box-shadow: 0 0 3px 0px #ffffff;\
border-radius: 2px;\
}\
.ace-snappy-light .ace_marker-layer .ace_step {\
background: rgb(198, 219, 174);\
}\
.ace-snappy-light .ace_marker-layer .ace_bracket {\
margin: -1px 0 0 -1px;\
border: 1px solid #3b3a32;\
}\
.ace-snappy-light .ace_marker-layer .ace_active-line {\
background: #eeeeee;\
}\
.ace-snappy-light .ace_gutter-active-line {\
background-color: #eeeeee;\
}\
.ace-snappy-light .ace_marker-layer .ace_selected-word {\
border: 1px solid #808dd3;\
}\
.ace-snappy-light .ace_fold {\
background-color: #808dd3;\
border-color: #555555;\
}\
.ace-snappy-light .ace_keyword{color:#da564a;}.ace-snappy-light .ace_keyword.ace_other.ace_unit{color:#4ea1df;}.ace-snappy-light .ace_constant.ace_language{color:#f66153;}.ace-snappy-light .ace_constant.ace_numeric{color:#4ea1df;}.ace-snappy-light .ace_constant.ace_character{color:#f66153;}.ace-snappy-light .ace_constant.ace_other{color:#f66153;}.ace-snappy-light .ace_support.ace_function{color:#606aa1;}.ace-snappy-light .ace_support.ace_constant{color:#f66153;}.ace-snappy-light .ace_support.ace_constant.ace_property-value{color:#4ea1df;}.ace-snappy-light .ace_support.ace_class{font-style:italic;\
color:#f66153;}.ace-snappy-light .ace_support.ace_type{font-style:italic;\
color:#f66153;}.ace-snappy-light .ace_storage{color:#f66153;}.ace-snappy-light .ace_storage.ace_type{color:#4ea1df;}.ace-snappy-light .ace_invalid{color:#f8f8f0;\
background-color:#00a8c6;}.ace-snappy-light .ace_invalid.ace_deprecated{color:#f8f8f0;\
background-color:#00a8c6;}.ace-snappy-light .ace_string{color:#4ea1df;}.ace-snappy-light .ace_comment{color:#bbbbbb;}.ace-snappy-light .ace_variable{color:#808dd3;}.ace-snappy-light .ace_variable.ace_parameter{font-style:italic;}.ace-snappy-light .ace_entity.ace_other.ace_attribute-name{color:#f66153;}.ace-snappy-light .ace_entity.ace_name.ace_function{color:#808dd3;}.ace-snappy-light .ace_entity.ace_name.ace_tag{color:#4ea1df;}";

var dom = require("../lib/dom");
dom.importCssString(exports.cssText, exports.cssClass);
});
