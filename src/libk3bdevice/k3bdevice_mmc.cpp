/*
 *
 * Copyright (C) 2003-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

/**
   This file contains all the MMC command implementations of the K3b device class
   to make the code more readable.
**/


#include "k3bdevice.h"
#include "k3bscsicommand.h"
#include "k3bdeviceglobals.h"
#include "kdebug.h"

#include <string.h>


bool K3b::Device::Device::testUnitReady() const
{
    ScsiCommand cmd( this );
    cmd.enableErrorMessages( false );
    cmd[0] = MMC_TEST_UNIT_READY;
    cmd[5] = 0; // Necessary to set the proper command length
    return( cmd.transport() == 0 );
}


bool K3b::Device::Device::getFeature( unsigned char** data, unsigned int& dataLen, unsigned int feature ) const
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_GET_CONFIGURATION;
    cmd[1] = 2;      // read only specified feature
    cmd[2] = feature>>8;
    cmd[3] = feature;
    cmd[8] = 8;      // we only read the data length first
    cmd[9] = 0;      // Necessary to set the proper command length

    // we only read the data length first
    dataLen = 8;
    if( !cmd.transport( TR_DIR_READ, header, 8 ) )
        dataLen = from4Byte( header ) + 4;
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": GET CONFIGURATION length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data or something invalid altogether.
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( (dataLen-8) % 8 || dataLen <= 8 )
        dataLen = 0xFFFF;

    // again with real length
    *data = new unsigned char[dataLen];
    ::memset( *data, 0, dataLen );

    cmd[7] = dataLen>>8;
    cmd[8] = dataLen;
    if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
        dataLen = qMin( dataLen, from4Byte( *data ) + 4 );
        return true;
    }
    else {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": GET CONFIGURATION with real length "
                 << dataLen << " failed." << endl;
        delete [] *data;
        *data = 0;
    }

    return false;
}


int K3b::Device::Device::featureCurrent( unsigned int feature ) const
{
    unsigned char* data = 0;
    unsigned int dataLen = 0;
    if( getFeature( &data, dataLen, feature ) ) {
        int ret = -1;
        if( dataLen >= 11 )
            ret = ( data[8+2]&1 ? 1 : 0 );  // check the current flag

        delete [] data;

        return ret;
    }
    else
        return -1;
}


bool K3b::Device::Device::readIsrc( unsigned int track, QByteArray& isrc ) const
{
    unsigned char* data = 0;
    unsigned int dataLen = 0;

    if( readSubChannel( &data, dataLen, 0x3, track ) ) {
        bool isrcValid = false;

        if( dataLen >= 8+18 ) {
            isrcValid = (data[8+4]>>7 & 0x1);

            if( isrcValid ) {
                isrc = QByteArray( reinterpret_cast<char*>(data[8+5]), 13 );

                // TODO: check the range of the chars

            }
        }

        delete [] data;

        return isrcValid;
    }
    else
        return false;
}


bool K3b::Device::Device::readMcn( QByteArray& mcn ) const
{
    unsigned char* data = 0;
    unsigned int dataLen = 0;

    if( readSubChannel( &data, dataLen, 0x2, 0 ) ) {
        bool mcnValid = false;

        if( dataLen >= 8+18 ) {
            mcnValid = (data[8+4]>>7 & 0x1);

            if( mcnValid )
                mcn = QByteArray( reinterpret_cast<char*>(data[8+5]), 14 );
        }

        delete [] data;

        return mcnValid;
    }
    else
        return false;
}


