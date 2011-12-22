/* VIPS function dispatch tables for image file load/save.
 */

/*

    This file is part of VIPS.

    VIPS is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/*

    These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/
#include <vips/intl.h>

#include <stdio.h>

#include <vips/vips.h>
#include <vips/internal.h>
#include <vips/debug.h>

/**
 * SECTION: foreign
 * @short_description: load and save images in a variety of formats
 * @stability: Stable
 * @see_also: <link linkend="libvips-image">image</link>
 * @include: vips/vips.h
 *
 * This set of operations load and save images in a variety of formats. 
 *
 * The operations share a base class that offers a simple way to search for a
 * subclass of #VipsForeign which can load a certain file (see
 * vips_foreign_find_load()) or which could be used to save an image to a
 * certain file type (see vips_foreign_find_save()). You can then run these
 * operations using vips_call() and friends to perform the load or save.
 *
 * A pair of convenience
 * functions, vips_foreign_load() and vips_foreign_save(), automate the
 * process, loading an image from a file or saving an image to a file. These
 * functions let you give load or save options as name - value pairs in the C
 * argument list. You can use vips_foreign_load_options() and
 * vips_foreign_save_options() to include option in the file name.
 *
 * For example:
 *
 * |[
 * vips_foreign_save (my_image, "frank.tiff", 
 *     "compression", VIPS_FOREIGN_TIFF_COMPRESSION_JPEG,
 *     NULL);
 * ]|
 *
 * Will save an image to the file "frank.tiff" in TIFF format (selected by
 * the file name extension) with JPEG compression.
 *
 * You can also invoke the operations directly, for example:
 *
 * |[
 * vips_tiffsave (my_image, "frank.anything", 
 *     "compression", VIPS_FOREIGN_TIFF_COMPRESSION_JPEG,
 *     NULL);
 * ]|
 *
 * To add support for a new file format to vips, simply define a new subclass
 * of #VipsForeignLoad or #VipsForeignSave. 
 *
 * If you define a new operation which is a subclass of #VipsForeign, support 
 * for it automatically appears in all VIPS user-interfaces. It will also be
 * transparently supported by vips_image_new_from_file() and friends.
 *
 * VIPS comes with VipsForeign for TIFF, JPEG, PNG, Analyze, PPM, OpenEXR, CSV,
 * Matlab, Radiance, RAW, FITS and VIPS. It also 
 * includes import filters which can
 * load with libMagick and with OpenSlide. 
 */

/**
 * VipsForeignFlags: 
 * @VIPS_FOREIGN_NONE: no flags set
 * @VIPS_FOREIGN_PARTIAL: the image may be read lazilly
 * @VIPS_FOREIGN_BIGENDIAN: image pixels are most-significant byte first
 *
 * Some hints about the image loader.
 *
 * @VIPS_FOREIGN_PARTIAL means that the image can be read directly from the
 * file without needing to be unpacked to a temporary image first. 
 *
 * @VIPS_FOREIGN_BIGENDIAN means that image pixels are most-significant byte
 * first. Depending on the native byte order of the host machine, you may
 * need to swap bytes. See copy_swap().
 */

/**
 * VipsForeignClass:
 *
 * The suffix list is used to select a format to save a file in, and to pick a
 * loader if you don't define is_a().
 *
 * You should also define @nickname and @description in #VipsObject. 
 */

/**
 * VipsForeignLoad:
 *
 * @out must be set by @header(). @load(), if defined, must set @real.
 */

/**
 * VipsForeignLoadClass:
 *
 * Add a new loader to VIPS by subclassing #VipsForeignLoad. Subclasses need to 
 * implement at least @header().
 *
 * As a complete example, here's the code for the PNG loader, minus the actual
 * calls to libpng.
 *
 * |[
typedef struct _VipsForeignLoadPng {
	VipsForeignLoad parent_object;

	char *filename; 
} VipsForeignLoadPng;

typedef VipsForeignLoadClass VipsForeignLoadPngClass;

G_DEFINE_TYPE( VipsForeignLoadPng, vips_foreign_load_png, 
	VIPS_TYPE_FOREIGN_LOAD );

static VipsForeignFlags
vips_foreign_load_png_get_flags_filename( const char *filename )
{
	return( 0 );
}

static VipsForeignFlags
vips_foreign_load_png_get_flags( VipsForeignLoad *load )
{
	VipsForeignLoadPng *png = (VipsForeignLoadPng *) load;

	return( vips_foreign_load_png_get_flags_filename( png->filename ) );
}

static int
vips_foreign_load_png_header( VipsForeignLoad *load )
{
	VipsForeignLoadPng *png = (VipsForeignLoadPng *) load;

	if( vips__png_header( png->filename, load->out ) )
		return( -1 );

	return( 0 );
}

static int
vips_foreign_load_png_load( VipsForeignLoad *load )
{
	VipsForeignLoadPng *png = (VipsForeignLoadPng *) load;

	if( vips__png_read( png->filename, load->real ) )
		return( -1 );

	return( 0 );
}

static void
vips_foreign_load_png_class_init( VipsForeignLoadPngClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;
	VipsForeignClass *foreign_class = (VipsForeignClass *) class;
	VipsForeignLoadClass *load_class = (VipsForeignLoadClass *) class;

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->nickname = "pngload";
	object_class->description = _( "load png from file" );

	foreign_class->suffs = vips__png_suffs;

	load_class->is_a = vips__png_ispng;
	load_class->get_flags_filename = 
		vips_foreign_load_png_get_flags_filename;
	load_class->get_flags = vips_foreign_load_png_get_flags;
	load_class->header = vips_foreign_load_png_header;
	load_class->load = vips_foreign_load_png_load;

	VIPS_ARG_STRING( class, "filename", 1, 
		_( "Filename" ),
		_( "Filename to load from" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsForeignLoadPng, filename ),
		NULL );
}

static void
vips_foreign_load_png_init( VipsForeignLoadPng *png )
{
}
 * ]|
 */

