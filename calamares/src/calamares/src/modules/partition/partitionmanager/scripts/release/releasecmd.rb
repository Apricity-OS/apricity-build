=begin
***************************************************************************
*   Copyright (C) 2008,2010 by Volker Lanz <vl@fidra.de>                  *
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

require 'releasebuilder.rb'

class ReleaseCommand
	def initialize
		@version = nil
		@checkoutFrom = 'trunk'
		@tag = ''
		@protocol = 'anonsvn'
		@user = ''
		@getDocs = true
		@getTranslations = true
		@skipBelow = 0
		@createTag = false
		@applyFixes = true
		@createTarball = true
	end

	def self.usage
		puts <<END_OF_TEXT
#{$0} [options]
where options are:
	--version (-v): mandatory
	--checkout-from (-c): trunk (default), branches, tags
	--tag (-t): name of tag
	--svn-access (-s): https, svn+ssh, anonsvn (default)
	--svn-user (-u): svn username (for svn+ssh)
	--get-docs (-d): also get documentation (default)
	--no-get-docs (-D): do not get documentation
	--get-translations (-r): also get translations (default)
	--no-get-translations (-R): do not get translations
	--skip-incomplete (-k): skip translations below a given percentage of completeness
	--create-tag (-e): create a new tag
	--no-create-tag (-E): do not create a new tag (default)
	--apply-fixes (-f): apply application-specific fixes to the source (default)
	--no-apply-fixes (-F): do not apply application-specific fixes to the source
	--create-tarball (-b): create a tarball (default)
	--no-create-tarball (-B): do not create a tarball
	--help (-h): show this usage
END_OF_TEXT
	end

	def readOptions
		opts = GetoptLong.new(
			[ '--version', '-v', GetoptLong::REQUIRED_ARGUMENT ],
			[ '--checkout-from', '-c', GetoptLong::REQUIRED_ARGUMENT ],
			[ '--tag', '-t', GetoptLong::REQUIRED_ARGUMENT ],
			[ '--svn-access', '-s', GetoptLong::REQUIRED_ARGUMENT ],
			[ '--svn-user', '-u', GetoptLong::REQUIRED_ARGUMENT ],
			[ '--get-docs', '-d', GetoptLong::NO_ARGUMENT ],
			[ '--no-get-docs', '-D', GetoptLong::NO_ARGUMENT ],
			[ '--get-translations', '-r', GetoptLong::NO_ARGUMENT ],
			[ '--no-get-translations', '-R', GetoptLong::NO_ARGUMENT ],
			[ '--skip-incomplete', '-k', GetoptLong::REQUIRED_ARGUMENT ],
			[ '--create-tag', '-e', GetoptLong::NO_ARGUMENT ],
			[ '--no-create-tag', '-E', GetoptLong::NO_ARGUMENT ],
			[ '--apply-fixes', '-f', GetoptLong::NO_ARGUMENT ],
			[ '--no-apply-fixes', '-F', GetoptLong::NO_ARGUMENT ],
			[ '--create-tarball', '-b', GetoptLong::NO_ARGUMENT ],
			[ '--no-create-tarball', '-B', GetoptLong::NO_ARGUMENT ],
			[ '--help', '-h', GetoptLong::NO_ARGUMENT ]
		)

		opts.each do |opt, arg|
			case opt
				when '--version' then @version = arg
				when '--checkout-from' then @checkoutFrom = arg
				when '--tag' then @tag = arg
				when '--svn-access' then @protocol = arg
				when '--svn-user' then @user = arg
				when '--get-docs' then @getDocs = true
				when '--no-get-docs' then @getDocs = false
				when '--get-translations' then @getTranslations = true
				when '--no-get-translations' then @getTranslations = false
				when '--skip-incomplete' then @skipBelow = arg.to_i
				when '--create-tag' then @createTag = true
				when '--no-create-tag' then @createTag = false
				when '--apply-fixes' then @applyFixes = true
				when '--no-apply-fixes' then @applyFixes = false
				when '--create-tarball' then @createTarball = true
				when '--no-create-tarball' then @createTarball = false
				when '--help' then ReleaseCommand.usage; exit
			end
		end
	end

	def validate
		if not @version
			puts 'Version can not be empty.'
			return false
		end

		if @protocol != 'anonsvn' and @user.empty?
			puts "The SVN protocol '#{@protocol}' requires a user name."
			return false
		end

		if @checkoutFrom == 'tags' and @tag.empty?
			puts 'Cannot check out from tag dir if tag is empty.'
			return false
		end

		return true
	end

	def run(app)
		releaseBuilder = ReleaseBuilder.new(@checkoutFrom, @tag, app, Dir.getwd, @protocol, @user, @version)
		releaseBuilder.run(@createTarball, @getTranslations, @skipBelow, @getDocs, @createTag, @applyFixes)
	end
end