bool K3b::Device::Device::getPerformance( unsigned char** data, unsigned int& dataLen,
                                          unsigned int type,
                                          unsigned int dataType,
                                          unsigned int lba ) const
{
    unsigned int descLen = 0;
    switch( type ) {
    case 0x0:
        descLen = 16;
        break;
    case 0x1:
        descLen = 8;
        break;
    case 0x2:
        descLen = 2048;
        break;
    case 0x3:
        descLen = 16;
        break;
    case 0x4:
        descLen = 8;
        break;
    case 0x5:
        descLen = 8; // FIXME: ??
        break;
    }

    dataLen = descLen + 8;
    unsigned char header[dataLen];
    ::memset( header, 0, dataLen );

    ScsiCommand cmd( this );
    cmd[0] = MMC_GET_PERFORMANCE;
    cmd[1] = dataType;
    cmd[2] = lba >> 24;
    cmd[3] = lba >> 16;
    cmd[4] = lba >> 8;
    cmd[5] = lba;
    cmd[9] = 1;      // first we read one descriptor
    cmd[10] = type;
    cmd[11] = 0;     // Necessary to set the proper command length
    if( cmd.transport( TR_DIR_READ, header, dataLen ) ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName()
                 << ": GET PERFORMANCE length det failed." << endl;
        return false;
    }

    dataLen = from4Byte( header ) + 4;

    // At least one Panasonic drive returns gigantic changing numbers for the data length
    // which makes K3b crash below when *data cannot be allocated. That's why we cut the
    // length here.
    // FIXME: 2048 is a proper upper boundary for the write speed but not for all
    //        return types. "Defect Status Data" for example might return way more data.
    // FIXME: Since we only use getPerformance for writing speeds and without a proper length
    //        those do not make sense it is better to fail here anyway.
    if( (dataLen-8) % descLen ||
        dataLen <= 8 ||
        dataLen > 2048 ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName()
                 << ": GET PERFORMANCE reports bogus dataLen: " << dataLen << endl;
        return false;
    }

    *data = new unsigned char[dataLen];
    ::memset( *data, 0, dataLen );

    unsigned int numDesc = (dataLen-8)/descLen;

    cmd[8] = numDesc>>8;
    cmd[9] = numDesc;
    if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
        dataLen = qMin( dataLen, from4Byte( *data ) + 4 );

        if( dataLen > 8 ) {
            return true;
        }
        else {
            kDebug() << "(K3b::Device::Device) " << blockDeviceName()
                    << ": GET PERFORMANCE reports invalid dataLen:" << dataLen << endl;
            delete [] *data;
            *data = 0;
            return false;
        }
    }
    else {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName()
                 << ": GET PERFORMANCE with real length "
                 << dataLen << " failed." << endl;
        delete [] *data;
        *data = 0;
        return false;
    }
}


bool K3b::Device::Device::setSpeed( unsigned int readingSpeed,
                                  unsigned int writingSpeed,
                                  bool cav ) const
{
    ScsiCommand cmd( this );
    cmd[0] = MMC_SET_SPEED;
    cmd[1] = ( cav ? 0x1 : 0x0 );
    cmd[2] = readingSpeed >> 8;
    cmd[3] = readingSpeed;
    cmd[4] = writingSpeed >> 8;
    cmd[5] = writingSpeed;
    cmd[11] = 0;      // Necessary to set the proper command length
    return ( cmd.transport( TR_DIR_WRITE ) == 0 );
}


bool K3b::Device::Device::seek( unsigned long lba ) const
{
    ScsiCommand cmd( this );
    cmd[0] = MMC_SEEK_10;
    cmd[2] = lba>>24;
    cmd[3] = lba>>16;
    cmd[4] = lba>>8;
    cmd[5] = lba;
    cmd[9] = 0;      // Necessary to set the proper command length
    return !cmd.transport();
}


bool K3b::Device::Device::readTrackInformation( unsigned char** data, unsigned int& dataLen, int type, int value ) const
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_TRACK_INFORMATION;
    cmd[9] = 0;      // Necessary to set the proper command length

    switch( type ) {
    case 0:
    case 1:
    case 2:
        cmd[1] = type & 0x3;
        cmd[2] = value>>24;
        cmd[3] = value>>16;
        cmd[4] = value>>8;
        cmd[5] = value;
        break;
    default:
        kDebug() << "(K3b::Device::readTrackInformation) wrong type parameter: " << type;
        return false;
    }

    // first we read the header
    dataLen = 4;
    cmd[8] = 4;
    if( cmd.transport( TR_DIR_READ, header, 4 ) == 0 )
        dataLen = from2Byte( header ) + 2;
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ TRACK INFORMATION length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data.
    // So we try to determine the correct size based on the medium type
    // DVD+R:  40 (MMC4)
    // DVD-DL: 48 (MMC5)
    // CD:     36 (MMC2)
    //
    if( dataLen <= 4 ) {
        int m = mediaType();
        if( m & (MEDIA_DVD_R_DL|MEDIA_DVD_R_DL_SEQ|MEDIA_DVD_R_DL_JUMP) )
            dataLen = 48;
        else if( m & (MEDIA_DVD_PLUS_R|MEDIA_DVD_PLUS_R_DL) )
            dataLen = 40;
        else
            dataLen = 36;
    }

    // again with real length
    *data = new unsigned char[dataLen];
    ::memset( *data, 0, dataLen );

    cmd[7] = dataLen>>8;
    cmd[8] = dataLen;
    if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
        dataLen = qMin( dataLen, from2Byte( *data ) + 2u );
        return true;
    }
    else {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ TRACK INFORMATION with real length "
                 << dataLen << " failed." << endl;
        delete [] *data;
        *data = 0;
    }

    return false;
}



