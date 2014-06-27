#ifndef ASTOCALCULATOR_H
#define ASTOCALCULATOR_H

#include <QDialog>

#include "ui_astrocalculator.h"

#include "stdafx.h"
#include "AA+.h"

class AstroCalculator : public QDialog, public Ui::AstroCalculator
{
    Q_OBJECT

public:
    AstroCalculator(QWidget *parent = 0);

private:
	CAADate date;
    double Height;

    void riseSetSun(QDate date);
    void updateHorizontalCoordSun();
    void updateSun();
    void updateHorizontalCoordPlanet();
    void riseSetPlanet(QDate date);
    void updatePlanet();
    void LunarRaDec(double JD, double& RA, double& Dec);
    void updateMoon();
    void updateWeekday();
    void setHorizontal(QLabel * labelX, QLabel *labelY, double X, double Y);
    void DMS (double Dd, int& D, int& M, double& S);

private slots:
    void setDateNow();
	void updateAll();
	void updateLongLat();
};

#endif
