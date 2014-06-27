//

#include <QtGui>

#include "astrocalculator.h"

AstroCalculator::AstroCalculator(QWidget *parent)
: QDialog(parent)
, Height(20)
{
	setupUi(this);
    setDateNow();
}

void AstroCalculator::setDateNow()
{
    // TODO: slot for ischecked
    if (LocalTimecheckBox->isChecked()) {
        dateTimeEdit->setDateTime( QDateTime::currentDateTime() );
    } else {
        dateTimeEdit->setDateTime( QDateTime::currentDateTime().toUTC() );
    }
	updateAll();
}

void AstroCalculator::updateWeekday()
{
	switch ( date.DayOfWeek() ) {
	case CAADate::SUNDAY :
		WeekdayLabel->setText("Sunday");
		break;
	case CAADate::MONDAY :
		WeekdayLabel->setText("Monday");
		break;
	case CAADate::TUESDAY :
		WeekdayLabel->setText("Tuesday");
		break;
	case CAADate::WEDNESDAY :
		WeekdayLabel->setText("Wednesday");
		break;
	case CAADate::THURSDAY :
		WeekdayLabel->setText("Thursday");
		break;
	case CAADate::FRIDAY :
		WeekdayLabel->setText("Friday");
		break;
	case CAADate::SATURDAY :
		WeekdayLabel->setText("Saturday");
		break;
	default:
		WeekdayLabel->setText("Error");
		break;
		;
	}
}

void AstroCalculator::setHorizontal(QLabel * labelX, QLabel* labelY, double X, double Y)  {
    QString temp_str = "A = " + QString::number(X) ;
    labelX->setText(temp_str);

    temp_str = "h = " + QString::number(Y) ;
    labelY->setText(temp_str);
}

void AstroCalculator::riseSetSun(QDate date)  {
    // to calculate rise/transit/set time at 0hUT must be used, see page 102
    CAADate date_0hUT(date.year(), date.month(), date.day(), 0,0,0,true);
    double JD = date_0hUT.Julian();

    CAAEllipticalPlanetaryDetails ObjectDetails = CAAElliptical::Calculate(JD - 1, CAAElliptical::SUN);
    double Alpha1 = ObjectDetails.ApparentGeocentricRA;
    double Delta1 = ObjectDetails.ApparentGeocentricDeclination;
    ObjectDetails = CAAElliptical::Calculate(JD, CAAElliptical::SUN);
    double Alpha2 = ObjectDetails.ApparentGeocentricRA;
    double Delta2 = ObjectDetails.ApparentGeocentricDeclination;
    ObjectDetails = CAAElliptical::Calculate(JD + 1, CAAElliptical::SUN);
    double Alpha3 = ObjectDetails.ApparentGeocentricRA;
    double Delta3 = ObjectDetails.ApparentGeocentricDeclination;

    CAARiseTransitSetDetails RiseTransitSetTime = CAARiseTransitSet::Calculate(JD, Alpha1, Delta1, Alpha2, Delta2, Alpha3, Delta3, longitudeInput->value(), latitudeInput->value(), -0.8333);

    double riseJD = (JD + (RiseTransitSetTime.Rise / 24.00));
    CAADate RiseDate(riseJD,true);
    double setJD = (JD + (RiseTransitSetTime.Set / 24.00));
    CAADate SetDate(setJD,true);
    double transitJD = (JD + (RiseTransitSetTime.Transit / 24.00));
    CAADate transitDate(transitJD,true);

    // Update GUI
    QString temp_str;
    temp_str = "Rise \t" + QString::number(RiseDate.Hour()) + "h"
            + QString::number(RiseDate.Minute()) + "m";
    SunRiseTime->setText(temp_str);

    temp_str = "Set \t" + QString::number(SetDate.Hour()) + "h"
            + QString::number(SetDate.Minute()) + "m";
    SunSetTime->setText(temp_str);

    temp_str = "Transit \t" + QString::number(transitDate.Hour()) + "h"
            + QString::number(transitDate.Minute()) + "m";
    SunTransitTime->setText(temp_str);
}