bool K3b::Device::Device::read10( unsigned char* data,
                                unsigned int dataLen,
                                unsigned long startAdress,
                                unsigned int length,
                                bool fua ) const
{
    ::memset( data, 0, dataLen );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_10;
    cmd[1] = ( fua ? 0x8 : 0x0 );
    cmd[2] = startAdress>>24;
    cmd[3] = startAdress>>16;
    cmd[4] = startAdress>>8;
    cmd[5] = startAdress;
    cmd[7] = length>>8;
    cmd[8] = length;
    cmd[9] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, data, dataLen ) ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ 10 failed!";
        return false;
    }
    else
        return true;
}


bool K3b::Device::Device::read12( unsigned char* data,
                                unsigned int dataLen,
                                unsigned long startAdress,
                                unsigned long length,
                                bool streaming,
                                bool fua ) const
{
    ::memset( data, 0, dataLen );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_12;
    cmd[1] = ( fua ? 0x8 : 0x0 );
    cmd[2] = startAdress>>24;
    cmd[3] = startAdress>>16;
    cmd[4] = startAdress>>8;
    cmd[5] = startAdress;
    cmd[6] = length>>24;
    cmd[7] = length>>16;
    cmd[8] = length>>8;
    cmd[9] = length;
    cmd[10] = (streaming ? 0x80 : 0 );
    cmd[11] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, data, dataLen ) ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ 12 failed!";
        return false;
    }
    else
        return true;
}


bool K3b::Device::Device::readCd( unsigned char* data,
                                unsigned int dataLen,
                                int sectorType,
                                bool dap,
                                unsigned long startAdress,
                                unsigned long length,
                                bool sync,
                                bool header,
                                bool subHeader,
                                bool userData,
                                bool edcEcc,
                                int c2,
                                int subChannel ) const
{
    ::memset( data, 0, dataLen );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_CD;
    cmd[1] = (sectorType<<2 & 0x1c) | ( dap ? 0x2 : 0x0 );
    cmd[2] = startAdress>>24;
    cmd[3] = startAdress>>16;
    cmd[4] = startAdress>>8;
    cmd[5] = startAdress;
    cmd[6] = length>>16;
    cmd[7] = length>>8;
    cmd[8] = length;
    cmd[9] = ( ( sync      ? 0x80 : 0x0 ) |
               ( subHeader ? 0x40 : 0x0 ) |
               ( header    ? 0x20 : 0x0 ) |
               ( userData  ? 0x10 : 0x0 ) |
               ( edcEcc    ? 0x8  : 0x0 ) |
               ( c2<<1 & 0x6 ) );
    cmd[10] = subChannel & 0x7;
    cmd[11] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, data, dataLen ) ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ CD failed!";
        return false;
    }
    else {
        return true;
    }
}


bool K3b::Device::Device::readCdMsf( unsigned char* data,
                                   unsigned int dataLen,
                                   int sectorType,
                                   bool dap,
                                   const K3b::Msf& startAdress,
                                   const K3b::Msf& endAdress,
                                   bool sync,
                                   bool header,
                                   bool subHeader,
                                   bool userData,
                                   bool edcEcc,
                                   int c2,
                                   int subChannel ) const
{
    ::memset( data, 0, dataLen );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_CD_MSF;
    cmd[1] = (sectorType<<2 & 0x1c) | ( dap ? 0x2 : 0x0 );
    cmd[3] = (startAdress+150).minutes();
    cmd[4] = (startAdress+150).seconds();
    cmd[5] = (startAdress+150).frames();
    cmd[6] = (endAdress+150).minutes();
    cmd[7] = (endAdress+150).seconds();
    cmd[8] = (endAdress+150).frames();
    cmd[9] = ( ( sync      ? 0x80 : 0x0 ) |
               ( subHeader ? 0x40 : 0x0 ) |
               ( header    ? 0x20 : 0x0 ) |
               ( userData  ? 0x10 : 0x0 ) |
               ( edcEcc    ? 0x8  : 0x0 ) |
               ( c2<<1 & 0x6 ) );
    cmd[10] = subChannel & 0x7;
    cmd[11] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, data, dataLen ) ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ CD MSF failed!";
        return false;
    }
    else
        return true;
}


