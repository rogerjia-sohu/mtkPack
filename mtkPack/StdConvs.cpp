#include "StdConvs.h"

mtkPack::StdConvs::StdConvs(void)
{
}


mtkPack::StdConvs::~StdConvs(void)
{
}

double mtkPack::StdConvs::FahrenheitToCelsius(const double& AValue) {
	return ((AValue - 32) * 5) / 9;
}

double mtkPack::StdConvs::CelsiusToFahrenheit(const double& AValue) {
	return ((AValue * 9) / 5) + 32;
}