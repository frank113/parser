#include <Rinternals.h>
#include <parser.h>

static Rboolean known_to_be_utf8 = FALSE ;
static Rboolean known_to_be_latin1 = FALSE ;

Rboolean get_latin1(){ return known_to_be_latin1 ; }
void set_latin1( Rboolean value ) { known_to_be_latin1 = value ; }
    
Rboolean get_utf8(){ return known_to_be_utf8 ; }
void set_utf8(Rboolean value) { known_to_be_utf8 = value ; }

/** 
 * same as _fgetc but without the \r business
 */
#ifdef Win32
inline int __fgetc(FILE* fp){
    int c;
    static int nexteof=0;
    if (nexteof) {
       nexteof = 0;
       return R_EOF;
    }
    c = fgetc(fp);
    if (c==EOF) {
       nexteof = 1;
       return '\n';
    }
    return c ;
}
#endif

/*{{{ nlines */
/** 
 * Get the number of lines from a file
 * 
 * @param fname the name of the file
 */
int nlines( const char* fname ){
	FILE *fp;
	if((fp = _fopen(R_ExpandFileName( fname ), "r")) == NULL){
		error(_("unable to open file to read"), 0);
	}
	
	int c, previous = 0 ;
	int n = 0 ; 
#ifdef Win32
	while( (c = __fgetc(fp)) ){
#else
	while( (c = fgetc(fp)) ){
#endif
if( c ==  R_EOF ){
			break ;
		}
		if( c == '\n' ){
			n++ ;
		}
		previous = c ;
	}
	fclose( fp ) ;
	if( previous != '\n' ){
		n++;
	}
	return n ; 
}
/*}}}*/

/*{{{ do_countchars */
SEXP countchars( const char* fname, int nl){
	
	// to quiet warnings
	known_to_be_latin1 = known_to_be_latin1 ;
	
	SEXP result ;
	PROTECT( result = allocVector( INTSXP, nl*2) ) ;
	FILE *fp;
	if((fp = _fopen(R_ExpandFileName( fname ), "r")) == NULL){
		error(_("unable to open file to read"), 0);
	}
	int c ;
	int col = 0 ;
	int bytes = 0; 
	int i =0;
	while( (c = _fgetc(fp)) ){
		if( c ==  R_EOF ){
			break ;
		}
		if( c == '\n' ){
			INTEGER( result)[ i ] = col   ; col   = 0 ;
			INTEGER( result)[ i + nl] = bytes ; bytes = 0 ;
			i++;
		} else{
			col++ ;
			bytes++ ;
			
			if (0x80 <= (unsigned char)c && (unsigned char)c <= 0xBF && known_to_be_utf8){ 
		    	bytes--;
			}
			if (c == '\t'){
				col = ((col + 7) & ~7);
			}
			
		}
	}
	fclose( fp ) ;
	SEXP dims ;
	PROTECT( dims = allocVector( INTSXP, 2 ) ) ;
	INTEGER(dims)[0]=nl;
	INTEGER(dims)[1]=2;
	setAttrib( result, mkString( "dim" ), dims ) ;
	UNPROTECT( 2 ); // result, dim 
	return result ;

}


/*}}}*/

/* :tabSize=4:indentSize=4:noTabs=false:folding=explicit:collapseFolds=1: */