bool K3b::Device::Device::readSubChannel( unsigned char** data, unsigned int& dataLen,
                                        unsigned int subchannelParam,
                                        unsigned int trackNumber ) const
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_SUB_CHANNEL;
    cmd[2] = 0x40;    // SUBQ
    cmd[3] = subchannelParam;
    cmd[6] = trackNumber;   // only used when subchannelParam == 03h (ISRC)
    cmd[8] = 4;
    cmd[9] = 0;      // Necessary to set the proper command length

    // first we read the header
    dataLen = 4;
    if( cmd.transport( TR_DIR_READ, header, 4 ) == 0 )
        dataLen = from2Byte( &header[2] ) + 4;
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ SUB-CHANNEL length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data. So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( dataLen <= 4 )
        dataLen = 0xFFFF;

    // again with real length
    *data = new unsigned char[dataLen];
    ::memset( *data, 0, dataLen );

    cmd[7] = dataLen>>8;
    cmd[8] = dataLen;
    if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
        dataLen = qMin( dataLen, from2Byte( (*data)+2 ) + 4u );
        return true;
    }
    else {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ SUB-CHANNEL with real length "
                 << dataLen << " failed." << endl;
        delete [] *data;
        *data = 0;
    }

    return false;
}


bool K3b::Device::Device::readTocPmaAtip( unsigned char** data, unsigned int& dataLen, int format, bool time, int track ) const
{
    unsigned int descLen = 0;

    switch( format ) {
    case 0x0:
        descLen = 8;
        break;
    case 0x1:
        descLen = 8;
        break;
    case 0x2:
        descLen = 11;
        break;
    case 0x3:
        descLen = 11;
        break;
    case 0x4:
        descLen = 4; // MMC2: 24 and MMC4: 28, so we use the highest common factor
        break;
    case 0x5:
        descLen = 18;
        break;
    }

    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_TOC_PMA_ATIP;
    cmd[1] = ( time ? 0x2 : 0x0 );
    cmd[2] = format & 0x0F;
    cmd[6] = track;
    cmd[8] = 4;
    cmd[9] = 0;      // Necessary to set the proper command length

    // we only read the header
    dataLen = 4;
    if( cmd.transport( TR_DIR_READ, header, 4 ) == 0 )
        dataLen = from2Byte( header ) + 2;
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ TOC/PMA/ATIP length det failed.";

    //
    // Some buggy firmwares return an invalid size here
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( (dataLen-4) % descLen || dataLen < 4+descLen ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ TOC/PMA/ATIP invalid length returned: " << dataLen;
        dataLen = 0xFFFF;
    }

    //
    // Not all drives like uneven numbers
    //
    if( dataLen%2 )
        ++dataLen;

    // again with real length
    *data = new unsigned char[dataLen];
    ::memset( *data, 0, dataLen );

    cmd[7] = dataLen>>8;
    cmd[8] = dataLen;
    if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
        dataLen = qMin( dataLen, from2Byte( *data ) + 2u );
        if( (dataLen-4) % descLen || dataLen < 4+descLen ) {
            // useless length
            delete [] *data;
            *data = 0;
            return false;
        }
        else
            return true;
    }
    else {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ TOC/PMA/ATIP format "
                 << format << " with real length "
                 << dataLen << " failed." << endl;
        delete [] *data;
        *data = 0;
    }

    return false;
}


