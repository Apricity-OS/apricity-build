=begin
***************************************************************************
*   Copyright (C) 2008-2009 by Volker Lanz <vl@fidra.de>                  *
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

class TranslationStatsBuilder
	def initialize(appName, version, workingDir, outputDir)
		@appName = appName
		@version = version
		@workingDir = workingDir
		@outputDir = outputDir
	end

	def run
		Dir.chdir "#{@workingDir}/#{@outputDir}"
	
		file = File.new("../#{@appName}-#{@version}-l10n-stats.html", File::CREAT | File::RDWR | File::TRUNC)
		
		statsHeader(file)

		numLanguages = 0
		translatedPercent = 0
		
		Dir.entries('po').sort.each do |lang|
			next if lang == '.' or lang == '..' or lang == 'CMakeLists.txt'
			numLanguages, translatedPercent = statsLine(file, lang, numLanguages, translatedPercent)
		end

		statsFooter(file, numLanguages, translatedPercent)

		file.close
	end

private
	def statsHeader(file)
		file.print <<END_OF_TEXT
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
	<meta http-equiv="content-type" content="text/html; charset=utf-8">
	<title>#{@appName} #{@version} Translation Statistics</title>
	<style>
		.th { color: #196aff; font-weight: bold; height: 12px; }
	</style>
</head>
<body>
<h1>#{@appName} #{@version} Translation Statistics</h1>
<br>
<table border="1" cellspacing="0">
	<tr>
		<th>Language</th>
		<th>Fuzzy</th>
		<th>Untranslated</th>
		<th>Total Missing</th>
		<th>Complete</th>
	</tr>
END_OF_TEXT
	end

	def statsFooter(file, numLanguages, translatedPercent)
		file.print <<END_OF_TEXT
	<tr>
		<th>#{numLanguages}</th>
		<th></th>
		<th></th>
		<th></th>
		<th>#{translatedPercent.to_s + " %"}</th>
	</tr>
</table>
</body>
</html>
END_OF_TEXT
	end

	def self.fileStats(filename)
		values = `msgfmt --statistics #{filename} -o /dev/null 2>&1`.scan /[\d]+/
		fuzzy = values[1].to_i
		untranslated = values[2].to_i
		total = values[0].to_i + values[1].to_i + values[2].to_i
		percentage = (total - fuzzy - untranslated) * 100 / total
		return fuzzy, untranslated, percentage
	end
	
	def statsLine(file, langName, numLanguages, translatedPercent)
		return 0, 0 if not File.exists?("po/#{langName}/#{@appName}.po")

		fuzzy, untranslated, percentage = TranslationStatsBuilder.fileStats("po/#{langName}/#{@appName}.po")
		
		textColor = case percentage
			when 0...70 then 'red'
			when 70...100 then 'orange'
			else 'green'
		end

		file.print <<END_OF_TEXT
	<tr>
		<td style="text-align: left; color: #{textColor}">#{langName}</td>
		<td style="text-align: center; color: #{textColor}">#{fuzzy}</td>
		<td style="text-align: center; color: #{textColor}">#{untranslated}</td>
		<td style="text-align: center; color: #{textColor}">#{fuzzy + untranslated}</td>
		<td style="text-align: center; color: #{textColor}">#{percentage.to_s + " %"}</td>
	</tr>
END_OF_TEXT

		numLanguages += 1
		translatedPercent = translatedPercent.to_i != 0 ? (translatedPercent + percentage) / 2 : percentage

		return numLanguages, translatedPercent
	end
end

