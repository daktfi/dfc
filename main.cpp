#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <string>

using uint128_t = unsigned __int128;

template <typename K>
typename std::enable_if<std::is_same<K, uint8_t>::value, std::string>::type hex( const K &v )
{
	char s[3];

	sprintf( s, "%02X", v );
	return std::string( s );
}

template <typename K>
typename std::enable_if<std::is_same<K, uint16_t>::value, std::string>::type hex( const K &v )
{
	char s[5];

	sprintf( s, "%04X", v );
	return std::string( s );
}

template <typename K>
typename std::enable_if<std::is_same<K, uint32_t>::value, std::string>::type hex( const K &v )
{
	char s[9];

	sprintf( s, "%08X", v );
	return std::string( s );
}

template <typename K>
typename std::enable_if<std::is_same<K, uint64_t>::value, std::string>::type hex( const K &v )
{
	char s[17];

	sprintf( s, "%016lX", v );
	return std::string( s );
}

template <typename K>
typename std::enable_if<std::is_same<K, uint128_t>::value, std::string>::type hex( const K &v )
{
	uint64_t h = static_cast<uint64_t>( v >> 64 ), l = static_cast<uint64_t>( v );

	return hex( h ) + hex( l );
}

template <typename K>
std::string empty( void )
{
	std::string s;

	for( int i = 0; i < sizeof( K ); ++i )
		s += "--";

	return s;
}

template <typename K>
int compare_files( int n, char **fnames )
{
	if( n < 2 )
		return 2;

	FILE *fp[n];

	for( int i = 0; i < n; ++i ) {
		fp[i] = fopen( fnames[i], "rb" );

		if( fp[i] == nullptr )
			return 3;
	}

	bool finished = false, differs = false, diff_len, equal, done[n];
	size_t pos = 0;
	K val[n];

	do {
		diff_len = true;

		for( int i = 0; i < n; ++i ) {
			done[i] = fread( &val[i], sizeof( K ), 1, fp[i] ) < 1;
			finished = finished || done[i];
			diff_len = diff_len && done[i];
		}

		equal = true;

		for( int i = 1; i < n && equal; ++i )
			equal = equal && ( val[i] == val[0] );

		if( !equal && !diff_len ) {
			differs = true;
			std::cout << hex( pos ) << "/" << pos / sizeof( K );

			for( int i = 0; i < n; ++i )
				if( done[i])
					std::cout << " " + empty<K>();
				else
					std::cout << " " + hex( val[i] );

			std::cout << std::endl;
		}

		if( !finished )
			pos += sizeof( K );
	} while( !finished );


	for( int i = 0; i < n; ++i )
		if( !done[i] ) {
			differs = true;
			std::cout << "At " << hex( pos ) << "/" << pos / sizeof( K )
			          << " some files are of different length" << std::endl;
			break;
		}

	if( !differs )
		std::cout << "At " << hex( pos ) << "/" << pos / sizeof( K )
		          << " files seems to be identical" << std::endl;

	return 0;
}

int main(int argc, char *argv[])
{
	if( argc > 2 ) {
		char flag = 'b', **fnames = argv + 1;
		int num = argc - 1, rc;

		if( **fnames == '-' ) {
			flag = (*fnames)[1] | 0x20;	// Adjust case to lower.
			++fnames;
			--num;
		}

		switch( flag ) {
			case 'b':	// Byte
				rc = compare_files<uint8_t>( num, fnames );
				break;
			case 'w':	// Word
				rc = compare_files<uint16_t>( num, fnames );
				break;
			case 'd':	// DWord
				rc = compare_files<uint32_t>( num, fnames );
				break;
			case 'q':	// QWord
				rc = compare_files<uint64_t>( num, fnames );
				break;
			case 'h':	// 8Word
				rc = compare_files<uint128_t>( num, fnames );
				break;
			default:
				rc = 1;
		}

		if( rc == 0 )
			return rc;
	}

	printf( "USAGE: dfc [-{b|w|d|q|h}] <file1> <file2> ... <fileN>\n" );
	return 0;
}
