/*  Copyright 1996-1999,2001,2002,2005,2006,2008,2009 Alain Knaff.
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
 */

#ifndef MTOOLS_STREAM_H
#define MTOOLS_STREAM_H

#include <time.h>
#include <sys/types.h>

struct doscp_t;
union bootsector;
struct device;

struct Stream_t
{
	struct Class_t *Class;
	int refs;
	struct Stream_t *Next;
	struct Stream_t *Buffer;
};

struct doscp_t *get_dosConvert_pass_through(struct Stream_t *Stream);

struct Class_t
{
	int (*read)(struct Stream_t *, char *, off_t, size_t);
	int (*write)(struct Stream_t *, char *, off_t, size_t);
	int (*flush)(struct Stream_t *);
	int (*freeFunc)(struct Stream_t *);
	int (*set_geom)(struct Stream_t *, struct device *, struct device *, int media, union bootsector *);
	int (*get_data)(struct Stream_t *, time_t *, size_t *, int *, int *);
	int (*pre_allocate)(struct Stream_t *, size_t);
	struct doscp_t *(*get_dosConvert)(struct Stream_t *);
};

#define READS(stream, buf, address, size) \
((stream)->Class->read)( (stream), (char *) (buf), (address), (size) )

#define WRITES(stream, buf, address, size) \
((stream)->Class->write)( (stream), (char *) (buf), (address), (size) )

#define SET_GEOM(stream, dev, orig_dev, media, boot) \
(stream)->Class->set_geom( (stream), (dev), (orig_dev), (media), (boot) )

#define GET_DATA(stream, date, size, type, address) \
(stream)->Class->get_data( (stream), (date), (size), (type), (address) )

#define PRE_ALLOCATE(stream, size) \
(stream)->Class->pre_allocate((stream), (size))

#define GET_DOSCONVERT(stream)			\
	(stream)->Class->get_dosConvert((stream))

int flush_stream(struct Stream_t *Stream);
struct Stream_t *copy_stream(struct Stream_t *Stream);
int free_stream(struct Stream_t **Stream);

#define DeclareThis(x) x *This = (x *) Stream

int get_data_pass_through(struct Stream_t *Stream, time_t *date, size_t *size, int *type, int *address);
int read_pass_through(struct Stream_t *Stream, char *buf, off_t start, size_t len);
int write_pass_through(struct Stream_t *Stream, char *buf, off_t start, size_t len);

#endif