/**
 * VipsForeignSaveClass:
 *
 * Call your saver in the class' @build() method after chaining up. The
 * prepared image should be ready for you to save in @ready.  
 *
 * As a complete example, here's the code for the CSV saver, minus the calls
 * to the actual save routines.
 *
 * |[
typedef struct _VipsForeignSaveCsv {
	VipsForeignSave parent_object;

	char *filename; 
	const char *separator;
} VipsForeignSaveCsv;

typedef VipsForeignSaveClass VipsForeignSaveCsvClass;

G_DEFINE_TYPE( VipsForeignSaveCsv, vips_foreign_save_csv, 
	VIPS_TYPE_FOREIGN_SAVE );

static int
vips_foreign_save_csv_build( VipsObject *object )
{
	VipsForeignSave *save = (VipsForeignSave *) object;
	VipsForeignSaveCsv *csv = (VipsForeignSaveCsv *) object;

	if( VIPS_OBJECT_CLASS( vips_foreign_save_csv_parent_class )->
		build( object ) )
		return( -1 );

	if( vips__csv_write( save->ready, csv->filename, csv->separator ) )
		return( -1 );

	return( 0 );
}

#define UC VIPS_FORMAT_UCHAR
#define C VIPS_FORMAT_CHAR
#define US VIPS_FORMAT_USHORT
#define S VIPS_FORMAT_SHORT
#define UI VIPS_FORMAT_UINT
#define I VIPS_FORMAT_INT
#define F VIPS_FORMAT_FLOAT
#define X VIPS_FORMAT_COMPLEX
#define D VIPS_FORMAT_DOUBLE
#define DX VIPS_FORMAT_DPCOMPLEX

static int bandfmt_csv[10] = {
// UC  C   US  S   UI  I  F  X  D  DX 
   UC, C,  US, S,  UI, I, F, X, D, DX
};

static void
vips_foreign_save_csv_class_init( VipsForeignSaveCsvClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;
	VipsForeignClass *foreign_class = (VipsForeignClass *) class;
	VipsForeignSaveClass *save_class = (VipsForeignSaveClass *) class;

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->nickname = "csvsave";
	object_class->description = _( "save image to csv file" );
	object_class->build = vips_foreign_save_csv_build;

	foreign_class->suffs = vips__foreign_csv_suffs;

	save_class->saveable = VIPS_SAVEABLE_MONO;
	save_class->format_table = bandfmt_csv;

	VIPS_ARG_STRING( class, "filename", 1, 
		_( "Filename" ),
		_( "Filename to save to" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsForeignSaveCsv, filename ),
		NULL );

	VIPS_ARG_STRING( class, "separator", 13, 
		_( "Separator" ), 
		_( "Separator characters" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( VipsForeignSaveCsv, separator ),
		"\t" ); 
}

static void
vips_foreign_save_csv_init( VipsForeignSaveCsv *csv )
{
	csv->separator = g_strdup( "\t" );
}
 * ]|
 */

G_DEFINE_ABSTRACT_TYPE( VipsForeign, vips_foreign, VIPS_TYPE_OPERATION );

static void
vips_foreign_print_class( VipsObjectClass *object_class, VipsBuf *buf )
{
	VipsForeignClass *class = VIPS_FOREIGN_CLASS( object_class );
	const char **p;

	VIPS_OBJECT_CLASS( vips_foreign_parent_class )->
		print_class( object_class, buf );
	vips_buf_appends( buf, " " );

	if( class->suffs ) {
		vips_buf_appends( buf, "(" );
		for( p = class->suffs; *p; p++ ) {
			vips_buf_appendf( buf, "%s", *p );
			if( p[1] )
				vips_buf_appends( buf, ", " );
		}
		vips_buf_appends( buf, "), " );
	}

	vips_buf_appendf( buf, "priority=%d", class->priority );

}

static void
vips_foreign_class_init( VipsForeignClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->nickname = "file";
	object_class->description = _( "load and save image files" );
	object_class->print_class = vips_foreign_print_class;
}

static void
vips_foreign_init( VipsForeign *object )
{
}

/* To iterate over supported files we build a temp list of subclasses of 
 * VipsForeign, sort by priority, iterate, and free.
 */

static void *
file_add_class( VipsForeignClass *file, GSList **files )
{
	/* Append so we don't reverse the list of files.
	 */
	*files = g_slist_append( *files, file );

	return( NULL );
}

static gint
file_compare( VipsForeignClass *a, VipsForeignClass *b )
{
        return( b->priority - a->priority );
}

/**
 * vips_foreign_map:
 * @base: base class to search below (eg. "VipsForeignLoad")
 * @fn: function to apply to each #VipsForeignClass
 * @a: user data
 * @b: user data
 *
 * Apply a function to every #VipsForeignClass that VIPS knows about. Foreigns
 * are presented to the function in priority order. 
 *
 * Like all VIPS map functions, if @fn returns %NULL, iteration continues. If
 * it returns non-%NULL, iteration terminates and that value is returned. The
 * map function returns %NULL if all calls return %NULL.
 *
 * See also: vips_slist_map().
 *
 * Returns: the result of iteration
 */
void *
vips_foreign_map( const char *base, VipsSListMap2Fn fn, void *a, void *b )
{
	GSList *files;
	void *result;

	files = NULL;
	(void) vips_class_map_all( g_type_from_name( base ), 
		(VipsClassMapFn) file_add_class, (void *) &files );

	files = g_slist_sort( files, (GCompareFunc) file_compare );
	result = vips_slist_map2( files, fn, a, b );
	g_slist_free( files );

	return( result );
}

/* Abstract base class for image load.
 */

G_DEFINE_ABSTRACT_TYPE( VipsForeignLoad, vips_foreign_load, VIPS_TYPE_FOREIGN );

static void
vips_foreign_load_dispose( GObject *gobject )
{
	VipsForeignLoad *load = VIPS_FOREIGN_LOAD( gobject );

	VIPS_UNREF( load->real );

	G_OBJECT_CLASS( vips_foreign_load_parent_class )->dispose( gobject );
}

static void
vips_foreign_load_print_class( VipsObjectClass *object_class, VipsBuf *buf )
{
	VipsForeignLoadClass *class = VIPS_FOREIGN_LOAD_CLASS( object_class );

	VIPS_OBJECT_CLASS( vips_foreign_load_parent_class )->
		print_class( object_class, buf );

	if( !G_TYPE_IS_ABSTRACT( G_TYPE_FROM_CLASS( class ) ) ) {
		if( class->is_a )
			vips_buf_appends( buf, ", is_a" );
		if( class->get_flags )
			vips_buf_appends( buf, ", get_flags" );
		if( class->get_flags_filename )
			vips_buf_appends( buf, ", get_flags_filename" );
		if( class->header )
			vips_buf_appends( buf, ", header" );
		if( class->load )
			vips_buf_appends( buf, ", load" );

		/* Sanity: it's OK to have get_flags() and 
		 * get_flags_filename() both NULL or both not-NULL.
		 */
		g_assert( !class->get_flags == !class->get_flags_filename );

		/* You can omit ->load(), you must not omit ->header().
		 */
		g_assert( class->header );
	}
}

/* Can this VipsForeign open this file?
 */
static void *
vips_foreign_load_new_from_foreign_sub( VipsForeignLoadClass *load_class, 
	const char *filename )
{
	VipsForeignClass *class = VIPS_FOREIGN_CLASS( load_class );

	if( load_class->is_a &&
		load_class->is_a( filename ) ) 
		return( load_class );
	else if( class->suffs && 
		vips_filename_suffix_match( filename, class->suffs ) )
		return( load_class );

	return( NULL );
}

