1.0.60 (1.1alpha1, 2010-xx-xx)
==============================

Features added since the last feature release include:
------------------------------------------------------

* Mount point management

* Support for new "sector based" alignment (vs. the traditional cylinder based
  alignment) required for compatibility with newer disks (where the phyisical
  sector size is 4096 bytes)

* SMART status reports

* Support new file systems: BTRFS, HPFS, LUKS, OCFS2 and ZFS

* Support GPT partition tables.

* Support for setting volume labels on FAT16 and FAT32 file systems

* Export and import a partition table in a human readable and editable format
  similar, but for technical reasons not identical, to sfdisk's.

* Support for backend plugins that do the heavy lifting. The first (and for now
  only) implementation is libparted-based.

* New operation: Shredding (i.e., securely deleting) a partition

* A configuration dialog including settings for:

	* File system colors

	* The default file system for new partitions

	* Applying operations as non-root

	* Setting the log level for the log output dock.

	* The backend plugin to use

	* How to shred: Overwrite with zeros (faster) or random data (slower, but
	  more secure)

* Allow editing start and end sectors in size dialogs in a new "advanced"
  dialog section.

* Instantly align partition start and end in size dialogs if partition
  alignment is on.

* Show a progress dialog when scanning for devices.

* A device properties dialog.


Additional changes:
-------------------

* Allow the Information dock widget to be docked at the top of bottom

* The partition list view now has more information available in additional
  columns. The user can sort the columns and turn individual columns on and off.

* Use the icon suggested by KDE's Solid backend in the Devices list.

* Always select the first device on program start.

* Use KDE's KIO for file transfer where possible.

* Performance improvements in the dialog to change a partition's size.

* The KPart and KCM have been removed.

* Allow hiding/showing the menu bar.

* Add a context menu to the log output with entries to save or clear the log.

* GUI support for devices with more than 2^31 sectors.

* Format sector and size values according to KDE settings for number formatting
  in New Partition, Resize and Insert dialogs.

* Allow fractional sizes in New Partition, Resize and Insert dialogs.


1.0.2 (2010-04-23)
==================

* Copy a file system's UUID to the copied file system when creating a file
  system from another one.

* Implement a workaround for a libparted bug that makes it sometimes fail to
  commit changes to the OS in versions earlier than 2.2.

* Update the partition's file system and re-check for constraints if the
  partition's role is changed in the "Create New Partition" dialog. This fixes
  a bug where the user was not able to grow an extended partition if he'd
  previously selected a file system that cannot grow beyond a certain size.

* Always use the correct icons in modified ok/cancel buttons in dialogs and
  message boxes

* Fix a crash when operations to create, resize and delete an extended
  partitions where incorrectly merged into one (bug 232092)


1.0.1 (2010-01-09)
==================

* Fix a bug that made it impossible to activate or deactivate a swap file
  system.

* Correctly handle linux swap even with newer parted versions.

* Ask for an administrator's password on startup so that non-KDE users can run
  the application from their launcher.

* Use the "blkid" command as an alternative to "vol_id" if the latter is
  unavailable.

* Do not crash when using the Oxygen style under KDE SC 4.4.

* Do not crash when the user clicks in the partition widget with newer
  g++ versions.

* Sort devices by name after scanning.

* Don't disable updates for the partition widget while applying operations.
  This leads to corrupted graphics unter Qt4.6.

* Work around a LibParted segmentation fault when the Linux Device Mapper is
  used and /dev/mapper has non-existing entries.

* The above also works around a LibParted problem when the BIOS has a floppy
  drive configured but none is actually present.

* Set item height in the partition and devices lists to 32 pixel to make them
  look less condensed.

1.0.0 (2009-08-17)
==================

None.

1.0.0-RC1 (2009-08-03)
======================