bool K3b::Device::Device::mechanismStatus( unsigned char** data, unsigned int& dataLen ) const
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_MECHANISM_STATUS;
    cmd[9] = 8;
    cmd[11] = 0;    // Necessary to set the proper command length

    // first we read the header
    dataLen = 8;
    if( cmd.transport( TR_DIR_READ, header, 8 ) == 0 )
        dataLen = from4Byte( &header[6] ) + 8;
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": MECHANISM STATUS length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data or something invalid altogether.
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( (dataLen-8) % 4 || dataLen <= 8 )
        dataLen = 0xFFFF;

    kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": MECHANISM STATUS "
             << (int)header[5] << " slots." << endl;

    // again with real length
    *data = new unsigned char[dataLen];
    ::memset( *data, 0, dataLen );

    cmd[8] = dataLen>>8;
    cmd[9] = dataLen;
    if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
        dataLen = qMin( dataLen, from4Byte( (*data)+6 ) + 8 );
        return true;
    }
    else {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": MECHANISM STATUS with real length "
                 << dataLen << " failed." << endl;
        delete [] *data;
        *data = 0;
    }

    return false;
}



bool K3b::Device::Device::modeSense( unsigned char** pageData, unsigned int& pageLen, int page ) const
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_MODE_SENSE;
    cmd[1] = 0x8;        // Disable Block Descriptors
    cmd[2] = page & 0x3F;
    cmd[8] = 8;
    cmd[9] = 0;           // Necessary to set the proper command length

    // first we determine the data length
    pageLen = 8;
    if( cmd.transport( TR_DIR_READ, header, 8 ) == 0 )
        pageLen = from2Byte( header ) + 2;
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": MODE SENSE length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data. So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( pageLen == 8 )
        pageLen = 0xFFFF;

    // again with real length
    *pageData = new unsigned char[pageLen];
    ::memset( *pageData, 0, pageLen );

    cmd[7] = pageLen>>8;
    cmd[8] = pageLen;
    if( cmd.transport( TR_DIR_READ, *pageData, pageLen ) == 0 ) {
        pageLen = qMin( pageLen, from2Byte( *pageData ) + 2u );
        return true;
    }
    else {
        delete [] *pageData;
        *pageData = 0;
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": MODE SENSE with real length "
                 << pageLen << " failed." << endl;
    }

    return false;
}


bool K3b::Device::Device::modeSelect( unsigned char* page, unsigned int pageLen, bool pf, bool sp ) const
{
    page[0] = 0;
    page[1] = 0;
    page[4] = 0;
    page[5] = 0;

    // we do not support Block Descriptors here
    page[6] = 0;
    page[7] = 0;

    // PS bit reserved
    page[8] &= 0x3F;

    ScsiCommand cmd( this );
    cmd[0] = MMC_MODE_SELECT;
    cmd[1] = ( sp ? 1 : 0 ) | ( pf ? 0x10 : 0 );
    cmd[7] = pageLen>>8;
    cmd[8] = pageLen;
    cmd[9] = 0;
    return( cmd.transport( TR_DIR_WRITE, page, pageLen ) == 0 );
}


// does only make sense for complete media
bool K3b::Device::Device::readCapacity( K3b::Msf& r ) const
{
    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_CAPACITY;
    cmd[9] = 0;      // Necessary to set the proper command length
    unsigned char buf[8];
    ::memset( buf, 0, 8 );
    if( cmd.transport( TR_DIR_READ, buf, 8 ) == 0 ) {
        r = from4Byte( buf );
        return true;
    }
    else
        return false;
}


bool K3b::Device::Device::readFormatCapacity( int wantedFormat, K3b::Msf& r,
                                              K3b::Msf* currentMax, int* currentMaxFormat ) const
{
    bool success = false;

    // the maximal length as stated in MMC4
    static const unsigned int maxLen = 4 + (8*32);

    unsigned char buffer[maxLen];
    ::memset( buffer, 0, maxLen );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_FORMAT_CAPACITIES;
    cmd[7] = maxLen >> 8;
    cmd[8] = maxLen & 0xFF;
    cmd[9] = 0;      // Necessary to set the proper command length
    if( cmd.transport( TR_DIR_READ, buffer, maxLen ) == 0 ) {

        unsigned int realLength = buffer[3] + 4;

        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << " READ FORMAT CAPACITY: Current/Max "
                 << (int)(buffer[8]&0x3) << " " << from4Byte( &buffer[4] ) << endl;

        if( currentMax )
            *currentMax = from4Byte( &buffer[4] );
        if( currentMaxFormat )
            *currentMaxFormat = (int)(buffer[8]&0x3);

        //
        // Descriptor Type:
        // 0 - reserved
        // 1 - unformatted :)
        // 2 - formatted. Here we get the used capacity (lead-in to last lead-out/border-out)
        // 3 - No media present
        //
        for( unsigned int i = 12; i < realLength-4; i+=8 ) {
            int format = (int)((buffer[i+4]>>2)&0x3f);
            kDebug() << "(K3b::Device::Device) " << blockDeviceName() << " READ FORMAT CAPACITY: "
                     << format << " " << from4Byte( &buffer[i] )
                     << " " << (int)( (buffer[i+5] << 16 & 0xFF0000) |
                                      (buffer[i+6] << 8  & 0xFF00) |
                                      (buffer[i+7]       & 0xFF) ) << endl;

            if( format == wantedFormat ) {
                // found the descriptor
                r = qMax( (int)from4Byte( &buffer[i] ), r.lba() );
                success = true;
            }
        }
    }

    return success;
}