/**
 * vips_foreign_find_load:
 * @filename: file to find a file for
 *
 * Searches for an operation you could use to load a file. 
 *
 * See also: vips_foreign_read().
 *
 * Returns: the nmae of an operation on success, %NULL on error
 */
const char *
vips_foreign_find_load( const char *filename )
{
	VipsForeignLoadClass *load_class;

	if( !vips_existsf( "%s", filename ) ) {
		vips_error( "VipsForeignLoad", 
			_( "file \"%s\" not found" ), filename );
		return( NULL );
	}

	if( !(load_class = (VipsForeignLoadClass *) vips_foreign_map( 
		"VipsForeignLoad",
		(VipsSListMap2Fn) vips_foreign_load_new_from_foreign_sub, 
		(void *) filename, NULL )) ) {
		vips_error( "VipsForeignLoad", 
			_( "\"%s\" not a known file format" ), filename );
		return( NULL );
	}

	return( G_OBJECT_CLASS_NAME( load_class ) );
}

/**
 * vips_foreign_is_a:
 * @loader: name of loader to use for test
 * @filename: file to test
 *
 * Return %TRUE if @filename can be loaded by @loader. @loader is something
 * like "tiffload" or "VipsForeignLoadTiff".
 *
 * Returns: %TRUE if @filename can be loaded by @loader.
 */
gboolean 
vips_foreign_is_a( const char *loader, const char *filename )
{
	VipsObjectClass *class;
	VipsForeignLoadClass *load_class;

	if( !(class = vips_class_find( "VipsForeignLoad", loader )) ) 
		return( FALSE );
	load_class = VIPS_FOREIGN_LOAD_CLASS( class );
	if( load_class->is_a &&
		load_class->is_a( filename ) ) 
		return( TRUE );

	return( FALSE );
}

/**
 * vips_foreign_flags:
 * @loader: name of loader to use for test
 * @filename: file to test
 *
 * Return the flags for @filename using @loader. 
 * @loader is something like "tiffload" or "VipsForeignLoadTiff".
 *
 * Returns: the flags for @filename.
 */
VipsForeignFlags 
vips_foreign_flags( const char *loader, const char *filename )
{
	VipsObjectClass *class;

	if( (class = vips_class_find( "VipsForeignLoad", loader )) ) {
		VipsForeignLoadClass *load_class = 
			VIPS_FOREIGN_LOAD_CLASS( class );

		if( load_class->get_flags_filename ) 
			return( load_class->get_flags_filename( filename ) );
	}

	return( 0 );
}

static VipsObject *
vips_foreign_load_new_from_string( const char *string )
{
	const char *file_op;
	GType type;
	VipsForeignLoad *load;

	if( !(file_op = vips_foreign_find_load( string )) )
		return( NULL );
	type = g_type_from_name( file_op );
	g_assert( type ); 

	load = VIPS_FOREIGN_LOAD( g_object_new( type, NULL ) );
	g_object_set( load,
		"filename", string,
		NULL );

	return( VIPS_OBJECT( load ) );
}

static guint64
vips_get_disc_threshold( void )
{
	static gboolean done = FALSE;
	static guint64 threshold;

	if( !done ) {
		const char *env;

		done = TRUE;

		/* 100mb default.
		 */
		threshold = 100 * 1024 * 1024;

		if( (env = g_getenv( "IM_DISC_THRESHOLD" )) ) 
			threshold = vips__parse_size( env );

		if( vips__disc_threshold ) 
			threshold = vips__parse_size( vips__disc_threshold );

		VIPS_DEBUG_MSG( "vips_get_disc_threshold: "
			"%zd bytes\n", threshold );
	}

	return( threshold );
}

/* Our start function ... do the lazy open, if necessary, and return a region
 * on the new image.
 */
static void *
vips_foreign_load_start( VipsImage *out, void *a, void *dummy )
{
	VipsForeignLoad *load = VIPS_FOREIGN_LOAD( a );
	VipsObjectClass *object_class = VIPS_OBJECT_GET_CLASS( a );
	VipsForeignLoadClass *class = VIPS_FOREIGN_LOAD_GET_CLASS( a );

	if( !load->real ) {
		const guint64 disc_threshold = vips_get_disc_threshold();
		const guint64 image_size = VIPS_IMAGE_SIZEOF_IMAGE( load->out );

		/* We open via disc if:
		 * - 'disc' is set
		 * - disc-threshold has not been set to zero
		 * - the format does not support lazy read
		 * - the uncompressed image will be larger than 
		 *   vips_get_disc_threshold()
		 */
		if( load->disc && 
			disc_threshold && 
			!(load->flags & VIPS_FOREIGN_PARTIAL) &&
			image_size > disc_threshold ) 
			if( !(load->real = vips_image_new_disc_temp( "%s.v" )) )
				return( NULL );

		/* Otherwise, fall back to a "p".
		 */
		if( !load->real && 
			!(load->real = vips_image_new()) )
			return( NULL );

		/* Read the image in.
		 */
		if( class->load( load ) ||
			vips_image_pio_input( load->real ) ) 
			return( NULL );

		/* ->header() read the header into @out, load has read the
		 * image into @real. They must match exactly in size, bands,
		 * format and coding for the copy to work.  
		 *
		 * Someversions of ImageMagick give different results between
		 * Ping and Load for some formats, for example.
		 */
		if( load->real->Xsize != out->Xsize ||
			load->real->Ysize != out->Ysize ||
			load->real->Bands != out->Bands ||
			load->real->Coding != out->Coding ||
			load->real->BandFmt != out->BandFmt ) {
			vips_error( object_class->nickname,
				"%s", _( "header() and load() differ" ) ); 
			return( NULL );
		}
	}

	return( vips_region_new( load->real ) );
}

/* Just pointer-copy.
 */
static int
vips_foreign_load_generate( VipsRegion *or, 
	void *seq, void *a, void *b, gboolean *stop )
{
	VipsRegion *ir = (VipsRegion *) seq;

        VipsRect *r = &or->valid;

        /* Ask for input we need.
         */
        if( vips_region_prepare( ir, r ) )
                return( -1 );

        /* Attach output region to that.
         */
        if( vips_region_region( or, ir, r, r->left, r->top ) )
                return( -1 );

        return( 0 );
}

