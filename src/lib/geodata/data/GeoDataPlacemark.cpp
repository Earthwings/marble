//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>"
//


// Own
#include "GeoDataPlacemark.h"

// Qt
#include <QtCore/QDataStream>
#include <QtCore/QDebug>

class GeoDataPlacemarkPrivate
{
  public:
    GeoDataPlacemarkPrivate()
      : m_geometry( 0 ),
	m_area( -1.0 ),
	m_population( -1 )
    {
    }

    ~GeoDataPlacemarkPrivate()
    {
        if( m_geometry ) delete m_geometry;
    }

    // Data for a Placemark in addition to those in GeoDataFeature.
    GeoDataGeometry*    m_geometry;     // any GeoDataGeometry entry like locations
    GeoDataPoint        m_coordinate;     // The geographic position
    QString             m_countrycode;  // Country code.
    double              m_area;         // Area in square kilometer
    qint64              m_population;   // population in number of inhabitants
};


GeoDataPlacemark::GeoDataPlacemark()
    : GeoDataFeature(),
      d( new GeoDataPlacemarkPrivate )
{
}

GeoDataPlacemark::GeoDataPlacemark( const QString& name )
    : GeoDataFeature( name ),
      d( new GeoDataPlacemarkPrivate )
{
}

GeoDataPlacemark::~GeoDataPlacemark()
{
#if DEBUG_GEODATA
    qDebug() << "delete Placemark";
#endif
    delete d;
}

GeoDataGeometry* GeoDataPlacemark::geometry()
{
    return d->m_geometry;
}

GeoDataPoint GeoDataPlacemark::coordinate() const
{
    return d->m_coordinate;
}

void GeoDataPlacemark::coordinate( double& lon, double& lat, double& alt )
{
    d->m_coordinate.geoCoordinates( lon, lat );
    alt = d->m_coordinate.altitude();
}

void GeoDataPlacemark::setCoordinate( double lon, double lat, double alt )
{
    d->m_coordinate = GeoDataPoint( lon, lat, alt );
}

void GeoDataPlacemark::setCoordinate( const GeoDataPoint &point )
{
    d->m_coordinate = GeoDataPoint( point );
}

void GeoDataPlacemark::setGeometry( GeoDataPoint *point )
{
    delete d->m_geometry;
    d->m_geometry = point;
}

void GeoDataPlacemark::setGeometry( GeoDataLineString *point )
{
    delete d->m_geometry;
    d->m_geometry = point;
}

void GeoDataPlacemark::setGeometry( GeoDataLinearRing *point )
{
    delete d->m_geometry;
    d->m_geometry = point;
}

void GeoDataPlacemark::setGeometry( GeoDataPolygon *point )
{
    delete d->m_geometry;
    d->m_geometry = point;
}

void GeoDataPlacemark::setGeometry( GeoDataMultiGeometry *point )
{
    delete d->m_geometry;
    d->m_geometry = point;
}

double GeoDataPlacemark::area() const
{
    return d->m_area;
}

void GeoDataPlacemark::setArea( double area )
{
    d->m_area = area;
}

qint64 GeoDataPlacemark::population() const
{
    return d->m_population;
}

void GeoDataPlacemark::setPopulation( qint64 population )
{
    d->m_population = population;
}

const QString GeoDataPlacemark::countryCode() const
{
    return d->m_countrycode;
}

void GeoDataPlacemark::setCountryCode( const QString &countrycode )
{
    d->m_countrycode = countrycode;
}

void GeoDataPlacemark::pack( QDataStream& stream ) const
{
    GeoDataFeature::pack( stream );

    stream << d->m_countrycode;
    stream << d->m_area;
    stream << d->m_population;

    stream << d->m_geometry->geometryId();
    d->m_geometry->pack( stream );
    d->m_coordinate.pack( stream );
}


void GeoDataPlacemark::unpack( QDataStream& stream )
{
    GeoDataFeature::unpack( stream );

    stream >> d->m_countrycode;
    stream >> d->m_area;
    stream >> d->m_population;

    int geometryId;
    stream >> geometryId;
    switch( geometryId ) {
        case GeoDataGeometryId:
            break;
        case GeoDataPointId:
            {
            GeoDataPoint* point = new GeoDataPoint();
            point->unpack( stream );
            delete d->m_geometry;
            d->m_geometry = point;
            }
            break;
        case GeoDataLineStringId:
            {
            GeoDataLineString* lineString = new GeoDataLineString();
            lineString->unpack( stream );
            delete d->m_geometry;
            d->m_geometry = lineString;
            }
            break;
        case GeoDataLinearRingId:
            {
            GeoDataLinearRing* linearRing = new GeoDataLinearRing();
            linearRing->unpack( stream );
            delete d->m_geometry;
            d->m_geometry = linearRing;
            }
            break;
        case GeoDataPolygonId:
            {
            GeoDataPolygon* polygon = new GeoDataPolygon();
            polygon->unpack( stream );
            delete d->m_geometry;
            d->m_geometry = polygon;
            }
            break;
        case GeoDataMultiGeometryId:
            {
            GeoDataMultiGeometry* multiGeometry = new GeoDataMultiGeometry();
            multiGeometry->unpack( stream );
            delete d->m_geometry;
            d->m_geometry = multiGeometry;
            }
            break;
        case GeoDataModelId:
            break;
        default: break;
    };
    d->m_coordinate.unpack( stream );
}
