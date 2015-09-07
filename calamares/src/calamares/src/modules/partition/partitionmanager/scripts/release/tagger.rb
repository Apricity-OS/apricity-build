=begin
***************************************************************************
*   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
*   Copyright (C) 2007-2008 by Harald Sitter <harald@getamarok.com>       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
***************************************************************************
=end

require 'fileutils'
require 'application.rb'

class Tagger
	def initialize(checkoutFrom, checkoutTag, app, protocol, user, version)
		@checkoutFrom = checkoutFrom
		@checkoutTag = checkoutTag
		@app = app
		@protocol = protocol
		@user = user
		@version = version

		@tmpDirName = 'tagging_dir'
	end

	def tagSource
		puts "Tagging source files..."

		repositorySource = ReleaseBuilder.repositoryRoot(@protocol, @user) + ReleaseBuilder.repositoryPath('src', @app, @checkoutFrom, @checkoutTag)
		repositoryTagRoot = ReleaseBuilder.repositoryRoot(@protocol, @user) + ReleaseBuilder.repositoryPath(nil, @app, 'tags', @version)

		`svn mkdir --parents -m 'Directory for tag #{@version}' #{repositoryTagRoot} >/dev/null 2>&1`
		`svn cp -m 'Tag #{@version}' #{repositorySource} #{repositoryTagRoot} >/dev/null 2>&1`
	
		puts "Done tagging source files."
	end

	def tagTranslations(translations)
		return if translations == nil or translations.length == 0

		repositoryTagRoot = ReleaseBuilder.repositoryRoot(@protocol, @user) + ReleaseBuilder.repositoryPath(nil, @app, 'tags', @version)
		
		puts "Tagging translations..."

		`svn co --depth immediates #{repositoryTagRoot} #{@tmpDirName} >/dev/null 2>&1`
		`svn mkdir --parents -m 'Translations for #{@version}' #{repositoryTagRoot}/po >/dev/null 2>&1`
		`svn up #{@tmpDirName}/po >/dev/null 2>&1`

		translations.each do |trans|
			`svn mkdir #{@tmpDirName}/po/#{trans} >/dev/null 2>&1`
			`svn cp po/#{trans} #{@tmpDirName}/po >/dev/null 2>&1`
		end
		
		`svn commit -m 'Tag translations for #{@version}' #{@tmpDirName}/po >/dev/null 2>&1`

		FileUtils.rm_rf @tmpDirName
		
		puts "Done tagging translations."
	end
	
	def tagDocumentation(docs)
		return if docs == nil or docs.length == 0

		repositoryTagRoot = ReleaseBuilder.repositoryRoot(@protocol, @user) + ReleaseBuilder.repositoryPath(nil, @app, 'tags', @version)
		
		puts "Tagging documentation..."
		
		`svn co --depth immediates #{repositoryTagRoot} #{@tmpDirName} >/dev/null 2>&1`
		`svn mkdir --parents -m 'Documentation for #{@version}' #{repositoryTagRoot}/doc >/dev/null 2>&1`
		`svn up #{@tmpDirName}/doc >/dev/null 2>&1`

		docs.each do |doc|
			`svn cp doc/#{doc} #{@tmpDirName}/doc/ >/dev/null 2>&1`
		end
		
		`svn commit -m 'Tag documentation for #{@version}' #{@tmpDirName}/doc >/dev/null 2>&1`

		FileUtils.rm_rf @tmpDirName
	
		puts "Done tagging documentation."
	end
end
