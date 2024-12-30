#ifndef __METHODAPI_H
#define __METHODAPI_H

#ifdef VS_METHOD_API_EXPORTS
#define VS_METHOD_API __declspec(dllexport)
#else
#define VS_METHOD_API __declspec(dllimport)
#endif

#ifdef _DEBUG
bool MatrixCalculation(double *pCoefficient, float *pValue, float *pUnknown, int nRows, int nCols);
#else
VS_METHOD_API bool MatrixCalculation(double *pCoefficient, float *pValue, float *pUnknown, int nRows, int nCols);
#endif // DEBUG

#endif //__METHODAPI_H