void AstroCalculator::updateHorizontalCoordSun()
{
    // calculate horizontal coord.
    double JD = date.Julian() + CAADynamicalTime::DeltaT(date.Julian()) / 86400.0;
    double SunLong = CAASun::ApparentEclipticLongitude(JD);
    double SunLat = CAASun::ApparentEclipticLatitude(JD);
    CAA2DCoordinate Equatorial = CAACoordinateTransformation::Ecliptic2Equatorial(SunLong, SunLat, CAANutation::TrueObliquityOfEcliptic(JD));
    double SunRad = CAAEarth::RadiusVector(JD);
    CAA2DCoordinate SunTopo = CAAParallax::Equatorial2Topocentric(Equatorial.X, Equatorial.Y, SunRad, longitudeInput->value(), latitudeInput->value(), Height, JD);
    double AST = CAASidereal::ApparentGreenwichSiderealTime(date.Julian());
    double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours(longitudeInput->value());
    double LocalHourAngle = AST - LongtitudeAsHourAngle - SunTopo.X;
    CAA2DCoordinate SunHorizontal = CAACoordinateTransformation::Equatorial2Horizontal(LocalHourAngle, SunTopo.Y, latitudeInput->value());
    SunHorizontal.Y += CAARefraction::RefractionFromTrue(SunHorizontal.Y, 1013, 10);

    QString temp_str = "h = \t" + QString::number(SunHorizontal.Y,'g',4) ;
    SunAltitude->setText(temp_str);

    temp_str = "A = \t" + QString::number(SunHorizontal.X) ;
    SunAzimuth->setText(temp_str);

    // http://www.unicode.org/charts/
    temp_str = QString(QChar(0x03b1)) + " = \t" + QString::number(Equatorial.X) ;
    SunRightAscension->setText(temp_str);

    temp_str = QString(QChar(0x03b4)) + " = \t" + QString::number(Equatorial.Y) ;
    SunDeclination->setText(temp_str);

    SunElongation->setText(QString(QChar(0x03c8)) + " = \t");
}

void AstroCalculator::updateSun()  {
    SunLabel->setText("\tSun");

    riseSetSun(dateTimeEdit->date());
    updateHorizontalCoordSun();
}

