import parted

#This creates a list of all devices
device_list = parted.getAllDevices()
for dev in device_list:
    #AFAIK isbusy = in use/mounted.  Needs to flag if 'yes' to prompt user to umount
    isbusy = dev.busy
    #path gives /dev/sda or something similar
    myname = dev.path
    #Hard drives measure themselves assuming kilo=1000, mega=1mil, etc
    limiter = 1000
    #Some disk size calculations
    byte_size = dev.length * dev.sectorSize
    megabyte_size = byte_size / (limiter * limiter)
    gigabyte_size = megabyte_size / limiter
    print(byte_size)
    #Must create disk object to drill down
    diskob = parted.Disk(dev)
    #Do not let user specify more than this number of primary partitions
    disk_max_pri = diskob.maxPrimaryPartitionCount
    #create list of partitions for this device(/dev/sda for example)
    partition_list = diskob.partitions
    for p in partition_list:
        #this is start sector, end sector, and length
        startbyte = p.geometry.start
        endbyte = p.geometry.end
        plength = p.geometry.length
        #lets calcule its size in something ppl understand
        psize = plength * dev.sectorSize
        #just calculating it in more sane formats
        #should probably add in something like
        #if psizemb < 1000 then display as MB, else as GB
        #I can't think of a case of less than 1mb partition
        psizemb = psize / (limiter * limiter)
        psizegb = psizemb / limiter
        #grabs the filesystem type
        ptype = p.fileSystem.type