static int
vips_foreign_load_build( VipsObject *object )
{
	VipsForeignLoad *load = VIPS_FOREIGN_LOAD( object );
	VipsForeignLoadClass *class = VIPS_FOREIGN_LOAD_GET_CLASS( object );

	VipsForeignFlags flags;

	flags = 0;
	if( class->get_flags )
		flags |= class->get_flags( load );
	g_object_set( load, "flags", flags, NULL );

	if( VIPS_OBJECT_CLASS( vips_foreign_load_parent_class )->
		build( object ) )
		return( -1 );

	g_object_set( object, "out", vips_image_new(), NULL ); 

	/* Read the header into @out.
	 */
	if( class->header &&
		class->header( load ) ) 
		return( -1 );

	/* If there's no ->load() method then the header read has done
	 * everything. Otherwise, it's just set fields and we now must
	 * convert pixels on demand.
	 */
	if( class->load ) {
		/* ->header() should set the dhint. It'll default to the safe
		 * SMALLTILE if header() did not set it.
		 */
		vips_demand_hint( load->out, load->out->dhint, NULL );

		/* Then 'start' creates the real image and 'gen' fetches 
		 * pixels for @out from @real on demand.
		 */
		if( vips_image_generate( load->out, 
			vips_foreign_load_start, 
			vips_foreign_load_generate, 
			vips_stop_one, 
			load, NULL ) ) 
			return( -1 );
	}

	return( 0 );
}

static void
vips_foreign_load_class_init( VipsForeignLoadClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;

	gobject_class->dispose = vips_foreign_load_dispose;
	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->build = vips_foreign_load_build;
	object_class->print_class = vips_foreign_load_print_class;
	object_class->new_from_string = vips_foreign_load_new_from_string;
	object_class->nickname = "fileload";
	object_class->description = _( "file loaders" );

	VIPS_ARG_IMAGE( class, "out", 2, 
		_( "Output" ), 
		_( "Output image" ),
		VIPS_ARGUMENT_REQUIRED_OUTPUT, 
		G_STRUCT_OFFSET( VipsForeignLoad, out ) );

	VIPS_ARG_ENUM( class, "flags", 6, 
		_( "Flags" ), 
		_( "Flags for this file" ),
		VIPS_ARGUMENT_OPTIONAL_OUTPUT,
		G_STRUCT_OFFSET( VipsForeignLoad, flags ),
		VIPS_TYPE_FOREIGN_FLAGS, VIPS_FOREIGN_NONE ); 

	VIPS_ARG_BOOL( class, "disc", 7, 
		_( "Disc" ), 
		_( "Open to disc" ),
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( VipsForeignLoad, disc ),
		TRUE );
}

static void
vips_foreign_load_init( VipsForeignLoad *load )
{
	load->disc = TRUE;
}

/* Abstract base class for image savers.
 */

G_DEFINE_ABSTRACT_TYPE( VipsForeignSave, vips_foreign_save, VIPS_TYPE_FOREIGN );

static void
vips_foreign_save_dispose( GObject *gobject )
{
	VipsForeignSave *save = VIPS_FOREIGN_SAVE( gobject );

	VIPS_UNREF( save->ready );

	G_OBJECT_CLASS( vips_foreign_save_parent_class )->dispose( gobject );
}

static void
vips_foreign_save_print_class( VipsObjectClass *object_class, VipsBuf *buf )
{
	VipsForeignSaveClass *class = VIPS_FOREIGN_SAVE_CLASS( object_class );

	VIPS_OBJECT_CLASS( vips_foreign_save_parent_class )->
		print_class( object_class, buf );

	vips_buf_appendf( buf, ", %s", 
		VIPS_ENUM_NICK( VIPS_TYPE_SAVEABLE, class->saveable ) );
}

/* Can we write this filename with this file? 
 */
static void *
vips_foreign_find_save_sub( VipsForeignSaveClass *save_class, 
	const char *filename )
{
	VipsForeignClass *class = VIPS_FOREIGN_CLASS( save_class );

	if( class->suffs &&
		vips_filename_suffix_match( filename, class->suffs ) )
		return( save_class );

	return( NULL );
}

/**
 * vips_foreign_find_save:
 * @filename: name to find a file for
 *
 * Searches for an operation you could use to save a file.
 *
 * See also: vips_foreign_write().
 *
 * Returns: the name of an operation on success, %NULL on error
 */
const char *
vips_foreign_find_save( const char *filename )
{
	VipsForeignSaveClass *save_class;

	if( !(save_class = (VipsForeignSaveClass *) vips_foreign_map( 
		"VipsForeignSave",
		(VipsSListMap2Fn) vips_foreign_find_save_sub, 
		(void *) filename, NULL )) ) {
		vips_error( "VipsForeignSave",
			_( "\"%s\" is not a supported image file." ), 
			filename );

		return( NULL );
	}

	return( G_OBJECT_CLASS_NAME( save_class ) );
}

static VipsObject *
vips_foreign_save_new_from_string( const char *string )
{
	const char *file_op;
	GType type;
	VipsForeignSave *save;

	if( !(file_op = vips_foreign_find_save( string )) )
		return( NULL );
	type = g_type_from_name( file_op );
	g_assert( type ); 

	save = VIPS_FOREIGN_SAVE( g_object_new( type, NULL ) );
	g_object_set( save,
		"filename", string,
		NULL );

	return( VIPS_OBJECT( save ) );
}

/* Generate the saveable image.
 */