void AstroCalculator::riseSetPlanet(QDate date)  {
    // to calculate rise/transit/set time at 0hUT must be used, see page 102
    CAADate date_0hUT(date.year(), date.month(), date.day(), 0,0,0,true);
    double JD = date_0hUT.Julian();

    CAAElliptical::EllipticalObject planet;
    QString temp_str;
    // Init labels
    PlanetLabel->setText("");
    PlanetRiseTime->setText("");
    PlanetSetTime->setText("");
    PlanetTransitTime->setText("");
    for (int i=0; i<9; i++)  {
        planet = static_cast<CAAElliptical::EllipticalObject> (i);

        switch ( planet ) {
        case CAAElliptical::MERCURY:
            temp_str = PlanetLabel->text() + "Mercury\t";
            PlanetLabel->setText(temp_str);
            break;
        case CAAElliptical::VENUS:
            temp_str = PlanetLabel->text() + "Venus\t";
            PlanetLabel->setText(temp_str);
            break;
        case CAAElliptical::MARS:
            temp_str = PlanetLabel->text() + "Mars\t";
            PlanetLabel->setText(temp_str);
            break;
        case CAAElliptical::JUPITER:
            temp_str = PlanetLabel->text() + "Jupiter\t";
            PlanetLabel->setText(temp_str);
            break;
        case CAAElliptical::SATURN:
            temp_str = PlanetLabel->text() + "Saturn\t";
            PlanetLabel->setText(temp_str);
            break;
        default:
            break;
        }

        CAAEllipticalPlanetaryDetails ObjectDetails = CAAElliptical::Calculate(JD - 1, planet);
        double Alpha1 = ObjectDetails.ApparentGeocentricRA;
        double Delta1 = ObjectDetails.ApparentGeocentricDeclination;
        ObjectDetails = CAAElliptical::Calculate(JD, planet);
        double Alpha2 = ObjectDetails.ApparentGeocentricRA;
        double Delta2 = ObjectDetails.ApparentGeocentricDeclination;
        ObjectDetails = CAAElliptical::Calculate(JD + 1, planet);
        double Alpha3 = ObjectDetails.ApparentGeocentricRA;
        double Delta3 = ObjectDetails.ApparentGeocentricDeclination;

        CAARiseTransitSetDetails RiseTransitSetTime = CAARiseTransitSet::Calculate(JD, Alpha1, Delta1, Alpha2, Delta2, Alpha3, Delta3, longitudeInput->value(), latitudeInput->value(), -0.5667);

        double riseJD = (JD + (RiseTransitSetTime.Rise / 24.00));
        CAADate RiseDate(riseJD,true);
        double setJD = (JD + (RiseTransitSetTime.Set / 24.00));
        CAADate SetDate(setJD,true);
        double transitJD = (JD + (RiseTransitSetTime.Transit / 24.00));
        CAADate transitDate(transitJD,true);

        // Update GUI
        temp_str = PlanetRiseTime->text()
                + QString("%1h%2m\t").arg(RiseDate.Hour()).arg(RiseDate.Minute(),2) ;
        PlanetRiseTime->setText(temp_str);

        temp_str = PlanetSetTime->text()
                + QString::number(SetDate.Hour()) + "h"
                + QString::number(SetDate.Minute()) + "m\t";
        PlanetSetTime->setText(temp_str);

        temp_str = PlanetTransitTime->text()
                + QString::number(transitDate.Hour()) + "h"
                + QString::number(transitDate.Minute()) + "m\t";
        PlanetTransitTime->setText(temp_str);
    }

}

void AstroCalculator::updateHorizontalCoordPlanet()
{
    // calculate horizontal coord.
    double JD = date.Julian() + CAADynamicalTime::DeltaT(date.Julian()) / 86400.0;

    CAAEllipticalPlanetaryDetails PlanetDetails;

    // Init labels
    PlanetAltitude->setText("");
    PlanetAzimuth->setText("");
    PlanetRightAscension->setText("");
    PlanetDeclination->setText("");
    PlanetElongation->setText("");
    double SunLong = CAASun::ApparentEclipticLongitude(JD);

    for (int i=0; i<9; i++)  {
        CAAElliptical::EllipticalObject planet = static_cast<CAAElliptical::EllipticalObject> (i);
        PlanetDetails = CAAElliptical::Calculate(date.Julian(), planet);
        double AST = CAASidereal::ApparentGreenwichSiderealTime(date.Julian());
        double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours(longitudeInput->value());

        double Alpha = PlanetDetails.ApparentGeocentricRA ;
        double LocalHourAngle = AST - LongtitudeAsHourAngle - Alpha;
        CAA2DCoordinate Horizontal = CAACoordinateTransformation::Equatorial2Horizontal(LocalHourAngle, PlanetDetails.ApparentGeocentricDeclination, latitudeInput->value() );

        QString temp_str = PlanetAltitude->text() + QString::number(Horizontal.Y,'g',4) + "\t";
        PlanetAltitude->setText(temp_str);

        temp_str = PlanetAzimuth->text() + QString::number(Horizontal.X) + "\t";
        PlanetAzimuth->setText(temp_str);

        temp_str = PlanetRightAscension->text() + QString::number(PlanetDetails.ApparentGeocentricRA) + "\t";
        PlanetRightAscension->setText(temp_str);

        temp_str = PlanetDeclination->text() + QString::number(PlanetDetails.ApparentGeocentricDeclination) + "\t";
        PlanetDeclination->setText(temp_str);

        double PlanetElong = acos(
                cos(CAACoordinateTransformation::DegreesToRadians(PlanetDetails.ApparentGeocentricLatitude))
                * cos(CAACoordinateTransformation::DegreesToRadians(PlanetDetails.ApparentGeocentricLongitude - SunLong)
                      ));
        PlanetElong = CAACoordinateTransformation::RadiansToDegrees(PlanetElong);
        if (PlanetDetails.ApparentGeocentricLongitude < SunLong) {
            PlanetElong = -PlanetElong;
        }
        temp_str = PlanetElongation->text() + QString::number(PlanetElong,'g',4) + "\t";
        PlanetElongation->setText(temp_str);
    }
}