* Correctly handle ext4 file systems even with patched parted 1.8.8 (and
  hopefully 1.8.9 too). Patch by Fatih Asici <fatih@pardus.org.tr>. (bug
  #195243)

* Clear the partition flags for a copied partition in the preview. (bug
  #202346)

* Write the new start sector to the partition's boot sector if an NTFS file
  system was moved or copied. (bug #202329)


1.0.0-BETA3 (2009-06-04)
========================

* Set the default file system in the New-Partition-dialog, don't just rely on
  it being the first one in the list.

* Sort items in file system combo boxes case-insensitively.

* Fix a bug where the total free space available could get smaller and smaller
  when moving an existing partition in a dialog.

* Fix a bug that the progress information wasn't set as window title for the
  top level application window.

* Speed up copying file systems (and thus moving, resizing and copying).

* Add some basic timing output to the detailed report for copying file systems.

* Find a file system's mount point even if it is identified by label in fstab.

* Add support for reading file system labels from FAT16 and FAT32.

* Fix a bug where the volume label for a linuxswap file system was lost when
  resizing it.

* Make sure all interesting information shown in labels anywhere in the
  application is user-selectable with the mouse.

* Display the UUID for most file systems that support it.

* Always show the current file system in the partition properties dialog's file
  system combo box, even if it cannot be created, is too big or too small.


1.0.0-BETA2 (2009-04-30)
========================

* Add an application icon contributed by David Miller.

* Allow setting the file system label in the dialog when creating a new partition.

* Add a context menu to operation list.

* Use a shell script to run partition manager as a child of hal-lock. This
  should in theory (and according to the hal-lock manpage) avoid notifications
  for new devices, but doesn't seem to work that well.

* Fix a bug that would keep the user from deleting a newly created logical
  partition because the application thought there were higher-numbered
  partitions still mounted.

* Add a kcm for KDE Partition Manager. If this is built and installed or not
  can be (like it already is the case with the KPart) configured via cmake.


1.0.0-BETA1 (2009-01-13)
========================

* Fix a potential crash when merging a New Operation and a Create File System
  Operation.

* Set extended partitions as busy as long as logical partitions inside them
  are mounted.

* Don't silently succeed  when unmounting a partition that cannot in fact be
  unmounted because it has no mount points.

* Prevent creating a new partition table on a device with mounted partitions.

* Add tooltips to partition widgets.

* Don't show cd/dvd readers or writers as devices even if libparted reports
  them.

* Add support for ext4.

* Fix calculation of reserved/free blocks for ext2/3/4.

* Output reason why a mount or unmount might have failed.

* Fix minimum width for extended partitions in the partition table widget.

* Fix a bug where a partition table just created would not disappear when the
  operation to create it was undone.

* Don't allow creating partitions smaller than the device's cylinder size.

* Fix an error that could lead to some space (a few megabytes) between two
  partitions being wasted.


1.0.0-ALPHA2 (2008-09-24)
=========================

* Add and install a .desktop file.

* Don't allow the user to close the main window while operations are being
  applied.

* Handle activating and deactivating swap. Also, swap does have labels.

* Don't try to print a mount point in the partition properties dialog if there
  is none.

* Show partition table type name and used/max primary partitions in the info
  pane.

* All disk label types but msdos are read only for now, until the support
  situation for them improves.

* Set default value for maximum number of primaries from 255 to 4.

* Actually set the correct disk label type before applying the operation to
  create a new disk label.

* Don't update the partition table widget while operations are being applied.
  This fixes a race condition that would potentially crash the application when
  applying operations.

* Update devices in the GUI after creating a new disk label or undoing
  operations.

* Set the operation state of all operations to StateNone when temporarily
  undoing them right before starting the thread to apply them. This fixes a
  crash when an operation fails and the remaining ones are undone twice.

* Update bug report address and homepage in the aboutdata.

* Unblock SIGCHLD in main so Kubuntu's kdesudo can run the application as root.

* If udevadm settle fails, try to call udevsettle to better support older
  systems.

* Fix the minimum size calculations for partition widgets.

* Add some doxygen documentation.


1.0.0-ALPHA1 (2008-09-18)
=========================

* First public release

