/*  Copyright 1997,2001-2003 Alain Knaff.
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
 * Buffer read/write module
 */

#include "msdos.h"
#include "mtools.h"
#include "buffer.h"
#include "force_io.h"
#include "stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROUND_DOWN(value, grain) ((value) - (value) % (grain))
#define ROUND_UP(value, grain) ROUND_DOWN((value) + (grain)-1, (grain))

struct Buffer_t
{
	struct Class_t *Class;
	int refs;
	struct Stream_t *Next;
	struct Stream_t *Buffer;

	size_t size;     	/* size of read/write buffer */
	int dirty;	       	/* is the buffer dirty? */

	size_t sectorSize;	/* sector size: all operations happen in multiples of this */
	size_t cylinderSize;	/* cylinder size: preferred alignemnt, but for efficiency, less data may be read */
	int ever_dirty;	       	/* was the buffer ever dirty? */
	size_t dirty_pos;
	size_t dirty_end;
	off_t current;		/* first sector in buffer */
	size_t cur_size;		/* the current size */
	char *buf;		/* disk read/write buffer */
};

/*
 * Flush a dirty buffer to disk.  Resets Buffer->dirty to zero.
 * All errors are fatal.
 */

static int _buf_flush(struct Buffer_t *Buffer)
{
	if (!Buffer->Next || !Buffer->dirty)
		return 0;

	if(Buffer->current < 0L)
	{
		fprintf(stderr,"Should not happen\n");
		return -1;
	}

	int ret = force_write(Buffer->Next, Buffer->buf + Buffer->dirty_pos, Buffer->current + Buffer->dirty_pos, Buffer->dirty_end - Buffer->dirty_pos);

	if (ret != (signed int) (Buffer->dirty_end - Buffer->dirty_pos))
	{
		if (ret < 0)
			perror("buffer_flush: write");
		else
			fprintf(stderr,"buffer_flush: short write\n");
		return -1;
	}

	Buffer->dirty = 0;
	Buffer->dirty_end = 0;
	Buffer->dirty_pos = 0;

	return 0;
}

static int invalidate_buffer(struct Buffer_t *Buffer, off_t start)
{
	if(_buf_flush(Buffer) < 0)
		return -1;

	/* start reading at the beginning of start's sector
	 * don't start reading too early, or we might not even reach
	 * start */
	Buffer->current = ROUND_DOWN(start, Buffer->sectorSize);
	Buffer->cur_size = 0;
	return 0;
}

#undef OFFSET
#define OFFSET (start - This->current)

typedef enum position_t
{
	OUTSIDE,
	APPEND,
	INSIDE,
	ERROR
} position_t;

static position_t isInBuffer(struct Buffer_t *This, off_t start, size_t *len)
{
	if(start >= This->current && start < This->current + (off_t)This->cur_size)
	{
		maximize(*len, This->cur_size - OFFSET);
		return INSIDE;
	}
	else if(start == This->current + (off_t)This->cur_size && This->cur_size < This->size && *len >= This->sectorSize)
	{
		/* append to the buffer for this, three conditions have to
		 * be met:
		 *  1. The start falls exactly at the end of the currently
		 *     loaded data
		 *  2. There is still space
		 *  3. We append at least one sector
		 */
		maximize(*len, This->size - This->cur_size);
		*len = ROUND_DOWN(*len, This->sectorSize);
		return APPEND;
	}
	else
	{
		if(invalidate_buffer(This, start) < 0)
			return ERROR;

		maximize(*len, This->cylinderSize - OFFSET);
		maximize(*len, This->cylinderSize - This->current % This->cylinderSize);

		return OUTSIDE;
	}
}

static int buf_read(struct Stream_t *Stream, char *buf, off_t start, size_t len)
{
	DeclareThis(struct Buffer_t);

	if(!len)
		return 0;

	switch(isInBuffer(This, start, &len))
	{
		case OUTSIDE:
		case APPEND:
		{
			/* always load until the end of the cylinder */
			size_t length = This->cylinderSize - (This->current + This->cur_size) % This->cylinderSize;
			maximize(length, This->size - This->cur_size);

			/* read it! */
			int ret = READS(This->Next, This->buf + This->cur_size, This->current + This->cur_size, length);

			if (ret < 0)
				return ret;

			This->cur_size += ret;

			if (This->current + (off_t)This->cur_size < start)
			{
				fprintf(stderr, "Short buffer fill\n");
				return -1;
			}
			break;
		}

		case INSIDE:
			/* nothing to do */
			break;

		case ERROR:
			return -1;
	}

	int offset = OFFSET;
	char* disk_ptr = This->buf + offset;
	maximize(len, This->cur_size - offset);
	memcpy(buf, disk_ptr, len);

	return len;
}

