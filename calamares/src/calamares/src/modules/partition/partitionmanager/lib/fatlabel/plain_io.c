/*  Copyright 1995-2007,2009 Alain Knaff.
 *  Copyright 2010 Volker Lanz (vl@fidra.de)
 *  This file is part of mtools.
 *
 *  Mtools is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mtools is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mtools.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Io to a plain file or device
 *
 * written by:
 *
 * Alain L. Knaff
 * alain@knaff.lu
 *
 */

#include "stream.h"
#include "mtools.h"
#include "msdos.h"
#include "plain_io.h"
#include "partition.h"
#include "llong.h"
#include "force_io.h"
#include "devices.h"

#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct SimpleFile_t
{
	struct Class_t *Class;
	int refs;
	struct Stream_t *Next;
	struct Stream_t *Buffer;
	struct stat64 statbuf;
	int fd;
	off_t offset;
	off_t lastwhere;
	int seekable;
	int privileged;
	int scsi_sector_size;
	void *extra_data; /* extra system dependant information for scsi */
	int swap; /* do the word swapping */
};

static int lock_dev(int fd, int mode)
{
	if (flock(fd, (mode ? LOCK_EX : LOCK_SH) | LOCK_NB) < 0)
		return errno == EINVAL || errno ==  EOPNOTSUPP ? 0 : 1;

	return 0;
}

typedef int (*iofn) (int, char *, int);

static void swap_buffer(char *buf, size_t len)
{
	unsigned int i;

	for (i = 0; i<len; i+=2)
	{
		char temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}
}

static int file_io(struct Stream_t *Stream, char *buf, off_t where, int len, iofn io)
{
	DeclareThis(struct SimpleFile_t);

	where += This->offset;

	if (This->seekable && where != This->lastwhere )
	{
		if(mt_lseek( This->fd, where, SEEK_SET) < 0 )
		{
			perror("seek");
			This->lastwhere = (off_t) -1;
			return -1;
		}
	}

	int ret = io(This->fd, buf, len);

	if (ret == -1 )
	{
		perror("plain_io");
		This->lastwhere = (off_t) -1;
		return -1;
	}

	This->lastwhere = where + ret;

	return ret;
}

static int file_read(struct Stream_t *Stream, char *buf, off_t where, size_t len)
{
	DeclareThis(struct SimpleFile_t);

	int result = file_io(Stream, buf, where, len, (iofn) read);

	if (This->swap)
		swap_buffer(buf, len);

	return result;
}

static int file_write(struct Stream_t *Stream, char *buf, off_t where, size_t len)
{
	DeclareThis(struct SimpleFile_t);

	if (!This->swap)
		return file_io(Stream, buf, where, len, (iofn) write);
	else
	{
		char* swapping = malloc(len);

		memcpy(swapping, buf, len);
		swap_buffer(swapping, len);

		int result = file_io(Stream, swapping, where, len, (iofn) write);

		free(swapping);
		return result;
	}
}

static int file_flush(struct Stream_t UNUSED(*Stream))
{
	return 0;
}

static int file_free(struct Stream_t *Stream)
{
	DeclareThis(struct SimpleFile_t);
	return (This->fd > 2) ? close(This->fd) : 0;
}

static int file_geom(struct Stream_t *Stream, struct device *dev, struct device *orig_dev, int media, union bootsector *boot)
{
	DeclareThis(struct SimpleFile_t);

	dev->ssize = 2; /* allow for init_geom to change it */
	dev->use_2m = 0x80; /* disable 2m mode to begin */

	if(media == 0xf0 || media >= 0x100)
	{
		dev->heads = WORD(nheads);
		dev->sectors = WORD(nsect);

		size_t tot_sectors = DWORD(bigsect);

		if (WORD(psect))
			tot_sectors = WORD(psect);

		int sect_per_track = dev->heads * dev->sectors;

		if(sect_per_track == 0)
		{
			/* add some fake values if sect_per_track is
			 * zero. Indeed, some atari disks lack the
			 * geometry values (i.e. have zeroes in their
			 * place). In order to avoid division by zero
			 * errors later on, plug 1 everywhere
			 */
			dev->heads = 1;
			dev->sectors = 1;
			sect_per_track = 1;
		}

		tot_sectors += sect_per_track - 1; /* round size up */
		dev->tracks = tot_sectors / sect_per_track;
	}
	else
	{
		fprintf(stderr,"Unknown media type\n");
		return -1; // TODO: make sure this is interpreted as invalid return code
	}

	int sectors = dev->sectors;
	dev->sectors = dev->sectors * WORD(secsiz) / 512;

	int ret = init_geom(This->fd,dev, orig_dev, &This->statbuf);
	dev->sectors = sectors;

	return ret;
}


static int file_data(struct Stream_t *Stream, time_t *date, size_t *size,
		     int *type, int *address)
{
	DeclareThis(struct SimpleFile_t);

	if(date)
		*date = This->statbuf.st_mtime;

	if(size)
		*size = This->statbuf.st_size;

	if(type)
		*type = S_ISDIR(This->statbuf.st_mode);

	if(address)
		*address = 0;

	return 0;
}