static int
vips_foreign_convert_saveable( VipsForeignSave *save )
{
	VipsForeignSaveClass *class = VIPS_FOREIGN_SAVE_GET_CLASS( save );
	VipsImage *in = save->in;

	/* in holds a reference to the output of our chain as we build it.
	 */
	g_object_ref( in );

	/* Does this class want a coded image we are already in? Nothing to
	 * do.
	 */
	if( class->coding != VIPS_CODING_NONE &&
		class->coding == in->Coding ) {
		VIPS_UNREF( save->ready );
		save->ready = in;

		return( 0 );
	}

	/* Otherwise ... we need to decode and then (possibly) recode at the
	 * end.
	 */

	/* If this is an VIPS_CODING_LABQ, we can go straight to RGB.
	 */
	if( in->Coding == VIPS_CODING_LABQ ) {
		VipsImage *out;

		if( vips_LabQ2disp( in, &out, im_col_displays( 7 ), NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	/* If this is an VIPS_CODING_RAD, we go to float RGB or XYZ. We should
	 * probably un-gamma-correct the RGB :(
	 */
	if( in->Coding == VIPS_CODING_RAD ) {
		VipsImage *out;

		if( vips_rad2float( in, &out, NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	/* Get the bands right. 
	 */
	if( in->Coding == VIPS_CODING_NONE ) {
		if( in->Bands == 2 && 
			class->saveable != VIPS_SAVEABLE_RGBA ) {
			VipsImage *out;

			if( vips_extract_band( in, &out, 0, NULL ) ) {
				g_object_unref( in );
				return( -1 );
			}
			g_object_unref( in );

			in = out;
		}
		else if( in->Bands > 3 && 
			class->saveable == VIPS_SAVEABLE_RGB ) {
			VipsImage *out;

			if( vips_extract_band( in, &out, 0, 
				"n", 3,
				NULL ) ) {
				g_object_unref( in );
				return( -1 );
			}
			g_object_unref( in );

			in = out;
		}
		else if( in->Bands > 4 && 
			(class->saveable == VIPS_SAVEABLE_RGB_CMYK || 
			 class->saveable == VIPS_SAVEABLE_RGBA) ) {
			VipsImage *out;

			if( vips_extract_band( in, &out, 0, 
				"n", 4,
				NULL ) ) {
				g_object_unref( in );
				return( -1 );
			}
			g_object_unref( in );

			in = out;
		}
		else if( in->Bands > 1 && 
			class->saveable == VIPS_SAVEABLE_MONO ) {
			VipsImage *out;

			if( vips_extract_band( in, &out, 0, NULL ) ) {
				g_object_unref( in );
				return( -1 );
			}
			g_object_unref( in );

			in = out;
		}

		/* Else we have VIPS_SAVEABLE_ANY and we don't chop bands down.
		 */
	}

	/* Interpret the Type field for colorimetric images.
	 */
	if( in->Bands == 3 && 
		in->BandFmt == VIPS_FORMAT_SHORT && 
		in->Type == VIPS_INTERPRETATION_LABS ) {
		VipsImage *out;

		if( vips_LabS2LabQ( in, &out, NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	if( in->Coding == VIPS_CODING_LABQ ) {
		VipsImage *out;

		if( vips_LabQ2Lab( in, &out, NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	if( in->Coding != VIPS_CODING_NONE ) {
		g_object_unref( in );
		return( -1 );
	}

	if( in->Bands == 3 && 
		in->Type == VIPS_INTERPRETATION_LCH ) {
		VipsImage *out;

                if( vips_LCh2Lab( in, &out, NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	if( in->Bands == 3 && 
		in->Type == VIPS_INTERPRETATION_YXY ) {
		VipsImage *out;

                if( vips_Yxy2Lab( in, &out, NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	if( in->Bands == 3 && 
		in->Type == VIPS_INTERPRETATION_UCS ) {
		VipsImage *out;

                if( vips_UCS2XYZ( in, &out, NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	if( in->Bands == 3 && 
		in->Type == VIPS_INTERPRETATION_LAB ) {
		VipsImage *out;

		if( vips_XYZ2disp( in, &out, im_col_displays( 7 ), NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	/* Cast to the output format.
	 */
	{
		VipsImage *out;

		if( vips_cast( in, &out, 
			class->format_table[in->BandFmt], NULL ) ) {
			g_object_unref( in );
			return( -1 );
		}
		g_object_unref( in );

		in = out;
	}

	/* Does this class want a coded image? The format_table[] should have
	 * got the format right for us.
	 */
	if( class->coding != VIPS_CODING_NONE ) {
		if( class->coding == VIPS_CODING_RAD ) {
			VipsImage *out;

			if( vips_float2rad( in, &out, NULL ) ) {
				g_object_unref( in );
				return( -1 );
			}
			g_object_unref( in );

			in = out;
		}
		else if( class->coding == VIPS_CODING_LABQ ) {
			VipsImage *out;

			if( vips_Lab2LabQ( in, &out, NULL ) ) {
				g_object_unref( in );
				return( -1 );
			}
			g_object_unref( in );

			in = out;
		}
	}

	VIPS_UNREF( save->ready );
	save->ready = in;

	return( 0 );
}

static int
vips_foreign_save_build( VipsObject *object )
{
	VipsForeignSave *save = VIPS_FOREIGN_SAVE( object );

	if( vips_foreign_convert_saveable( save ) )
		return( -1 );

	if( VIPS_OBJECT_CLASS( vips_foreign_save_parent_class )->
		build( object ) )
		return( -1 );

	return( 0 );
}

static void
vips_foreign_save_class_init( VipsForeignSaveClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *object_class = (VipsObjectClass *) class;

	gobject_class->dispose = vips_foreign_save_dispose;
	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->build = vips_foreign_save_build;
	object_class->print_class = vips_foreign_save_print_class;
	object_class->new_from_string = vips_foreign_save_new_from_string;
	object_class->nickname = "filesave";
	object_class->description = _( "file savers" );

	class->coding = VIPS_CODING_NONE;

	VIPS_ARG_IMAGE( class, "in", 0, 
		_( "Input" ), 
		_( "Image to save" ),
		VIPS_ARGUMENT_REQUIRED_INPUT,
		G_STRUCT_OFFSET( VipsForeignSave, in ) );
}

static void
vips_foreign_save_init( VipsForeignSave *object )
{
}

/**
 * vips_foreign_load:
 * @filename: file to load
 * @out: output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Loads @filename into @out using the loader recommended by
 * vips_foreign_find_load().
 *
 * See also: vips_foreign_save(), vips_foreign_read_options().
 *
 * Returns: 0 on success, -1 on error
 */
int
vips_foreign_load( const char *filename, VipsImage **out, ... )
{
	const char *operation;
	va_list ap;
	int result;

	if( !(operation = vips_foreign_find_load( filename )) )
		return( -1 );

	va_start( ap, out );
	result = vips_call_split( operation, ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_foreign_save:
 * @in: image to write
 * @filename: file to write to
 * @...: %NULL-terminated list of optional named arguments
 *
 * Saves @in to @filename using the saver recommended by
 * vips_foreign_find_save().
 *
 * See also: vips_foreign_load().
 *
 * Returns: 0 on success, -1 on error
 */
int
vips_foreign_save( VipsImage *in, const char *filename, ... )
{
	const char *operation;
	va_list ap;
	int result;

	if( !(operation = vips_foreign_find_save( filename )) )
		return( -1 );

	va_start( ap, filename );
	result = vips_call_split( operation, ap, in, filename );
	va_end( ap );

	return( result );
}

/**
 * vips_foreign_load_options:
 * @filename: file to load
 * @out: output image
 *
 * Loads @filename into @out using the loader recommended by
 * vips_foreign_find_load().
 *
 * Arguments to the loader may be embedded in the filename using the usual
 * syntax.
 *
 * See also: vips_foreign_load().
 *
 * Returns: 0 on success, -1 on error
 */
int
vips_foreign_load_options( const char *filename, VipsImage **out )
{
	VipsObjectClass *oclass = g_type_class_ref( VIPS_TYPE_FOREIGN_LOAD );

	VipsObject *object;

	/* This will use vips_foreign_load_new_from_string() to pick a loader,
	 * then set options from the remains of the string.
	 */
	if( !(object = vips_object_new_from_string( oclass, filename )) )
		return( -1 );

	if( vips_cache_operation_build( (VipsOperation **) &object ) ) {
		/* The build may have made some output objects before
		 * failing.
		 */
		vips_object_unref_outputs( object );
		g_object_unref( object );
		return( -1 );
	}

	g_object_get( object, "out", out, NULL );

	/* Getting @out will have upped its count so it'll be safe.
	 * We can junk all other outputs,
	 */
	vips_object_unref_outputs( object );

	/* @out holds a ref to new_object, we can drop ours.
	 */
	g_object_unref( object );

	return( 0 );
}

/**
 * vips_foreign_save_options:
 * @in: image to write
 * @filename: file to write to
 *
 * Saves @in to @filename using the saver recommended by
 * vips_foreign_find_save().
 *
 * See also: vips_foreign_save().
 *
 * Returns: 0 on success, -1 on error
 */
int
vips_foreign_save_options( VipsImage *in, const char *filename )
{
	VipsObjectClass *oclass = g_type_class_ref( VIPS_TYPE_FOREIGN_SAVE );
	VipsObject *object;

	/* This will use vips_foreign_save_new_from_string() to pick a saver,
	 * then set options from the tail of the filename.
	 */
	if( !(object = vips_object_new_from_string( oclass, filename )) )
		return( -1 );

	g_object_set( object, "in", in, NULL );

	/* ... and running _build() should save it.
	 */
	if( vips_cache_operation_build( (VipsOperation **) &object ) ) {
		g_object_unref( object );
		return( -1 );
	}

	g_object_unref( object );

	return( 0 );
}

/* Called from iofuncs to init all operations in this dir. Use a plugin system
 * instead?
 */
void
vips_foreign_operation_init( void )
{
	extern GType vips_foreign_load_rad_get_type( void ); 
	extern GType vips_foreign_save_rad_get_type( void ); 
	extern GType vips_foreign_load_mat_get_type( void ); 
	extern GType vips_foreign_load_ppm_get_type( void ); 
	extern GType vips_foreign_save_ppm_get_type( void ); 
	extern GType vips_foreign_load_png_get_type( void ); 
	extern GType vips_foreign_save_png_file_get_type( void ); 
	extern GType vips_foreign_save_png_buffer_get_type( void ); 
	extern GType vips_foreign_load_csv_get_type( void ); 
	extern GType vips_foreign_save_csv_get_type( void ); 
	extern GType vips_foreign_load_fits_get_type( void ); 
	extern GType vips_foreign_save_fits_get_type( void ); 
	extern GType vips_foreign_load_analyze_get_type( void ); 
	extern GType vips_foreign_load_openexr_get_type( void ); 
	extern GType vips_foreign_load_openslide_get_type( void ); 
	extern GType vips_foreign_load_jpeg_file_get_type( void ); 
	extern GType vips_foreign_load_jpeg_buffer_get_type( void ); 
	extern GType vips_foreign_save_jpeg_file_get_type( void ); 
	extern GType vips_foreign_save_jpeg_buffer_get_type( void ); 
	extern GType vips_foreign_save_jpeg_mime_get_type( void ); 
	extern GType vips_foreign_load_tiff_get_type( void ); 
	extern GType vips_foreign_save_tiff_get_type( void ); 
	extern GType vips_foreign_load_vips_get_type( void ); 
	extern GType vips_foreign_save_vips_get_type( void ); 
	extern GType vips_foreign_load_raw_get_type( void ); 
	extern GType vips_foreign_save_raw_get_type( void ); 
	extern GType vips_foreign_save_raw_fd_get_type( void ); 
	extern GType vips_foreign_load_magick_get_type( void ); 

	vips_foreign_load_rad_get_type(); 
	vips_foreign_save_rad_get_type(); 
	vips_foreign_load_ppm_get_type(); 
	vips_foreign_save_ppm_get_type(); 
	vips_foreign_load_csv_get_type(); 
	vips_foreign_save_csv_get_type(); 
	vips_foreign_load_analyze_get_type(); 
	vips_foreign_load_raw_get_type(); 
	vips_foreign_save_raw_get_type(); 
	vips_foreign_save_raw_fd_get_type(); 
	vips_foreign_load_vips_get_type(); 
	vips_foreign_save_vips_get_type(); 

#ifdef HAVE_PNG
	vips_foreign_load_png_get_type(); 
	vips_foreign_save_png_file_get_type(); 
	vips_foreign_save_png_buffer_get_type(); 
#endif /*HAVE_PNG*/

#ifdef HAVE_MATIO
	vips_foreign_load_mat_get_type(); 
#endif /*HAVE_MATIO*/

#ifdef HAVE_JPEG
	vips_foreign_load_jpeg_file_get_type(); 
	vips_foreign_load_jpeg_buffer_get_type(); 
	vips_foreign_save_jpeg_file_get_type(); 
	vips_foreign_save_jpeg_buffer_get_type(); 
	vips_foreign_save_jpeg_mime_get_type(); 
#endif /*HAVE_JPEG*/

#ifdef HAVE_TIFF
	vips_foreign_load_tiff_get_type(); 
	vips_foreign_save_tiff_get_type(); 
#endif /*HAVE_TIFF*/

#ifdef HAVE_OPENSLIDE
	vips_foreign_load_openslide_get_type(); 
#endif /*HAVE_OPENSLIDE*/

#ifdef HAVE_MAGICK
	vips_foreign_load_magick_get_type(); 
#endif /*HAVE_MAGICK*/

#ifdef HAVE_CFITSIO
	vips_foreign_load_fits_get_type(); 
	vips_foreign_save_fits_get_type(); 
#endif /*HAVE_CFITSIO*/

#ifdef HAVE_OPENEXR
	vips_foreign_load_openexr_get_type(); 
#endif /*HAVE_OPENEXR*/
}

/**
 * vips_magickload:
 * @filename: file to load
 * @out: decompressed image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read in an image using libMagick, the ImageMagick library. This library can
 * read more than 80 file formats, including SVG, BMP, EPS, DICOM and many 
 * others.
 * The reader can handle any ImageMagick image, including the float and double
 * formats. It will work with any quantum size, including HDR. Any metadata
 * attached to the libMagick image is copied on to the VIPS image.
 *
 * The reader should also work with most versions of GraphicsMagick. See the
 * "--with-magickpackage" configure option.
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_magickload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "magickload", ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_tiffload:
 * @filename: file to load
 * @out: decompressed image
 * @page: load this page
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a TIFF file into a VIPS image. It is a full baseline TIFF 6 reader, 
 * with extensions for tiled images, multipage images, LAB colour space, 
 * pyramidal images and JPEG compression. including CMYK and YCbCr.
 *
 * @page means load this page from the file. By default the first page (page
 * 0) is read.
 *
 * Any ICC profile is read and attached to the VIPS image.
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_tiffload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "tiffload", ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_tiffsave:
 * @in: image to save 
 * @filename: file to write to 
 * @compression; use this compression scheme
 * @Q: quality factor
 * @predictor; compress with this prediction
 * @profile: attach this ICC profile
 * @tile; set %TRUE to write a tiled tiff
 * @tile_width; set tile size
 * @tile_height; set tile size
 * @pyramid; set %TRUE to write an image pyramid
 * @squash; squash 8-bit images down to 1 bit
 * @resunit; use pixels per inch or cm for the resolution
 * @xres; horizontal resolution
 * @yres; vertical resolution
 * @bigtiff; write a BigTiff file
 * @...: %NULL-terminated list of optional named arguments
 *
 * Write a VIPS image to a file as TIFF.
 *
 * Use @compression to set the tiff compression. Currently jpeg, packbits,
 * fax4, lzw, none and deflate are supported. The default is no compression.
 * JPEG compression is a good lossy compressor for photographs, packbits is 
 * good for 1-bit images, and deflate is the best lossless compression TIFF 
 * can do. LZW has patent problems and is no longer recommended.
 *
 * Use @Q to set the JPEG compression factor. Default 75.
 *
 * Use @predictor to set the predictor for lzw and deflate compression. 
 *
 * Predictor is not set by default. There are three predictor values recognised
 * at the moment (2007, July): 1 is no prediction, 2 is a horizontal 
 * differencing and 3 is a floating point predictor. Refer to the libtiff 
 * specifications for further discussion of various predictors. In short, 
 * predictor helps to better compress image, especially in case of digital 
 * photos or scanned images and bit depths > 8. Try it to find whether it 
 * works for your images.
 *
 * Use @profile to give the filename of a profile to be embedded in the TIFF.
 * This does not affect the pixels which are written, just the way 
 * they are tagged. You can use the special string "none" to mean 
 * "don't attach a profile".
 *
 * If no profile is specified and the VIPS header 
 * contains an ICC profile named VIPS_META_ICC_NAME ("icc-profile-data"), the
 * profile from the VIPS header will be attached.
 *
 * Set @tile to TRUE to write a tiled tiff.  By default tiff are written in
 * strips. Use @tile_width and @tile_height to set the tile size. The defaiult
 * is 128 by 128.
 *
 * Set @pyramid to write the image as a set of images, one per page, of
 * decreasing size. 
 *
 * Set @squash to make 8-bit uchar images write as 1-bit TIFFs with zero
 * pixels written as 0 and non-zero as 1.
 *
 * Use @resunit to override the default resolution unit.  
 * The default 
 * resolution unit is taken from the header field "resolution-unit"
 * (#VIPS_META_RESOLUTION_UNIT in C). If this field is not set, then 
 * VIPS defaults to cm.
 *
 * Use @xres and @yres to override the default horizontal and vertical
 * resolutions. By default these values are taken from the VIPS image header. 
 *
 * Set @bigtiff to attempt to write a bigtiff. 
 * Bigtiff is a variant of the TIFF
 * format that allows more than 4GB in a file.
 *
 * See also: vips_tiffload(), vips_image_write_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_tiffsave( VipsImage *in, const char *filename, ... )
{
	va_list ap;
	int result;

	va_start( ap, filename );
	result = vips_call_split( "tiffsave", ap, in, filename );
	va_end( ap );

	return( result );
}

/**
 * vips_jpegload_buffer:
 * @buf: memory area to load
 * @len: size of memory area
 * @out: image to write
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a JPEG-formatted memory block into a VIPS image. It can read most 
 * 8-bit JPEG images, including CMYK and YCbCr.
 *
 * This function is handy for processing JPEG image thumbnails.
 *
 * Caution: on return only the header will have been read, the pixel data is
 * not decompressed until the first pixel is read. Therefore you must not free
 * @buf until you have read pixel data from @out.
 *
 * See also: vips_jpegload().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_jpegload_buffer( void *buf, size_t len, VipsImage **out, ... )
{
	va_list ap;
	VipsArea *area;
	int result;

	/* We don't take a copy of the data or free it.
	 */
	area = vips_area_new_blob( NULL, buf, len );

	va_start( ap, out );
	result = vips_call_split( "jpegload_buffer", ap, area, out );
	va_end( ap );

	vips_area_unref( area );

	return( result );
}

/**
 * vips_jpegload:
 * @filename: file to load
 * @out: decompressed image
 * @flags: image flags
 * @shrink: shrink by this much on load
 * @fail: fail on warnings
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a JPEG file into a VIPS image. It can read most 8-bit JPEG images, 
 * including CMYK and YCbCr.
 *
 * @shrink means shrink by this integer factor during load.  Possible values 
 * are 1, 2, 4 and 8. Shrinking during read is very much faster than 
 * decompressing the whole image and then shrinking later.
 *
 * Setting @fail to true makes the JPEG reader fail on any warnings. 
 * This can be useful for detecting truncated files, for example. Normally 
 * reading these produces a warning, but no fatal error.  
 *
 * Example:
 *
 * |[
 * vips_jpegload( "fred.jpg", &out,
 * 	"shrink", 8,
 * 	"fail", TRUE,
 * 	NULL );
 * ]|
 *
 * Any embedded ICC profiles are ignored: you always just get the RGB from 
 * the file. Instead, the embedded profile will be attached to the image as 
 * metadata.  You need to use something like im_icc_import() to get CIE 
 * values from the file. Any EXIF data is also attached as VIPS metadata.
 *
 * The int metadata item "jpeg-multiscan" is set to the result of 
 * jpeg_has_multiple_scans(). Interlaced jpeg images need a large amount of
 * memory to load, so this field gives callers a chance to handle these
 * images differently.
 *
 * The EXIF thumbnail, if present, is attached to the image as 
 * "jpeg-thumbnail-data". See vips_image_get_blob().
 *
 * This function only reads the image header and does not decompress any pixel
 * data. Decompression only occurs when pixels are accessed by some other
 * function.
 *
 * See also: vips_jpegload_buffer(), vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_jpegload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "jpegload", ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_jpegsave_mime:
 * @in: image to save 
 * @Q: JPEG quality factor
 * @profile: attach this ICC profile
 * @...: %NULL-terminated list of optional named arguments
 *
 * As vips_jpegsave(), but save as a mime jpeg on stdout.
 *
 * See also: vips_jpegsave(), vips_image_write_to_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_jpegsave_mime( VipsImage *in, ... )
{
	va_list ap;
	int result;

	va_start( ap, in );
	result = vips_call_split( "jpegsave_mime", ap, in );
	va_end( ap );

	return( result );
}

/**
 * vips_jpegsave_buffer:
 * @in: image to save 
 * @buf: return output buffer here
 * @len: return output length here
 * @Q: JPEG quality factor
 * @profile: attach this ICC profile
 * @...: %NULL-terminated list of optional named arguments
 *
 * As vips_jpegsave(), but save to a memory buffer. 
 *
 * The address of the buffer is returned in @obuf, the length of the buffer in
 * @olen. You are responsible for freeing the buffer with g_free() when you
 * are done with it.
 *
 * See also: vips_jpegsave(), vips_image_write_to_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_jpegsave_buffer( VipsImage *in, void **buf, size_t *len, ... )
{
	va_list ap;
	VipsArea *area;
	int result;

	va_start( ap, len );
	result = vips_call_split( "jpegsave_buffer", ap, in, &area );
	va_end( ap );

	if( buf ) {
		*buf = area->data;
		area->free_fn = NULL;
	}
	if( buf ) 
		*len = area->length;

	vips_area_unref( area );

	return( result );
}

/**
 * vips_jpegsave:
 * @in: image to save 
 * @filename: file to write to 
 * @Q: quality factor
 * @profile: attach this ICC profile
 * @...: %NULL-terminated list of optional named arguments
 *
 * Write a VIPS image to a file as JPEG.
 *
 * Use @Q to set the JPEG compression factor. Default 75.
 *
 * Use @profile to give the filename of a profile to be em,bedded in the JPEG.
 * This does not affect the pixels which are written, just the way 
 * they are tagged. You can use the special string "none" to mean 
 * "don't attach a profile".
 *
 * If no profile is specified and the VIPS header 
 * contains an ICC profile named VIPS_META_ICC_NAME ("icc-profile-data"), the
 * profile from the VIPS header will be attached.
 *
 * The image is automatically converted to RGB, Monochrome or CMYK before 
 * saving. Any metadata attached to the image is saved as EXIF, if possible.
 *
 * See also: vips_jpegsave_buffer(), vips_image_write_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_jpegsave( VipsImage *in, const char *filename, ... )
{
	va_list ap;
	int result;

	va_start( ap, filename );
	result = vips_call_split( "jpegsave", ap, in, filename );
	va_end( ap );

	return( result );
}

/**
 * vips_openexrload:
 * @filename: file to load
 * @out: decompressed image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a OpenEXR file into a VIPS image. 
 *
 * The reader can handle scanline and tiled OpenEXR images. It can't handle
 * OpenEXR colour management, image attributes, many pixel formats, anything
 * other than RGBA.
 *
 * This reader uses the rather limited OpenEXR C API. It should really be
 * redone in C++.
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_openexrload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "openexrload", ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_openslideload:
 * @filename: file to load
 * @out: decompressed image
 * @layer: load this layer
 * @associated: load this associated image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a virtual slide supported by the OpenSlide library into a VIPS image.
 * OpenSlide supports images in Aperio, Hamamatsu VMS, Hamamatsu VMU, MIRAX,
 * and Trestle formats.  
 *
 * To facilitate zooming, virtual slide formats include multiple scaled-down
 * versions of the high-resolution image.  These are typically called
 * "levels", though OpenSlide and im_openslide2vips() call them "layers".
 * By default, vips_openslideload() reads the highest-resolution layer
 * (layer 0).  Set @layer to the layer number you want.
 *
 * In addition to the slide image itself, virtual slide formats sometimes
 * include additional images, such as a scan of the slide's barcode.
 * OpenSlide calls these "associated images".  To read an associated image,
 * set @associated to the image's name.
 * A slide's associated images are listed in the
 * "slide-associated-images" metadata item.
 *
 * The output of this operator is in pre-multipled ARGB format. Use
 * im_argb2rgba() to decode to png-style RGBA. 
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_openslideload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "openslideload", ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_fitsload:
 * @filename: file to load
 * @out: decompressed image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a FITS image file into a VIPS image. 
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_fitsload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "fitsload", ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_fitssave:
 * @in: image to save 
 * @filename: file to write to 
 * @...: %NULL-terminated list of optional named arguments
 *
 * Write a VIPS image to a file as FITS.
 *
 * See also: vips_image_write_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_fitssave( VipsImage *in, const char *filename, ... )
{
	va_list ap;
	int result;

	va_start( ap, filename );
	result = vips_call_split( "fitssave", ap, in, filename );
	va_end( ap );

	return( result );
}

/**
 * vips_pngload:
 * @filename: file to load
 * @out: decompressed image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a PNG file into a VIPS image. It can read all png images, including 8-
 * and 16-bit images, 1 and 3 channel, with and without an alpha channel.
 *
 * There is no support for embedded ICC profiles.
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_pngload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "pngload", ap, filename, out );
	va_end( ap );

	return( result );
}

/**
 * vips_pngsave:
 * @in: image to save 
 * @filename: file to write to 
 * @compression: compression level
 * @interlace: interlace image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Write a VIPS image to a file as PNG.
 *
 * @compression means compress with this much effort (0 - 9). Default 6.
 *
 * Set @interlace to %TRUE to interlace the image with ADAM7 
 * interlacing. Beware
 * than an interlaced PNG can be up to 7 times slower to write than a
 * non-interlaced image.
 *
 * There is no support for attaching ICC profiles to PNG images.
 *
 * The image is automatically converted to RGB, RGBA, Monochrome or Mono +
 * alpha before saving. Images with more than one byte per band element are
 * saved as 16-bit PNG, others are saved as 8-bit PNG.
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_pngsave( VipsImage *in, const char *filename, ... )
{
	va_list ap;
	int result;

	va_start( ap, filename );
	result = vips_call_split( "pngsave", ap, in, filename );
	va_end( ap );

	return( result );
}

/**
 * vips_pngsave_buffer:
 * @in: image to save 
 * @buf: return output buffer here
 * @len: return output length here
 * @compression: compression level
 * @interlace: interlace image
 * @...: %NULL-terminated list of optional named arguments
 *
 * As vips_pngsave(), but save to a memory buffer. 
 *
 * The address of the buffer is returned in @obuf, the length of the buffer in
 * @olen. You are responsible for freeing the buffer with g_free() when you
 * are done with it.
 *
 * See also: vips_pngsave(), vips_image_write_to_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_pngsave_buffer( VipsImage *in, void **buf, size_t *len, ... )
{
	va_list ap;
	VipsArea *area;
	int result;

	va_start( ap, len );
	result = vips_call_split( "pngsave_buffer", ap, in, &area );
	va_end( ap );

	if( buf ) {
		*buf = area->data;
		area->free_fn = NULL;
	}
	if( buf ) 
		*len = area->length;

	vips_area_unref( area );

	return( result );
}

/**
 * vips_matload:
 * @filename: file to load
 * @out: output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Read a Matlab save file into a VIPS image. 
 *
 * This operation searches the save
 * file for the first array variable with between 1 and 3 dimensions and loads
 * it as an image. It will not handle complex images. It does not handle
 * sparse matrices. 
 *
 * See also: vips_image_new_from_file().
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_matload( const char *filename, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "matload", ap, filename, out ); 
	va_end( ap );

	return( result );
}

