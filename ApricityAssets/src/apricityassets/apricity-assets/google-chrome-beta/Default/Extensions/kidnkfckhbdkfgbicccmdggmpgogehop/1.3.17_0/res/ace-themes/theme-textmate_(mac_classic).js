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

define('ace/theme/textmate_(mac_classic)', ['require', 'exports', 'module' , 'ace/lib/dom'], function(require, exports, module) {

exports.isDark = false;
exports.cssClass = "ace-textmate-(mac-classic)";
exports.cssText = "/* THIS THEME WAS AUTOGENERATED BY Theme.tmpl.css (UUID: 71D40D9D-AE48-11D9-920A-000D93589AF6) */\
.ace-textmate-(mac-classic) .ace_gutter {\
background: #e8e8e8;\
color: #333;\
}\
.ace-textmate-(mac-classic) .ace_print-margin {\
width: 1px;\
background: #e8e8e8;\
}\
.ace-textmate-(mac-classic) {\
background-color: #FFFFFF;\
color: #000000;\
}\
.ace-textmate-(mac-classic) .ace_cursor {\
color: #000000;\
}\
.ace-textmate-(mac-classic) .ace_marker-layer .ace_selection {\
background: rgba(77, 151, 255, 0.33);\
}\
.ace-textmate-(mac-classic).ace_multiselect .ace_selection.ace_start {\
box-shadow: 0 0 3px 0px #FFFFFF;\
border-radius: 2px;\
}\
.ace-textmate-(mac-classic) .ace_marker-layer .ace_step {\
background: rgb(198, 219, 174);\
}\
.ace-textmate-(mac-classic) .ace_marker-layer .ace_bracket {\
margin: -1px 0 0 -1px;\
border: 1px solid #BFBFBF;\
}\
.ace-textmate-(mac-classic) .ace_marker-layer .ace_active-line {\
background: rgba(0, 0, 0, 0.071);\
}\
.ace-textmate-(mac-classic) .ace_gutter-active-line {\
background-color: rgba(0, 0, 0, 0.071);\
}\
.ace-textmate-(mac-classic) .ace_marker-layer .ace_selected-word {\
border: 1px solid rgba(77, 151, 255, 0.33);\
}\
.ace-textmate-(mac-classic) .ace_fold {\
background-color: #0000A2;\
border-color: #000000;\
}\
.ace-textmate-(mac-classic) .ace_keyword{color:#0000FF;}.ace-textmate-(mac-classic) .ace_constant{color:#C5060B;}.ace-textmate-(mac-classic) .ace_constant.ace_language{color:#585CF6;}.ace-textmate-(mac-classic) .ace_constant.ace_numeric{color:#0000CD;}.ace-textmate-(mac-classic) .ace_constant.ace_character.ace_escape{color:#26B31A;}.ace-textmate-(mac-classic) .ace_support.ace_function{color:#3C4C72;}.ace-textmate-(mac-classic) .ace_support.ace_constant{color:#06960E;}.ace-textmate-(mac-classic) .ace_support.ace_class{color:#6D79DE;}.ace-textmate-(mac-classic) .ace_support.ace_type{color:#6D79DE;}.ace-textmate-(mac-classic) .ace_storage{color:#0000FF;}.ace-textmate-(mac-classic) .ace_invalid{color:#FFFFFF;\
background-color:#990000;}.ace-textmate-(mac-classic) .ace_string{color:#036A07;}.ace-textmate-(mac-classic) .ace_comment{font-style:italic;\
color:#0066FF;}.ace-textmate-(mac-classic) .ace_variable{color:#0000A2;}.ace-textmate-(mac-classic) .ace_variable.ace_language{color:#318495;}.ace-textmate-(mac-classic) .ace_variable.ace_parameter{font-style:italic;}.ace-textmate-(mac-classic) .ace_meta.ace_tag{color:#1C02FF;}.ace-textmate-(mac-classic) .ace_entity.ace_other.ace_attribute-name{font-style:italic;}.ace-textmate-(mac-classic) .ace_entity.ace_name.ace_function{color:#0000A2;}.ace-textmate-(mac-classic) .ace_markup.ace_heading{color:#0C07FF;}.ace-textmate-(mac-classic) .ace_markup.ace_list{color:#B90690;}";

var dom = require("../lib/dom");
dom.importCssString(exports.cssText, exports.cssClass);
});
