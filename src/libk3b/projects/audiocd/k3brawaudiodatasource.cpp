/*
 *
 * Copyright (C) 2009 Sebastian Trueg <trueg@k3b.org>
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

#include "k3brawaudiodatasource.h"

#include <QtCore/QFile>

#include <KLocale>


class K3b::RawAudioDataSource::Private
{
public:
    QFile imageFile;

    bool openImageFile() {
        return imageFile.open( QIODevice::ReadOnly );
    }
};


K3b::RawAudioDataSource::RawAudioDataSource()
    : AudioDataSource(),
      d( new Private() )
{
}


K3b::RawAudioDataSource::RawAudioDataSource( const QString& path )
    : AudioDataSource(),
      d( new Private() )
{
    d->imageFile.setFileName( path );
}


K3b::RawAudioDataSource::RawAudioDataSource( const RawAudioDataSource& other )
    : AudioDataSource( other ),
      d( new Private() )
{
    d->imageFile.setFileName( other.d->imageFile.fileName() );
}


K3b::RawAudioDataSource::~RawAudioDataSource()
{
    delete d;
}


K3b::Msf K3b::RawAudioDataSource::originalLength() const
{
    return Msf::fromAudioBytes( d->imageFile.size() );
}


bool K3b::RawAudioDataSource::seek( const Msf& pos )
{
    return d->imageFile.seek( pos.audioBytes() );
}


int K3b::RawAudioDataSource::read( char* data, unsigned int max )
{
    return d->imageFile.read( data, max );
}


QString K3b::RawAudioDataSource::type() const
{
    return i18n( "Raw Audio CD Image" );
}


QString K3b::RawAudioDataSource::sourceComment() const
{
    return type();
}


K3b::AudioDataSource* K3b::RawAudioDataSource::copy() const
{
    return new RawAudioDataSource( *this );
}
