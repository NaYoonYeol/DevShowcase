using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Common")]
    public class MathHelper
    {
        private static Vector3 _bezierPoint = Vector3.zero;

        public static Vector3 QuadraticBezierCurves(Vector3 startPoint, Vector3 controlPoint, Vector3 endPoint, float moveRatio)
        {
            _bezierPoint = Vector3.zero;

            Vector3 m1 = Vector3.Lerp(startPoint, controlPoint, moveRatio);
            Vector3 m2 = Vector3.Lerp(controlPoint, endPoint, moveRatio);

            _bezierPoint = Vector3.Lerp(m1, m2, moveRatio);

            return _bezierPoint;
        }

        public static Vector3 CubicBezierCurves(Vector3 startPoint, Vector3 controlPoint1, Vector3 controlPoint2, Vector3 endPoint, float moveRatio)
        {
            _bezierPoint = Vector3.zero;

            Vector3 m1 = Vector3.Lerp(startPoint, controlPoint1, moveRatio);
            Vector3 m2 = Vector3.Lerp(controlPoint1, controlPoint2, moveRatio);
            Vector3 m3 = Vector3.Lerp(controlPoint2, endPoint, moveRatio);

            Vector3 n1 = Vector3.Lerp(m1, m2, moveRatio);
            Vector3 n2 = Vector3.Lerp(m2, m3, moveRatio);

            _bezierPoint = Vector3.Lerp(n1, n2, moveRatio);

            return _bezierPoint;
        }

        /// <summary>
        /// Return whether the SqrMagnitude of the two vectors is less than or equal to 0.01f.
        /// </summary>
        /// <returns>bool</returns>
        public static bool IsApproximateVector(Vector3 a, Vector3 b)
        {
            return Vector3.SqrMagnitude(a - b) <= 0.01f;
        }

        /// <summary>
        /// Return whether the SqrMagnitude of the vector is close enough to the zero vector. (Comparison value: 0.01f)
        /// </summary>
        /// <param name="v">Vector to compare</param>
        /// <returns>bool</returns>
        public static bool IsNearZeroVector(Vector3 v)
        {
            return Vector3.SqrMagnitude(v) <= 0.01f;
        }

        /// <summary>
        /// Return whether the direction vector is forward or backward based on the forward vector.
        /// </summary>
        /// <returns>bool</returns>
        public static bool FrontDiscrimination(Vector3 forward, Vector3 direction)
        {
            return Vector3.Dot(forward, direction) > 0;
        }
    }

}
