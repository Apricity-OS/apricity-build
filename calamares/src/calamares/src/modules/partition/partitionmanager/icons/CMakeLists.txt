# collect the needed icon files
set (PARTITIONMANAGER_ICONS
   hi16-apps-partitionmanager.png
   hi22-apps-partitionmanager.png
   hi32-apps-partitionmanager.png
   hi48-apps-partitionmanager.png
   hi64-apps-partitionmanager.png
   hi128-apps-partitionmanager.png
)
ecm_install_icons(ICONS ${PARTITIONMANAGER_ICONS} DESTINATION ${ICON_INSTALL_DIR})
