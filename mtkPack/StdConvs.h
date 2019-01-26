#pragma once

namespace mtkPack {

	class StdConvs
	{
	public:
		StdConvs(void);
		~StdConvs(void);

		static double FahrenheitToCelsius(const double& AValue);
		static double CelsiusToFahrenheit(const double& AValue);
	};

}
