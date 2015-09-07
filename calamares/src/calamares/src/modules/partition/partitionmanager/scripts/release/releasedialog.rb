#!/usr/bin/ruby
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

require 'Qt4'

# generate with:
# $ rbuic4 releasedialog.ui > ui_releasedialog.rb
require 'ui_releasedialog.rb'

require 'releasebuilder.rb'

class ReleaseDialog < Qt::Dialog
	slots 'on_comboAccess_currentIndexChanged(int)',
		'on_comboCheckout_currentIndexChanged(int)',
		'on_checkTranslations_toggled(bool)'

	def initialize(app)
		super()

		@app = app

		@ui = Ui_ReleaseDialog.new()
		@ui.setupUi(self)

		@ui.labelName.setText(@app.product)
	end

	def validate
		if @ui.editVersion.text.empty?
			Qt::MessageBox.information(self, tr('Missing Information'), tr('The application\'s version can not be empty.'))
			return false
		end
	
		if @ui.comboAccess.currentIndex != 0 and @ui.editUser.text.empty?
			Qt::MessageBox.information(self, tr('Missing Information'), tr('Please provide a user name for the selected SVN access method.'))
			return false
		end
		
		return true
	end
	
	def accept
		return if not validate

		skipBelow = @ui.checkSkipTrans.isChecked ? @ui.spinSkipTrans.value : 0

		hide
		
		releaseBuilder = ReleaseBuilder.new(@ui.comboCheckout.currentText, @ui.comboTag.currentText, @app, Dir.getwd, @ui.comboAccess.currentText, @ui.editUser.text, @ui.editVersion.text)
		releaseBuilder.run(@ui.checkTarball.isChecked, @ui.checkTranslations.isChecked, skipBelow, @ui.checkDocs.isChecked, @ui.checkTag.isChecked, @ui.checkFixes.isChecked)

		super
	end

private
	def on_comboAccess_currentIndexChanged(index)
		@ui.editUser.setEnabled(index > 0)
	end
	
	def on_comboCheckout_currentIndexChanged(index)
		@ui.comboTag.clear
		@ui.comboTag.setEnabled(index == 1 && updateBranches || index == 2 && updateTags)
	end
	
	def on_checkTranslations_toggled(state)
		@ui.checkSkipTrans.setEnabled(state)
		@ui.sliderSkipTrans.setEnabled(state && @ui.checkSkipTrans.isChecked)
		@ui.spinSkipTrans.setEnabled(state && @ui.checkSkipTrans.isChecked)
	end
	
	def updateTags
		tags = `svn ls svn://anonsvn.kde.org/home/kde/tags/#{@app.name}`.chomp!
		
		return false if not tags or tags.length == 0
		
		tags.sort.each { |t| @ui.comboTag.addItem(t.delete("/\n\r")) }
		return true
	end

	def updateBranches
		branches = `svn ls svn://anonsvn.kde.org/home/kde/branches/#{@app.name}`.chomp!
		
		return false if not branches or branches.length == 0
		
		branches.sort.each { |t| @ui.comboTag.addItem(t.delete("/\n\r")) }
		return true
	end
end