void AstroCalculator::updatePlanet()  {
    riseSetPlanet(dateTimeEdit->date());
    updateHorizontalCoordPlanet();
}

void AstroCalculator::LunarRaDec(double JD, double& RA, double& Dec)
{
    double Lambda = CAAMoon::EclipticLongitude(JD);
    double Beta = CAAMoon::EclipticLatitude(JD);
    double Epsilon = CAANutation::TrueObliquityOfEcliptic(JD);
    CAA2DCoordinate Lunarcoord = CAACoordinateTransformation::Ecliptic2Equatorial(Lambda, Beta, Epsilon);

    RA = Lunarcoord.X;
    Dec = Lunarcoord.Y;
}

void AstroCalculator::updateMoon()  {
	CAADate date_0hUT(dateTimeEdit->date().year(), dateTimeEdit->date().month(), dateTimeEdit->date().day(), 0,0,0,true);
    double JD = date_0hUT.Julian();

    double Alpha1;
    double Alpha2;
    double Alpha3;
    double Delta1;
    double Delta2;
    double Delta3;
    LunarRaDec(JD - 1, Alpha1, Delta1);
    LunarRaDec(JD, Alpha2, Delta2);
    LunarRaDec(JD + 1, Alpha3, Delta3);

    double Longitude = longitudeInput->value();
    double Latitude = latitudeInput->value();
    CAARiseTransitSetDetails RiseTransitSetTime = CAARiseTransitSet::Calculate(JD, Alpha1, Delta1, Alpha2, Delta2, Alpha3, Delta3, Longitude, Latitude, 0.125);

    double riseJD = (JD + (RiseTransitSetTime.Rise / 24.00));
    CAADate RiseDate(riseJD,true);
    double setJD = (JD + (RiseTransitSetTime.Set / 24.00));
    CAADate SetDate(setJD,true);
    double transitJD = (JD + (RiseTransitSetTime.Transit / 24.00));
    CAADate transitDate(transitJD,true);

    // Init labels
    MoonLabel->setText("Moon");
    QString temp_str;

    if (RiseTransitSetTime.bRiseValid) {
        temp_str = QString("%1h%2m\t").arg(RiseDate.Hour()).arg(RiseDate.Minute(),2) ;
		MoonRiseTime->setText(temp_str);
	} else {
        temp_str = "Does not rise";
		MoonRiseTime->setText(temp_str);
	}

    temp_str = QString::number(transitDate.Hour()) + "h"
            + QString::number(transitDate.Minute()) + "m\t";
    MoonTransitTime->setText(temp_str);

    if (RiseTransitSetTime.bSetValid) {
        temp_str = QString::number(SetDate.Hour()) + "h"
                + QString::number(SetDate.Minute()) + "m\t";
        MoonSetTime->setText(temp_str);
    }  else  {
        temp_str = "Does not set";
		MoonSetTime->setText(temp_str);
	}

	//Calculate the topocentric horizontal position of the Moon 
	double JDMoon = date.Julian() + CAADynamicalTime::DeltaT(date.Julian()) / 86400.0;  
	double MoonLong = CAAMoon::EclipticLongitude(JDMoon);
	double MoonLat = CAAMoon::EclipticLatitude(JDMoon);
	CAA2DCoordinate Equatorial = CAACoordinateTransformation::Ecliptic2Equatorial(MoonLong, MoonLat, CAANutation::TrueObliquityOfEcliptic(JDMoon));
	double MoonRad = CAAMoon::RadiusVector(JDMoon);
	MoonRad /= 149597870.691; //Convert KM to AU
	CAA2DCoordinate MoonTopo = CAAParallax::Equatorial2Topocentric(Equatorial.X, Equatorial.Y, MoonRad, Longitude, Latitude, Height, JDMoon);
	double AST = CAASidereal::ApparentGreenwichSiderealTime(date.Julian());
	double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours(Longitude);
	double LocalHourAngle = AST - LongtitudeAsHourAngle - MoonTopo.X;
	CAA2DCoordinate MoonHorizontal = CAACoordinateTransformation::Equatorial2Horizontal(LocalHourAngle, MoonTopo.Y, Latitude);
	MoonHorizontal.Y += CAARefraction::RefractionFromTrue(MoonHorizontal.Y, 1013, 10);

    temp_str = QString::number(MoonHorizontal.X) ;
    MoonAzimuth->setText(temp_str);

    temp_str = QString::number(MoonHorizontal.Y,'g',4) ;
	MoonAltitude->setText(temp_str);

    temp_str = QString::number(Equatorial.X) ;
    MoonRightAscension->setText(temp_str);

    temp_str = QString::number(Equatorial.Y) ;
    MoonDeclination->setText(temp_str);

    double SunLong = CAASun::ApparentEclipticLongitude(JD);
    double MoonElong = acos(
            cos(CAACoordinateTransformation::DegreesToRadians(MoonLat))
            * cos(CAACoordinateTransformation::DegreesToRadians(MoonLong - SunLong)
                  ));
    MoonElong = CAACoordinateTransformation::RadiansToDegrees(MoonElong);

//    double MoonElong = CAAMoon::MeanElongation(JDMoon); // 'Mean'
    temp_str = QString::number(MoonElong) + "\t";
    MoonElongation->setText(temp_str);
}

