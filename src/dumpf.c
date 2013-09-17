/*
 * vim: ts=8 sw=8
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gcc-compat.h>

#define	PERLINE		16

static	char *		me = "dumpf";
static	int		nonfatal;
static	off_t		initialOffset;
static	off_t		resid;
static	char *		ofile;
static	int		noAscii;
static	unsigned	sw_canSkip = 1;

static void
usage(
	char *		fmt,
	...
)
{
	if( fmt )	{
		va_list		ap;

		fprintf( stderr, "%s: ", me );
		va_start( ap, fmt );
		vfprintf( stderr, fmt, ap );
		va_end( ap );
		fprintf( stderr, ".\n" );
	}
	fprintf(
		stderr,
		"usage: "
		"%s "
		"[-a] "
		"[-b begin] "
		"[-h[elp]] "
		"[-n nbytes] "
		"[-o ofile] "
		"file .."
		"\n",
		me
	);
}

static	void
show_line(
	off_t		offset,
	uint8_t *	input,
	size_t		n
)
{
	size_t		i;
	char		hex[ PERLINE ][ 3 ];
	char		text[ PERLINE ];
	/* Insert filler everywhere				 */
	memset( text, '~', sizeof( text ) );
	for( i = 0; i < PERLINE; ++i )	{
		strncpy( hex[ i ], " ..", 3 );
	}
	/* Interpret the bytes we have				 */
	for( i = 0; i < n; ++i )	{
		int const	c = input[ i ];
		char		value[ 4 ];

		/* Build the hex representation			 */
		sprintf( value, " %02X", c );
		strncpy( hex[ i ], value, 3 );
		/* Build the printable representation		 */
		text[ i ] = isprint( c ) ? c : '.';
	}
	/* Output what we have					 */
	printf(
		"%07llX %*.*s  %*.*s\n", 
		(unsigned long long) offset, 
		PERLINE*3,
		PERLINE*3,
		(char *) hex, 
		PERLINE,
		PERLINE,
		noAscii ? "" : text
	);
}

static void
process(
	char * const	fn _unused,
	FILE * const	fyle
)
{
	off_t		offset;
	uint8_t		old_input[ PERLINE ];
	size_t		old_n;
	off_t		old_offset;
	unsigned	skipping;

	offset = initialOffset;
	if( offset && fseek( fyle, offset, SEEK_SET ) )	{
		fprintf(
			stderr, 
			"%s: could not seek to %llu\n", 
			me, 
			(unsigned long long) offset 
		);
		++nonfatal;
	}
	skipping = 0;
	old_n = 0;
	old_offset = 0;
	while( !feof( fyle ) )	{
		off_t const	padding = offset % PERLINE;
		uint8_t		input[ PERLINE ];
		size_t		n;
		size_t		gulp;

		/* Back up if offset is not multiple of PERLINE		 */
		offset -= padding;
		/* Read just enough to finish this line			 */
		gulp = PERLINE - padding;
		if( resid && ((off_t) gulp > resid) )	{
			gulp = resid;
		}
		n = fread( input+padding, 1, gulp, fyle );
		if( n <= 0 )	{
			/* Hit EOF					 */
			break;
		}
		/* Compress duplicate lines if we're allowed		*/
		if( sw_canSkip && skipping )	{
			if(
				(n == old_n)			&&
				!memcmp( input+padding, old_input, n )
			)	{
				/* Line is a duplicate			*/
				old_offset = offset;
				++skipping;
				continue;
			}
			/* Data changed, show old if we have any	*/
			if( old_n )	{
				if( skipping > 1 )	{
					printf(
						"... %llu lines skipped\n",
						(unsigned long long) skipping
					);
				}
				show_line( old_offset, old_input, old_n );
			}
			old_n = 0;
			skipping = 0;
		} else if( sw_canSkip && (old_n == n) )	{
			/* Maybe this line duplicates last one		*/
			if( !memcmp(
				input+padding,
				old_input,
				old_n
			) )	{
				/* Yes, it does!			*/
				old_offset = offset;
				skipping = 1;
				continue;
			}
		}
		/* Save copy so we can tell if next line duplicates us	*/
		old_n = n;
		old_offset = offset;
		memcpy( old_input, input+padding, n );
		show_line( old_offset, old_input, old_n );
		/* Advance the position					 */
		offset += PERLINE;
		/* If that was all we wanted, GAME OVER			 */
		if( resid && ((resid -= gulp) == 0) )	{
			break;
		}
	}
	if( skipping && old_n )	{
		show_line( old_offset, old_input, old_n );
	}
}

int
main(
	int		argc,
	char * *	argv
)
{
	char *		bp;
	int		c;

        /* Determine how we are being run                                */
	me = argv[ 0 ];
	if( (bp = strrchr( me, '/' )) != NULL )	{
		me = bp + 1;
	}
	/* Parse any command line options				 */
	while( (c = getopt( argc, argv, "Dab:dhn:o:" )) != EOF )	{
		switch( c )	{
		default:
			fprintf( 
				stderr, 
				"%s: '-%c' not implemented yet\n", 
				me, 
				c 
			);
			/* FALLTHROUGH */
		case '?':
			++nonfatal;
			break;
		case 'a':
			++noAscii;
			break;
		case 'b':
			initialOffset = strtoull( optarg, NULL, 0 );
			break;
		case 'd':
			sw_canSkip = 0;
			break;
		case 'h':
			usage( NULL );
			exit( 0 );
			/*NOTREACHED*/
		case 'n':
			resid = strtoull( optarg, NULL, 0 );
			break;
		case 'o':
			ofile = optarg;
			break;
		}
	}
	do	{
		/* Catch illegal command lines				 */
		if( nonfatal )	{
			usage( "illegal switch(es)" );
			break;
		}
		/* Redirect output if asked				 */
		if( ofile )	{
			unlink( ofile );
			if( freopen( ofile, "w", stdout ) != stdout )	{
				fprintf(
					stderr,
					"%s: "
					"could not open '%s' for writing.\n",
					me,
					ofile
				);
				++nonfatal;
				break;
			}
		}
		if( optind < argc )	{
			/* Any remaining args are input files		 */
			int const	multi = ((argc - optind) > 1);
			while( optind < argc )	{
				char * const	fn = argv[ optind++ ];
				FILE * const	fyle = fopen( fn, "rb" );

				if( multi )	{
					printf( "File: %s\n", fn );
				}
				if( fyle )	{
					process( fn, fyle );
					fclose( fyle );
				} else	{
					fprintf(
						stderr,
						"%s: "
						"cannot stat file (%s)"
						".\n",
						me,
						fn
					);
					++nonfatal;
				}
				if( multi && (optind < argc) )	{
					printf( "\n" );
				}
			}
		} else	{
			/* Process STDIN				 */
			process( "{stdin}", stdin );
		}
	} while( 0 );
	exit( nonfatal ? 1 : 0 );
}
