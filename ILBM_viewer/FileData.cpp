#include "FileData.h"
#include "utility.h"


IFFReader::File::File( const string& path ) : 
	path_( path ), size_( 0 ), type_( IFFReader::IFF_T::FORM_NOT_FOUND )
{
	stream_.open( path, std::ios::binary );
	
	if ( !stream_.is_open() ) {
		return;
	}

	try {
		if ( read_tag(stream_) != "FORM" ) {
			type_ = IFFReader::IFF_T::UNREADABLE;
			return;
		}

		size_ = read_long( stream_ );
		const auto tag = read_tag( stream_ );

		// Make more interesting later.
		if ( tag == "ILBM" ) {
			asILBM_ = shared_ptr<ILBM>( new ILBM(stream_) );
			type_ = IFFReader::IFF_T::ILBM;
		}
	}
	catch (...) {  // Filthy. Add proper exceptions later.
		type_ = IFFReader::IFF_T::UNREADABLE; // Abort if file malformed or missing.
	}
}


// Returns blank pointer if file was not parsed. Have fun with that.
shared_ptr<IFFReader::ILBM> IFFReader::File::AsILBM() const
{
	if ( type_ != IFFReader::IFF_T::ILBM ) {
		return shared_ptr< IFFReader::ILBM >();
	}
	return asILBM_;
}


const IFFReader::IFF_T IFFReader::File::GetType() const
{
	return type_; 
}