bool K3b::Device::Device::readDiscInformation( unsigned char** data, unsigned int& dataLen ) const
{
    unsigned char header[2];
    ::memset( header, 0, 2 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_DISC_INFORMATION;
    cmd[8] = 2;
    cmd[9] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, header, 2 ) == 0 )
        dataLen = from2Byte( header ) + 2;
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName()
                 << ": READ DISC INFORMATION length det failed" << endl;

    if( dataLen < 32 ) {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName()
                 << ": Device reports bogus disc information length of " << dataLen << endl;
        dataLen = 32;
    }

    *data = new unsigned char[dataLen];
    ::memset( *data, 0, dataLen );

    cmd[7] = dataLen>>8;
    cmd[8] = dataLen;
    if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
        dataLen = qMin( dataLen, from2Byte( *data ) + 2u );
        return true;
    }
    else {
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ DISC INFORMATION with real length "
                 << dataLen << " failed." << endl;
        delete [] *data;
        *data = 0;
    }

    return false;
}


bool K3b::Device::Device::readDvdStructure( unsigned char** data, unsigned int& dataLen,
                                          unsigned int format,
                                          unsigned int layer,
                                          unsigned long address,
                                          unsigned int agid ) const
{
    return readDiscStructure( data, dataLen, 0x0, format, layer, address, agid );
}


bool K3b::Device::Device::readDiscStructure( unsigned char** data, unsigned int& dataLen,
                                           unsigned int mediaType,
                                           unsigned int format,
                                           unsigned int layer,
                                           unsigned long address,
                                           unsigned int agid ) const
{
    unsigned char header[4];
    ::memset( header, 0, 4 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_DVD_STRUCTURE;
    cmd[1] = mediaType & 0xF;
    cmd[2] = address>>24;
    cmd[3] = address>>16;
    cmd[4] = address>>8;
    cmd[5] = address;
    cmd[6] = layer;
    cmd[7] = format;
    cmd[10] = (agid<<6);
    cmd[11] = 0;      // Necessary to set the proper command length

    cmd[9] = 4;
    if( cmd.transport( TR_DIR_READ, header, 4 ) == 0 ) {
        // again with real length
        dataLen = from2Byte( header ) + 2;

        *data = new unsigned char[dataLen];
        ::memset( *data, 0, dataLen );

        cmd[8] = dataLen>>8;
        cmd[9] = dataLen;
        if( cmd.transport( TR_DIR_READ, *data, dataLen ) == 0 ) {
            dataLen = qMin( dataLen, from2Byte( *data ) + 2u );
            return true;
        }
        else {
            kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ DVD STRUCTURE with real length failed.";
            delete [] *data;
            *data = 0;
        }
    }
    else
        kDebug() << "(K3b::Device::Device) " << blockDeviceName() << ": READ DVD STRUCTURE length det failed";

    return false;
}


int K3b::Device::Device::readBufferCapacity( long long& bufferLength, long long& bufferAvail ) const
{
    unsigned char data[12];
    ::memset( data, 0, 12 );

    ScsiCommand cmd( this );
    cmd[0] = MMC_READ_BUFFER_CAPACITY;
    cmd[8] = 12;
    cmd[9] = 0;      // Necessary to set the proper command length
    int r = cmd.transport( TR_DIR_READ, data, 12 );
    if( r )
        return r;

    unsigned int dataLength = from2Byte( data );

    if( dataLength >= 10 ) {
        bufferLength = from4Byte( &data[4] );
        bufferAvail = from4Byte( &data[8] );
    }
    else {
        bufferAvail = bufferLength = 0;
    }

    return 0;
}
