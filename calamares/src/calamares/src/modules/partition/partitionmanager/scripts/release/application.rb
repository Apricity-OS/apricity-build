=begin
***************************************************************************
*   Copyright (C) 2008-2009 by Volker Lanz <vl@fidra.de>                  *
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

class Application
	attr_reader :product, :component, :section, :name
	def initialize(product, component, section, name)
		@product = product
		@component = component
		@section = section
		@name = name
	end

	def applyFixes(workingDir, outputDir)
		puts 'applying fixes for base application class'
	end
end