void AstroCalculator::updateAll()  {

    // Update current time
	date.Set(dateTimeEdit->date().year(), dateTimeEdit->date().month(), dateTimeEdit->date().day(), 
		dateTimeEdit->time().hour(), dateTimeEdit->time().minute(), dateTimeEdit->time().second() ,true);

    QString temp_str = "JD: " + QString::number(date.Julian(), 'f');
	JDLabel->setText(temp_str);

    double AST = CAASidereal::ApparentGreenwichSiderealTime(date.Julian());
    double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours(longitudeInput->value());
    double LocalHourAngle = AST - LongtitudeAsHourAngle;
    int D,M; double S;
    DMS(LocalHourAngle, D, M, S);
    temp_str = QString(QChar(0x03b8)) + ": "
            + QString::number(D) + "h"
            + QString::number(M) + "m"
            + QString::number(S, 'f', 2) + "s";
    SiderealLabel->setText(temp_str);

    // other methods to update
	updateWeekday();
    updateSun();
    updateMoon();
    updatePlanet();
}

void AstroCalculator::updateLongLat()
{
    // 0 Turnhout -4.95, 51.32
    // 1 Ukkel -4.333248, 50.799968
    // 2 Utrecht -5.129, 52.086
	switch ( citySelection->currentIndex() ) {
    case 0:
        longitudeInput->setValue(-4.95);
        latitudeInput->setValue(51.32);
		break;
    case 1:
        longitudeInput->setValue(-4.333248);
        latitudeInput->setValue(50.799968);
        break;
    case 2:
		longitudeInput->setValue(-5.129);
		latitudeInput->setValue(52.086);
		break;
	default:
		break;
	}
	updateAll();
}

//PA chap 8
void AstroCalculator::DMS (double Dd, int& D, int& M, double& S)
{
  //
  // Variables
  //
  double x;


  x = fabs(Dd);   D = int(x);
  x = (x-D)*60.0; M = int(x);  S = (x-M)*60.0;

  if (Dd<0.0) { if (D!=0) D*=-1; else if (M!=0) M*=-1; else S*=-1.0; }
}
