#pragma once

// TODOs:
// - Strong typedef for Angle class component (use this library? https://www.foonathan.net/2016/10/strong-typedefs/).
// - Make TEN::Math::Angles namespace for all angle-related classes and potential utility functions.

//namespace TEN::Math::Angles
//{
	class Angle
	{
	private:
		// Normalized angle component (stored as radians)
		float Value = 0.0f;

	public:
		// Constructors
		Angle(float radians);
		static Angle FromDeg(float degrees);

		// Utilities
		void Compare(Angle angle, float epsilon = 0.0f);
		static bool Compare(float angle0, float angle1, float epsilon = 0.0f);

		void ShortestAngularDistance(Angle angleTo);
		static float ShortestAngularDistance(float angleFrom, float angleTo);

		void InterpolateLinear(Angle angleTo, float alpha = 1.0f, float epsilon = 0.0f);
		static float InterpolateLinear(float angleFrom, float angleTo, float alpha = 1.0f, float epsilon = 0.0f);

		void InterpolateConstant(Angle angleTo, float rate);
		static float InterpolateConstant(float angleFrom, float angleTo, float rate);

		void InterpolateConstantEaseOut(Angle angleTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);
		static float InterpolateConstantEaseOut(float angleFrom, float angleTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);

		static float OrientBetweenPoints(Vector3 point0, Vector3 point1);

		static float DeltaHeading(Vector3 origin, Vector3 target, float heading); // TODO: I don't even know what this does.

		// Converters
		static float DegToRad(float degrees);
		static float RadToDeg(float radians);
		static float ShrtToRad(short shortForm);
		static short DegToShrt(float degrees);
		static short RadToShrt(float radians);

		// Operators
		// TODO: Temporary. A strong typedef with templated overloaded operators would be the proper way.
		operator float() const;
		bool   operator ==(float value);
		bool   operator !=(float value);
		Angle  operator +(float value);
		Angle  operator -(float value);
		Angle  operator *(float value);
		Angle  operator *(int value);
		Angle  operator /(float value);
		Angle  operator /(int value);
		Angle& operator =(float value);
		Angle& operator +=(float value);
		Angle& operator -=(float value);
		Angle& operator *=(float value);
		Angle& operator *=(int value);
		Angle& operator /=(float value);
		Angle& operator /=(int value);

	private:
		// Utilities
		void Normalize();
		static float Normalize(float angle);
		static float ClampAlpha(float value);
		static float ClampEpsilon(float value);
	};
//}