static struct Class_t SimpleFileClass =
{
	file_read,
	file_write,
	file_flush,
	file_free,
	file_geom,
	file_data,
	0, /* pre_allocate */
	0
};


struct Stream_t *SimpleFileOpen(struct device *dev, struct device *orig_dev,
			const char *name, int mode, char *errmsg,
			int mode2, int locked, size_t *maxSize)
{
	struct SimpleFile_t *This;

	This = New(struct SimpleFile_t);

	if (!This)
	{
		fprintf(stderr, "%s %d: Allocation memory for simple file failed.\n", __FILE__, __LINE__);
		return 0;
	}

	This->scsi_sector_size = 512;
	This->seekable = 1;

	This->Class = &SimpleFileClass;
	if (!name || strcmp(name,"-") == 0)
	{
		This->fd = mode == O_RDONLY ? 0 : 1;
		This->seekable = 0;
		This->refs = 1;
		This->Next = 0;
		This->Buffer = 0;

		if (fstat64(This->fd, &This->statbuf) < 0)
		{
		    free(This);

		    if(errmsg)
				snprintf(errmsg,199,"Can't stat -: %s", strerror(errno));

		    return NULL;
		}

		return (struct Stream_t *) This;
	}

	if(dev)
		mode |= dev->mode;

    This->fd = open(name, mode | O_LARGEFILE, 0666);

	if (This->fd < 0)
	{
		free(This);

		if(errmsg)
			snprintf(errmsg, 199, "Can't open %s: %s", name, strerror(errno));

		return NULL;
	}

	if (fstat64(This->fd, &This->statbuf) < 0)
	{
		free(This);

		if(errmsg)
			snprintf(errmsg, 199,"Can't stat %s: %s", name, strerror(errno));

		return NULL;
	}

	/* lock the device on writes */
	if (locked && lock_dev(This->fd, mode == O_RDWR))
	{
		if(errmsg)
			snprintf(errmsg, 199, "plain floppy: device \"%s\" busy (%s):", dev ? dev->name : "unknown", strerror(errno));
		close(This->fd);
		free(This);

		return NULL;
	}

	/* set default parameters, if needed */
	if (dev)
	{
		if (dev->tracks && init_geom(This->fd, dev, orig_dev, &This->statbuf))
		{
			close(This->fd);
			free(This);

			if(errmsg)
				sprintf(errmsg,"init: set default params");

			return NULL;
		}

		This->offset = (off_t) dev->offset;
	}
	else
		This->offset = 0;

	This->refs = 1;
	This->Next = 0;
	This->Buffer = 0;

	if(maxSize)
	{
		*maxSize = max_off_t_seek;

		if(This->offset > 0 && (size_t) This->offset > *maxSize)
		{
			close(This->fd);
			free(This);

			if(errmsg)
				sprintf(errmsg,"init: Big disks not supported");

			return NULL;
		}

		*maxSize -= This->offset;
	}
	/* partitioned drive */

	This->swap = 0;

	if(!(mode2 & NO_OFFSET) && dev && dev->partition > 4)
	    fprintf(stderr, "Invalid partition %d (must be between 0 and 4), ignoring it\n", dev->partition);

	while(!(mode2 & NO_OFFSET) && dev && dev->partition && dev->partition <= 4)
	{
		unsigned char buf[2048];
		struct partition *partTable = (struct partition *)(buf+ 0x1ae);
		size_t partOff;

		/* read the first sector, or part of it */
		if (force_read((struct Stream_t *)This, (char*) buf, 0, 512) != 512)
			break;

		if( _WORD(buf + 510) != 0xaa55)
			break;

		partOff = BEGIN(partTable[dev->partition]);
		if (maxSize)
		{
			if (partOff > *maxSize >> 9)
			{
				close(This->fd);
				free(This);

				if(errmsg)
					sprintf(errmsg,"init: Big disks not supported");

				return NULL;
			}

			*maxSize -= (off_t) partOff << 9;
		}

		This->offset += (off_t) partOff << 9;
		if(!partTable[dev->partition].sys_ind)
		{
			if(errmsg)
				sprintf(errmsg, "init: non-existent partition");

			close(This->fd);
			free(This);

			return NULL;
		}

		if(!dev->tracks)
		{
			dev->heads = head(partTable[dev->partition].end) + 1;
			dev->sectors = sector(partTable[dev->partition].end);
			dev->tracks = cyl(partTable[dev->partition].end) - cyl(partTable[dev->partition].start) + 1;
		}

		dev->hidden = dev->sectors*head(partTable[dev->partition].start) + sector(partTable[dev->partition].start) - 1;
		break;
	}

	This->lastwhere = -This->offset;
	/* provoke a seek on those devices that don't start on a partition
	 * boundary */

	return (struct Stream_t *) This;
}

int SimpleFileClose(struct Stream_t* Stream)
{
	DeclareThis(struct SimpleFile_t);
	return close(This->fd);
}