static int buf_write(struct Stream_t *Stream, char *buf, off_t start, size_t len)
{
	DeclareThis(struct Buffer_t);
	size_t offset;

	if(!len)
		return 0;

	This->ever_dirty = 1;

	switch(isInBuffer(This, start, &len))
	{
		case OUTSIDE:
			if(start % This->cylinderSize || len < This->sectorSize)
			{
				size_t readSize;
				int ret;

				readSize = This->cylinderSize - This->current % This->cylinderSize;

				ret = READS(This->Next, This->buf, This->current, readSize);
				/* read it! */
				if (ret < 0)
					return ret;

				if(ret % This->sectorSize)
				{
					fprintf(stderr, "Weird: read size (%d) not a multiple of sector size (%d)\n", ret, (int) This->sectorSize);

				    ret -= ret % This->sectorSize;
					if(ret == 0)
					{
						fprintf(stderr, "Nothing left\n");
						return -1; // TODO: check in caller
				    }
				}

				This->cur_size = ret;

				/* for dosemu. Autoextend size */
				if(!This->cur_size)
				{
					memset(This->buf,0,readSize);
					This->cur_size = readSize;
				}
				offset = OFFSET;
				break;
			}
			/* FALL THROUGH */

		case APPEND:
			len = ROUND_DOWN(len, This->sectorSize);
			offset = OFFSET;
			maximize(len, This->size - offset);
			This->cur_size += len;
			if(This->Next->Class->pre_allocate)
				PRE_ALLOCATE(This->Next,
							 This->current + This->cur_size);
			break;

		case INSIDE:
			/* nothing to do */
			offset = OFFSET;
			maximize(len, This->cur_size - offset);
			break;

		case ERROR:
			return -1;

		default:
			return -1; // TODO: check in caller
	}

	char* disk_ptr = This->buf + offset;

	/* extend if we write beyond end */
	if(offset + len > This->cur_size) {
		len -= (offset + len) % This->sectorSize;
		This->cur_size = len + offset;
	}

	memcpy(disk_ptr, buf, len);

	if(!This->dirty || offset < This->dirty_pos)
		This->dirty_pos = ROUND_DOWN(offset, This->sectorSize);

	if(!This->dirty || offset + len > This->dirty_end)
		This->dirty_end = ROUND_UP(offset + len, This->sectorSize);

	if(This->dirty_end > This->cur_size)
	{
		fprintf(stderr, "Internal error, dirty end too big dirty_end=%x cur_size=%x len=%x offset=%d sectorSize=%x\n",
				(unsigned int) This->dirty_end,
				(unsigned int) This->cur_size,
				(unsigned int) len,
				(int) offset, (int) This->sectorSize);
		fprintf(stderr, "offset + len + grain - 1 = %x\n", (int) (offset + len + This->sectorSize - 1));
		fprintf(stderr, "ROUNDOWN(offset + len + grain - 1) = %x\n", (int)ROUND_DOWN(offset + len + This->sectorSize - 1, This->sectorSize));
		fprintf(stderr, "This->dirty = %d\n", This->dirty);
		return -1; // TODO: check in caller
	}

	This->dirty = 1;

	return len;
}

static int buf_flush(struct Stream_t *Stream)
{
	int ret;
	DeclareThis(struct Buffer_t);

	if (!This->ever_dirty)
		return 0;
	ret = _buf_flush(This);
	if(ret == 0)
		This->ever_dirty = 0;
	return ret;
}


static int buf_free(struct Stream_t *Stream)
{
	DeclareThis(struct Buffer_t);

	if(This->buf)
		free(This->buf);
	This->buf = 0;
	return 0;
}

static struct Class_t BufferClass =
{
	buf_read,
	buf_write,
	buf_flush,
	buf_free,
	0, /* set_geom */
	get_data_pass_through, /* get_data */
	0, /* pre-allocate */
	get_dosConvert_pass_through /* dos convert */
};

struct Stream_t *buf_init(struct Stream_t *Next, int size, int cylinderSize, int sectorSize)
{
	struct Buffer_t *Buffer;
	struct Stream_t *Stream;

	if(size % cylinderSize != 0)
	{
		fprintf(stderr, "size not multiple of cylinder size\n");
		return NULL; // TODO: check in caller
	}

	if(cylinderSize % sectorSize != 0)
	{
		fprintf(stderr, "cylinder size not multiple of sector size\n");
		return NULL; // TODO: check in caller
	}

	if(Next->Buffer)
	{
		Next->refs--;
		Next->Buffer->refs++;
		return Next->Buffer;
	}

	Stream = (struct Stream_t *) malloc (sizeof(struct Buffer_t));

	if(!Stream)
		return 0;

	Buffer = (struct Buffer_t *) Stream;
	Buffer->buf = malloc(size);

	if (!Buffer->buf)
	{
		free(Stream);
		return 0;
	}

	Buffer->size = size;
	Buffer->dirty = 0;
	Buffer->cylinderSize = cylinderSize;
	Buffer->sectorSize = sectorSize;

	Buffer->ever_dirty = 0;
	Buffer->dirty_pos = 0;
	Buffer->dirty_end = 0;
	Buffer->current = 0L;
	Buffer->cur_size = 0; /* buffer currently empty */

	Buffer->Next = Next;
	Buffer->Class = &BufferClass;
	Buffer->refs = 1;
	Buffer->Buffer = 0;
	Buffer->Next->Buffer = (struct Stream_t *) Buffer;

	return Stream;
}

